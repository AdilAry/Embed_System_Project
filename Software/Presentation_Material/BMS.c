/*
MODBUS RTU slave (USART)
04 READ INPUT REGISTER
https://www.modbustools.com/modbus.html#function04
Step 7.
- USART2 interrupt, ok
- wrong slave address, ok
- read 7 bytes from usartx, ok
- crc check, ok
- reads sensor, ok
- gives resonse to master, ok
- Reads ADC and sends data to master
- USART1
*/

/* Includes */
#include "stm32l1xx.h"
#define HSI_VALUE ((uint32_t)16000000)
#include "nucleo152start.h"
#include <stdint.h>
#include <stdbool.h>
#include <math.h>


// Constants,  first MODBUS stuff, trigger constants and temperature calc variables according to Steinhart-Hart simplified equation

#define SLAVE_ADDRESS 0x01
#define FUNCTION_CODE 0x04
#define REGISTER_NUM 7  // 4 cells + 1 current + 2 temps
#define RESPONSE_BYTES (REGISTER_NUM * 2)
#define TX_EN_PIN GPIO_ODR_ODR_5  // PA5 controls MAX3485 RE/DE for MODBUS


#define MAX_CELL_VOLTAGE 3650      // 3.65V in mV
#define MAX_DISCHARGE_CURRENT 10000 // 10A in mA
#define MIN_TEMP -20               // -20C
#define MAX_TEMP 40                // 40C

#define T_BETA 3435 
#define T_R25 100000      // 100k at 25C
#define SERIES_RESISTOR 10000      // 10k series resistor
#define VREF 3300              // 3.3V reference in mV
#define T0_KELVIN 298.15           // 25C in Kelvin

volatile uint8_t modbus_flag = 0;
int16_t cell_voltages[4] = {0};    // mV
int16_t current_mA = 0;           	// mA
int16_t temperatures[2] = {0};     // C

// function prototypes
uint16_t ADC_Read(uint8_t channel);
void read_all_sensors(void);
void select_cell(uint8_t cell_num);
int16_t adc_to_voltage(uint16_t adc_val);
int16_t calc_temp(uint16_t adc_val);
int16_t calc_current(uint16_t adc_val);
void control_mosfets(void);

void ADC_Init(void);
void GPIO_Init(void);
void USART1_Init(void);
uint8_t USART1_read(void);
void USART1_write(uint8_t data);
void delay_Ms(int delay);
void delay_us(unsigned long delay);
void respond_with_bms_data(void);
void wrong_slave_address(void);
unsigned short CRC16(const uint8_t *data, uint16_t length);

/**
**=======================================================================
====
**
** Abstract: main program
**
**=======================================================================
====
*/

int main(void) {
    __disable_irq();
    
    SetSysClock();
    SystemCoreClockUpdate();
    
    GPIO_Init();
    ADC_Init();
    USART1_Init();
    
    USART1->CR1 |= USART_CR1_RXNEIE;
    NVIC_EnableIRQ(USART1_IRQn);
    
    __enable_irq();
    
    uint8_t frame[8] = {0};
    
    while (1) {
        read_all_sensors();
        control_mosfets();
        
        // MODBUS
        if (modbus_flag == 1) {
            frame[0] = SLAVE_ADDRESS;
            for (int i = 1; i < 8; i++) {
                frame[i] = USART1_read();
            }
            
            uint16_t crc_calc = CRC16(frame, 6);
            if (((crc_calc & 0xFF) == frame[6]) && ((crc_calc >> 8) == frame[7])) {
                uint16_t start_addr = (frame[2] << 8) | frame[3];
                uint16_t quantity = (frame[4] << 8) | frame[5];
                
                if (quantity > REGISTER_NUM || frame[1] != FUNCTION_CODE) {
                    wrong_slave_address();
                    continue;
                }
                
                respond_with_bms_data();
            }
            
            modbus_flag = 0;
            USART1->CR1 |= USART_CR1_RXNEIE;
        } else if (modbus_flag == 2) {
            wrong_slave_address();
        }
        
        delay_Ms(100);  // Updates info 10 times in a secod, can be changed to be less or more often. 
    }
}


uint16_t ADC_Read(uint8_t channel) {
    ADC1->SQR5 = channel;  // Select channel
    ADC1->CR2 |= ADC_CR2_SWSTART;  // Start conversion
    
    while (!(ADC1->SR & ADC_SR_EOC));  // Wait for conversion
    
    return ADC1->DR;
}

