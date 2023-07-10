# STM32 in-application programming (IAP) using the USART via YMODEM Protocol
 In-application programming (IAP) is a method of updating firmware or software on a device while it is still in operation.

## Example
This example is based on the [X-CUBE-IAP-USART](https://www.st.com/en/embedded-software/x-cube-iap-usart.html) Fimrware Package designed to work for STM32G073Z_EVAL, STM32L476G_EVAL and STM3210C_EVAL boards.
This firmware package is migrated to work on STM32G0xx devices. 


## Hardware and Tools required
- Nucleo-G071RB
- USB A to Micro USB B Cable 
- STM32 CubeIDE (Tested on v1.21.1)
- Teraterm(v4.105 or higher) is recommended, although any terminal emulator software can be used.

## Description of IAP_Main directory 
This directory contains a set of sources files and pre-configured projects that 
describes how to build an application to be loaded into Flash memory using
In-Application Programming (using USART via YMODEM protocol).

**Directory Contents**
- "STM32G0xx_IAP/IAP_Main/Inc": contains the IAP firmware header files 

    - STM32G0xx_IAP/IAP_Main/Inc/main.h              The main include file of the project.
    - STM32G0xx_IAP/IAP_Main/Inc/common.h            This file provides all the headers of the common functions.
    - STM32G0xx_IAP/IAP_Main/Inc/flash_if.h          This file provides all the firmware 
                                                     function headers of the flash_if.c file.
    - STM32G0xx_IAP/IAP_Main/Inc/menu.h              This file provides all the firmware
                                                     function headers of the menu.c file.
    - STM32G0xx_IAP/IAP_Main/Inc/ymodem.h            This file provides all the firmware
                                                     function headers of the ymodem.c file.
    - STM32G0xx_IAP/IAP_Main/Inc/STM32G0xx_hal_conf.h  Library Configuration file
    - STM32G0xx_IAP/IAP_Main/Inc/STM32G0xx_it.h      Header for STM32G0xx_it.c 


- "STM32G0xx_IAP/IAP_Main/Src": contains the IAP firmware source files
    - STM32G0xx_IAP/IAP_Main/Src/main.c              Main program
    - STM32G0xx_IAP/IAP_Main/Src/STM32G0xx_it.c      Interrupt handlers
    - STM32G0xx_IAP/IAP_Main/Src/STM32G0xx_hal_msp.c Microcontroller specific packages
                                                     initialization file.
    - STM32G0xx_IAP/IAP_Main/Src/flash_if.c          The file contains write, erase and disable
                                                     write protection of the internal Flash
                                                     memory.
    - STM32G0xx_IAP/IAP_Main/Src/menu.c              This file contains the menu to select
                                                     downloading a new binary file, uploading
                                                     internal Flash memory, executing the binary
                                                     and disabling the write protection of
                                                     write-protected pages
    - STM32G0xx_IAP/IAP_Main/Src/common.c            This file provides functions related to
                                                     read/write from/to USART peripheral
    - STM32G0xx_IAP/IAP_Main/Src/ymodem.c            This file provides all the firmware functions
                                                     related to the ymodem protocol.
    - STM32G0xx_IAP/IAP_Main/Src/system_STM32G0xx.c  STM32G0xx system source file


**IAP implementation on STM32 Nucleo-G071RB**
![Implementation](Screenshot_1.png)
(1) User application location address is defined in the flash_if.h file as: 
#define APPLICATION_ADDRESS           ((uint32_t)0x08008000) <br>
To modify it, change the default value to the desired one. Note that the application must be linked
relatively to the new address too.

![Flash Representation](<Screenshot 2023-06-25 161810.png>)
 
**USART AND TERATERM CONFIGURATIONS**
- Word Length = 8 Bits
- One Stop Bit
- No parity
- BaudRate = 115200 baud
- flow control: None 
- Ymodem protocol is using CRC16 by default. To switch to checksum, comment #define CRC16_F in ymodem.c

## Description of IAP_User_Application directory
This is the user application code that needs to be flashed on the Nucleo-G071RB board at location 0x08008000. 

The example illustrated is blinky where the board's on-board LED(PA5) will keep blinking for every 200ms in a while(1) loop once the IAP application jumps to the user application.


**Directory Contents**
 - "STM32G0xx_IAP/IAP_User_Application/Inc": contains the binary_template firmware header files 
 - STM32G0xx_IAP/IAP_User_Application/Inc/STM32G0xx_hal_conf.h  Library Configuration file
 - STM32G0xx_IAP/IAP_User_Application/Inc/STM32G0xx_it.h      Header for STM32G0xx_it.c
 - STM32G0xx_IAP/IAP_User_Application/Inc/main.h              Header for main.c
 - "STM32G0xx_IAP/IAP_User_Application/Src": contains the binary_template firmware source files 
 - STM32G0xx_IAP/IAP_User_Application/Src/main.c              Main program
 - STM32G0xx_IAP/IAP_User_Application/Src/STM32G0xx_it.c      Interrupt handlers
 - STM32G0xx_IAP/IAP_User_Application/Src/system_STM32G0xx.c  STM32G0xx system source file

**Configurations to be done in this project only if using this code as a reference**
1. Set the flash program load address to 0x08008000 on the .ld file of the project
2. Uncomment the line #if defined(USER_VECT_TAB_ADDRESS) and change the VECT_TAB_OFFSET value to 0x00008000U in the system_stm32g0xx.c file
    **Note**: 0x08008000 is the User Apllication start address and this value should be changed accordingly if this code is being used as a reference

## Steps
- Build both the projects(IAP_User_Application and IAP_Main) and generate the .bin file for the IAP_User_Application project during the build process. You can generate the .bin of a project by following Project-> Properties -> C/C++ build -> Settings -> MCU Post Build Outputs -> Check on "Convert to .bin file" -> Apply
- Run IAP_Main and communicate with the project using teraterm(make sure the teraterm UART configurations match with the project)as follows:

     1. Download image to internal flash:
       Once you choose this option, the program will wait until you send the .bin file
       To send the .bin file perform File->Transfer->Ymodem->Send->Choose IAP_User_Application.bin(IAP_User_Application\Debug) on teraterm

    2. Upload image from internal flash:
       Select the location where you want to store the file on your local computer by performing File-> Change Directory on Teraterm.
       Choose this option and perform File->Transfer->Ymodem->Receive to save the .bin file at the desired location

    3. Execute the loaded Application
       Once you have downloaded the image on the flash, choose this option to execute the IAP_User_Application from location 0x08008000 and you should observe the onboard LED toggling for every 200ms

    4. Enable/Disable the write protection
        This option will enable/disable the write protection from Page 16-63(IAP_User_Application loaded address)
        When write protection is ON, if you try to download the flash image an error will be displayed

## Troubleshooting

**Issues observed on teraterm v4.105** :
Sometimes teraterm might not work as execpted and not send the .bin file to the program accurately. During such an instance try to relaunch teraterm or  do a mass erase on the microcontrollers flash or restart your PC.

**Caution** : Issues and the pull-requests are **not supported** to submit problems or suggestions related to the software delivered in this repository. The DBFU_L476 example is being delivered as-is, and not necessarily supported by ST.

**For any other question** related to the product, the hardware performance or characteristics, the tools, the environment, you can submit it to the **ST Community** on the STM32 MCUs related [page](https://community.st.com/s/topic/0TO0X000000BSqSWAW/stm32-mcus).
