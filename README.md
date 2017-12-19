# not so lunar clock
MSP430G based clock slow down device.

## description
The MCU replaces standard quartz clock electronics. It uses RTC with 32.768kHz crystal. MCU is mostly in sleep mode. The clock speed can be setted by external trimmer to 24 - 32 days. The measurement is done once in several minutes and the trimmer is switched off when it's not needed - to save some power. The device is powered by two AAA bateries. The higher voltage than standard clock have is needed for MCU.

![the clock](/doc/back_side.jpg)

## todo
Switching off the ADC module when it's not needed should help to reduce power consumption. Maybe I'll try it in some next years, when bateries are gone...

