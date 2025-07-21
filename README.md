# Bild-an-Eletronic-Speed-Controller-
This project aims to outline an ESC (Electronic Speed Controller) using KiCad, simulate its behavior with LTspice, and study its electronic fundamentals. Based on this, a PCB will be designed and an ESP32 will be used for control purposes.

## 1 Materials
- Controller: ESP32-S3
- 6 Mosfets 
- Drive Gate
- resistor pull-down 
- 2 capcitor bypass
- 1 capacitor bootstrap 
- 1 Diodo Bootstrap

## 2 theoretical fundamentals
### 2.1 ESP32-S3
Responsible for the commutation logic, BEMF (Back Electromotive Force) reading, and PWM generation.<br>
First, we will start with PWM. On ESP32, we can use a module called MCPWM (MOTOR CONROL PULSE WIDTH MODULATION).This module focuses on controlling three-phases motor like BLDCs, and offers high frequency occuracy.

according to Espressif's official documatation: 

> "ESP32-S3 integrates two MCPWMs that can be used to drive digital motors and smart light. Each MCPWM
peripheral has one clock divider (prescaler), three PWM timers, three PWM operators, and a capture module.
PWM timers are used for generating timing references. The PWM operators generate desired waveform based
on the timing references. Any PWM operator can be configured to use the timing references of any PWM
timers. Different PWM operators can use the same PWM timer’s timing references to produce related PWM
signals. PWM operators can also use different PWM timers’ values to produce the PWM signals that work
alone. Different PWM timers can also be synchronized together."



