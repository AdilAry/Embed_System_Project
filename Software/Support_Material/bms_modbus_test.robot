*** Settings ***
Library    ModbusLibrary
Suite Setup       Connect To BMS
Suite Teardown    Disconnect From BMS

*** Variables ***
# Modbus RTU Settings (Matching your USART1_Init: 9600 baud)
${PORT}             COM3    # Change to your actual Serial Port (e.g., /dev/ttyUSB0)
${BAUDRATE}         9600
${SLAVE_ID}         1

# Thresholds from your C code
${MAX_CELL_V}       3650    # 3.65V
${MAX_CURRENT}      10000   # 10A
${MIN_TEMP}         -20
${MAX_TEMP}         40

*** Test Cases ***
Verify BMS Data Within Safety Limits
    [Documentation]    Reads 7 registers and ensures no faults are present.
    # Read 7 registers starting from address 0
    ${data}=    Read Input Registers    0    7    unit=${SLAVE_ID}
    
    # Unpack the list [Cell1, Cell2, Cell3, Cell4, Current, Temp1, Temp2]
    ${c1}  ${c2}  ${c3}  ${c4}  ${curr}  ${t1}  ${t2}=    Set Variable    ${data}

    Log    Cell Voltages: ${c1}, ${c2}, ${c3}, ${c4} mV
    Log    Current: ${curr} mA
    Log    Temperatures: ${t1}, ${t2} C

    # Validation Logic (Negative Testing)
    Should Be True    ${c1} <= ${MAX_CELL_V}    Cell 1 Overvoltage!
    Should Be True    ${c2} <= ${MAX_CELL_V}    Cell 2 Overvoltage!
    Should Be True    ${curr} <= ${MAX_CURRENT}  Overcurrent Detected!
    
    # Check Temp Range
    Should Be True    ${t1} >= ${MIN_TEMP} and ${t1} <= ${MAX_TEMP}    Temp 1 Out of Range!

Verify CRC and Communication
    [Documentation]    Implicitly tests your CRC16 C function. 
    ...               If CRC fails, the library will throw an error.
    ${result}=    Read Input Registers    0    1    unit=${SLAVE_ID}
    Should Not Be Empty    ${result}

*** Keywords ***
Connect To BMS
    # method='rtu' for serial communication
    Open Modbus Connection    ${PORT}    rtu    ${BAUDRATE}    stopbits=1    bytesize=8    parity=N

Disconnect From BMS
    Close Modbus Connection