# Drone
Do you really want to know whats going on?
## What is this?
This is an on going drone build with a Raspberry Pi (running Debian Linux) as the main flight controller.
## Languages
This project is mainly written in C++ and Node.js.
It includes HTML/CSS for front end and UI.
## Components
- Raspberry Pi 3 B
- PCA9685 (pwm control)
- BMP390 (pressure+temp sensor)
- MPU6050 (accelerometer+gyroscope)
- Arducam
- Simonk 35A ESC x 4
- EMAX 2213/935KV BLDC
- Lots of PLA and screws

## Software
- Debian Linux
- libi2c
- node/npm

## How to use (Probably won't work for any other hardware without extensive configuration)
```sh
git clone
cd drone
make

cd server
npm install

cd ..
# in seperate shells
./bin/drone
node server/server.js
```
#### Make certain tool
```sh
git clone
cd drone
make <tool name>
./bin/<tool name>
# tool name does not include .cpp at the end (led.cpp -> make led)
```

## Directories
| Directory | Description |
| ----------- | ----------- |
| `bin` | compiled stuff goes here |
| `build` | all the .o stuff | 
| `include` | all header files used (and unused) in the program |
| `src` | source code to the main drone control library |
| `main` | contains the main function for the main drone control |
| `tools` | contains all other files handling the main drone control library, but is not the main control program |
| `server` | contains files pertaining to the socket.io server |
