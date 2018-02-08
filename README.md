<<<<<<< HEAD
# PWM DC motor driver with servo input signal

The device based on MSP430G2231 turns one servo signal into PWM output for dc motor driver and the second servo signal into light ON/OFF.

##Schematic:

![Schema](/doc/schema.png)
=======
# not so lunar clock
MSP430G based clock slow down device.

## description
The MCU replaces standard quartz clock electronics. It uses RTC with 32.768kHz crystal. MCU is mostly in sleep mode. The clock speed can be setted by external trimmer to 24 - 32 days. The measurement is done once in several minutes and the trimmer is switched off when it's not needed - to save some power. The device is powered by two AAA bateries. The higher voltage than standard clock have is needed for MCU.

## schematic
![schematic](/doc/schema.png)

## prototype
![the clock](/doc/back_side.jpg)

## todo
Switching off the ADC module when it's not needed should help to reduce power consumption. Maybe I'll try it in some next years, when bateries are gone...
>>>>>>> 539679d5c9b5d76f5be2fb45123abc2a2bd3c75d

