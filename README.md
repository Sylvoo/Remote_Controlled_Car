# Remote Controlled Car with real-time parameter monitoring🚗💨

**Author:** Sylwester Ślusarczyk, Filip Mrozik, AGH 🧑‍🎓

## Project Description

The "**Remote Controlled Car**" project is an advanced remote control system for a mobile vehicle based on the **ESP32** microcontroller and the **L298N** motor driver. The code is divided into two modules:

**car** (drive control)  
**controller** (control interface).

The main goal of the project is to create an **intelligent remote-controlled vehicle** that combines both **motor control** and **wireless communication**. 🕹️🤖

## Key Features

### Drive Control 🚗
- Control of two DC motors via the **L298N** motor driver (H-bridges),
- Control of driving direction: *forward/reverse* and *turn left/right*,
- Speed regulation of the motors through PWM.

### Communication and Control 🌐
- Remote control of the vehicle via the **ESPnow** protocol,
- Use of a **dual-axis XY joystick** for controlling the vehicle's movement,
- The **controller** module handles the reception of commands from the joystick and generates control signals,
- The **car** module performs physical drive actions and reacts to the signals.

### Planned Extensions 🔧
- **HTTP Server on the ESP32 Car** for real-time parameter monitoring, with telemetry data viewable through a web browser,
- Possibility to expand with additional sensors and automatic control algorithms,
### Electronic Schema

<img width="588" height="743" alt="image" src="https://github.com/user-attachments/assets/aabd61e4-feeb-4b7b-86cc-54b863bb76b3" />
<img width="702" height="684" alt="image" src="https://github.com/user-attachments/assets/c6deec84-abc3-437b-9eb2-9b6bf66c042c" />

