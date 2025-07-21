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
Responsible for the commutation logic, BEMF (Back Electromotive Force) reading, and PWM generation.
First, we will start with PWM. On ESP32, we can use a module called MCPWM (MOTOR CONROL PULSE WIDTH MODULATION).This module focuses on controlling three-phases motor like BLDCs, and offers high frequency occuracy.
