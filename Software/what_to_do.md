The code has to be on the runner VM. 
Make sure the STM is connected to the runner as well.

run make in the root Test, after which run->
openocd -f interface/stlink.cfg -f target/stm32l1x_dual_bank.cfg -c "init; reset halt; flash write_image erase x.bin 0x8000000; reset; exit"

Where x in x.bin is the name of the file gotten after running make

---
No currently sure whats the python file for, but I guess communication, for the runner to send the "Polo"

--- 
You can also read UART using the "screen" command, like:

screen /dev/ttyACM0 115200
or
screen -r