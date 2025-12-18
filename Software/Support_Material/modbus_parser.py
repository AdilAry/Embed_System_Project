import time
from pymodbus.client import ModbusSerialClient
from prometheus_client import start_http_server, Gauge

SERIAL_PORT = '/dev/ttyUSB0'
BAUD_RATE = 9600
SLAVE_ID = 0x01
POLL_INTERVAL = 2 

CELL_VOLTAGE = Gauge('bms_cell_voltage_mv', 'Voltage of individual cells in mV', ['cell_index'])
CURRENT = Gauge('bms_current_ma', 'System current in mA')
TEMPERATURE = Gauge('bms_temperature_c', 'System temperatures in Celsius', ['sensor_index'])

def read_bms_data():
    client = ModbusSerialClient(
        port=SERIAL_PORT,
        baudrate=BAUD_RATE,
        parity='N',
        stopbits=1,
        bytesize=8,
        timeout=1
    )

    if not client.connect():
        print(f"Failed to connect to {SERIAL_PORT}")
        return

    try:
        result = client.read_input_registers(address=0, count=7, slave=SLAVE_ID)

        if not result.isError():
            
            for i in range(4):
                CELL_VOLTAGE.labels(cell_index=i+1).set(result.registers[i])
            
            curr = result.registers[4]
            if curr > 32767: curr -= 65536
            CURRENT.set(curr)
            
            for i in range(2):
                temp = result.registers[5+i]
                if temp > 32767: temp -= 65536
                TEMPERATURE.labels(sensor_index=i+1).set(temp)
                
            print(f"Logged: V1={result.registers[0]}mV, I={curr}mA, T1={result.registers[5]}C")
        else:
            print("Modbus Read Error:", result)

    except Exception as e:
        print(f"Error during polling: {e}")
    finally:
        client.close()

if __name__ == "__main__":
    start_http_server(8000)
    print("Prometheus Exporter started on port 8000")
    
    while True:
        read_bms_data()
        time.sleep(POLL_INTERVAL)