void select_cell(uint8_t cell_num) {
    // Important node to add is that we used 3 input MUXes, due to some unforseen things, here we are using just 2 address lines cause hardware wise A0 line is perma low, 0
	
    if (cell_num & 0x01) {
        GPIOC->ODR |= GPIO_ODR_ODR_2;  // A1 = 1
    } else {
        GPIOC->ODR &= ~GPIO_ODR_ODR_2;  // A1 = 0
    }
    
    if (cell_num & 0x02) {
        GPIOC->ODR |= GPIO_ODR_ODR_3;  // A2 = 1
    } else {
        GPIOC->ODR &= ~GPIO_ODR_ODR_3;  // A2 = 0
    }
    
    delay_us(10);
}

// All accordign to the Communication Documentation
void read_all_sensors(void) {
    // cycling through cells with MUXes
    for (int i = 0; i < 4; i++) {
        select_cell(i);
        uint16_t adc = ADC_Read(4);  // PA4 = ADC_IN4
        cell_voltages[i] = adc_to_voltage(adc);
    }
    
    // Read current (PB1 = ADC_IN9)
    uint16_t current_adc = ADC_Read(9);
    current_mA = calc_current(current_adc);
    
    // Read temperatures (PA0 = ADC_IN0, PA1 = ADC_IN1)
    uint16_t temp1_adc = ADC_Read(0);
    uint16_t temp2_adc = ADC_Read(1);
    temperatures[0] = calc_temp(temp1_adc);
    temperatures[1] = calc_temp(temp2_adc);
}

int16_t calc_temp(uint16_t adc_val) {
    if (adc_val == 0) return -999;  // Error value
    
    // Calculate thermistor resistance
    float v_out = (adc_val * 3.3) / 4095.0;
    float r_th = (SERIES_RESISTOR * v_out) / (3.3 - v_out);
    
    // Beta equation: 1/T = 1/T0 + (1/B)*ln(R/R0)
    float temp_k = 1.0 / ((1.0 / T0_KELVIN) + (1.0 / T_BETA) * log(r_th / T_R25));
    float temp_c = temp_k - 273.15;
    
    return (int16_t)(temp_c);
}

int16_t calc_current(uint16_t adc_val) {
    int16_t voltage_mv = adc_to_voltage(adc_val);
    // I = V / 0.001(our shutn resistor value) = V * 1000 (in milli)
    return voltage_mv * 1000;
}

int16_t adc_to_voltage(uint16_t adc_val) {
    // ADC is 12-bit
    return (int16_t)((adc_val * VREF) / 4095);
}

void control_mosfets(void) {
    int fault = 0;	
	// if fault becomes 1 then we trigger discharge
    
    // Check cell voltages
    for (int i = 0; i < 4; i++) {
        if (cell_voltages[i] > MAX_CELL_VOLTAGE) {
            fault = 1;
            break;
        }
    }
    
    // Check current
    if (current_mA > MAX_DISCHARGE_CURRENT) {
        fault = 1;
    }
    
    // Check temperatures
    if (temperatures[0] < MIN_TEMP || temperatures[0] > MAX_TEMP ||
        temperatures[1] < MIN_TEMP || temperatures[1] > MAX_TEMP) {
        fault = 1;
    }
    
    if (fault==1) {
        GPIOC->ODR &= ~GPIO_ODR_ODR_0;
        GPIOC->ODR |= GPIO_ODR_ODR_1;
    } else {
        GPIOC->ODR |= GPIO_ODR_ODR_0;
        GPIOC->ODR &= ~GPIO_ODR_ODR_1;
    }
}

//////////////////////////////////////////////

void respond_with_bms_data(void) {
    GPIOA->ODR |= TX_EN_PIN;
    
    uint8_t response[RESPONSE_BYTES + 5] = {0};
    response[0] = SLAVE_ADDRESS;
    response[1] = FUNCTION_CODE;
    response[2] = RESPONSE_BYTES;
    
    // sneding data: 4 cells + current + 2 temps
    int idx = 3;
    for (int i = 0; i < 4; i++) {
        response[idx++] = (cell_voltages[i] >> 8) & 0xFF;
        response[idx++] = cell_voltages[i] & 0xFF;
    }
    response[idx++] = (current_mA >> 8) & 0xFF;
    response[idx++] = current_mA & 0xFF;
    for (int i = 0; i < 2; i++) {
        response[idx++] = (temperatures[i] >> 8) & 0xFF;
        response[idx++] = temperatures[i] & 0xFF;
    }
    
    uint16_t crc = CRC16(response, RESPONSE_BYTES + 3);
    response[RESPONSE_BYTES + 3] = crc & 0xFF;
    response[RESPONSE_BYTES + 4] = (crc >> 8) & 0xFF;
    
    for (int i = 0; i < RESPONSE_BYTES + 5; i++) {
        USART1_write(response[i]);
    }
    
    while (!(USART1->SR & USART_SR_TC));  // Wait for transmission complete
    GPIOA->ODR &= ~TX_EN_PIN;  // Disable transmitter
}

