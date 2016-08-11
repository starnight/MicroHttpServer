This is the Micro HTTP Server on FreeRTOS example for STM32F407, especially for STM32F4-Discovery.

### Tools and software requirements ###

* [GNU toolchain](https://launchpad.net/gcc-arm-embedded)
  for ARM Cortex-M.

* [stlink](https://github.com/texane/stlink) STM32 debug & flash utility
 written by texane for Linux.

* [STM32F4 library](http://www.st.com/web/catalog/tools/FM147/CL1794/SC961/SS1743/PF257901) STM32F4 DSP and standard peripherals library.

* [FreeRTOS](http://www.freertos.org/a00104.html?1) FreeRTOS library.

### Hardware ###

According to the schematic of [STM32F4-Discovery](http://www.st.com/st-web-ui/static/active/en/resource/technical/document/user_manual/DM00039084.pdf) Peripherals:

* LED4 (Green): PD12 connected to LD4

* LED3 (Orange): PD13 connected to LD3

* LED5 (Red): PD14 connected to LD5

* LED6 (Blue): PD15 connected to LD6

* User Button: PA0 connected to B1

* USART2 TX: PA2 connected to console RX

* USART2 RX: PA3 connected to console TX

* USART6 TX: PC6 connected to ESP8266 RX

* USART6 RX: PC7 connected to ESP8266 TX

The WiFi module [ESP01](https://www.hackster.io/esp/products/esp8266-esp-01) is one kind of ESP8266 WiFi module.

### Usage ###

1. Edit the Makefile to modify the "STM\_DIR" to the path of STM32F4 DSP and
   standard peripherals library.

2. Edit the Makefile to modify the "FREERTOS\_DIR" to the path of FreeRTOS
   library.

3. Define the macroes of ```_AP_SSID``` and ```_AP_PWD``` in src/main.c which are the SSID and password of the AP.

4. Compile: ``` make ```

5. Flash to STM32F407: ``` make flash ```

6. Get a terminal connected with the right UART settings to USART2 on STM32F407. (115200 baud rate, 8 data bits, 1 stop bit, no parity check and non-flow control.)

7. Get a ESP8266 connected with the right UART settings to USART6 on STM32F407. (115200 baud rate, 8 data bits, 1 stop bit, no parity check and non-flow control.)

8. Reset the power of STM32F4-Discovery.

9. Wait the Micro HTTP Server being started. Then use a browser to browse the defined web pages or web APIs.