void wrong_slave_address(void) {
    USART1->CR1 &= ~USART_CR1_RE;
    delay_Ms(10);
    USART1->CR1 |= USART_CR1_RE | USART_CR1_RXNEIE;
    modbus_flag = 0;
}

unsigned short CRC16(const uint8_t *data, uint16_t length) {
    uint16_t crc = 0xFFFF;
    while (length--) {
        crc ^= *data++;
        for (uint8_t i = 0; i < 8; ++i) {
            crc = (crc & 1) ? (crc >> 1) ^ 0xA001 : crc >> 1;
        }
    }
    return crc;
}

void USART1_Init(void) {
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    
    GPIOA->AFR[1] &= ~(0xFF << 4);
    GPIOA->AFR[1] |= (7 << 4) | (7 << 8);
    GPIOA->MODER &= ~(0xF << 18);
    GPIOA->MODER |= (2 << 18) | (2 << 20);
    
    USART1->BRR = 0x0D05;  // 9600 baud, 32MHz
    USART1->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;
}

void USART1_write(uint8_t data) {
    while (!(USART1->SR & USART_SR_TXE));
    USART1->DR = data;
}

uint8_t USART1_read(void) {
    while (!(USART1->SR & USART_SR_RXNE));
    return USART1->DR;
}

void GPIO_Init(void) {
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOBEN | RCC_AHBENR_GPIOCEN;
    
    GPIOA->MODER |= (3 << (0 * 2)) | (3 << (1 * 2)) | (3 << (4 * 2));
    GPIOB->MODER |= (3 << (1 * 2));
    GPIOC->MODER &= ~((3 << (0 * 2)) | (3 << (1 * 2)));
    GPIOC->MODER |= (1 << (0 * 2)) | (1 << (1 * 2));
    GPIOC->MODER &= ~((3 << (2 * 2)) | (3 << (3 * 2)));
    GPIOC->MODER |= (1 << (2 * 2)) | (1 << (3 * 2));
    
    GPIOA->MODER &= ~(3 << (5 * 2));
    GPIOA->MODER |= (1 << (5 * 2));
    GPIOA->ODR &= ~TX_EN_PIN;
    
    GPIOC->ODR &= ~(GPIO_ODR_ODR_0 | GPIO_ODR_ODR_1);
}

void delay_Ms(int delay) {
    for (int i = 0; i < delay * 2460; ++i) __NOP();
}

void delay_us(unsigned long delay) {
    RCC->APB2ENR |= RCC_APB2ENR_TIM11EN;
    TIM11->PSC = 31;  // 32MHz / 32 = 1MHz (1us tick)
    TIM11->ARR = 1;
    TIM11->CNT = 0;
    TIM11->CR1 = 1;
    
    for (unsigned long i = 0; i < delay; ++i) {
        while (!(TIM11->SR & 1));
        TIM11->SR &= ~1;
        TIM11->CNT = 0;
    }
    
    TIM11->CR1 = 0;
}

void USART1_IRQHandler(void) {
    if (USART1->SR & USART_SR_RXNE) {
        uint8_t addr = USART1->DR;
        modbus_flag = (addr == SLAVE_ADDRESS) ? 1 : 2;
        USART1->CR1 &= ~USART_CR1_RXNEIE;
    }
}

void ADC_Init(void) {
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
    
    // ADC configuration
    ADC1->CR2 = ADC_CR2_ADON;  // Turn on ADC
    delay_us(10);
    
    ADC1->SMPR2 = 0x07 | (0x07 << 3) | (0x07 << 12);  // 239.5 cycles for CH0, CH1, CH4
    ADC1->SMPR3 = 0x07 << 27;  // 239.5 cycles for CH9 (PB1)
}