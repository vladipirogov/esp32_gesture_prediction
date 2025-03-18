# Demo projet for gesture Recognition with ESP32 and MPU6050

## Overview
This project implements gesture recognition using an ESP32 microcontroller and an MPU6050 accelerometer. The recognized gestures are transmitted via Bluetooth using the NimBLE stack. TensorFlow Lite for Microcontrollers (TFLM) is used to perform real-time gesture classification.

## Features
- Collects accelerometer data from MPU6050
- Processes sensor data and classifies gestures using TensorFlow Lite
- Uses FreeRTOS tasks and queues for data exchange between gesture prediction and BLE transmission
- Supports BLE communication to send recognized gestures to a client device

## Hardware Requirements
- ESP32 development board
- MPU6050 accelerometer
- Power supply (USB or battery)

## Software Requirements
- ESP-IDF v 4.4.5
- TensorFlow Lite for Microcontrollers
- NimBLE (ESP-IDF BLE stack)
- FreeRTOS

## Installation & Setup
### 1. Clone the repository
```sh
git clone https://github.com/vladipirogov/esp32_gesture_prediction.git
cd esp32_gesture_prediction
```

### 2. Set up ESP-IDF
Follow the [ESP-IDF setup guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html) to install and configure the development environment.

### 3. Configure the project
Run:
```sh
idf.py menuconfig
```
Check the MPU6050 and Bluetooth settings.

### 4. Build and flash
```sh
idf.py build flash monitor
```

## Project Structure
```
├── main
│   ├── gesture.cc              # Gesture recognition task using TFLM
│   ├── gesture.h               # Header file for gesture recognition
│   ├── ble_provider.c          # Bluetooth communication logic
│   ├── ble_provider.h          # Header file for BLE provider
│   ├── mpu6050.c               # MPU6050 sensor interface
│   ├── model.h                 # TensorFlow Lite model (generated as xxd array)
│   ├── main.c                  # Main entry point
│   ├── pwm.h                   # Light PWM header
│   ├── pwm.c                   # Light PWM source
│   ├── CMakeLists.txt          # Build configuration
├── include
│   ├── mpu6050.h               # Header file for MPU6050 sensor
├── sdkconfig.defaults.esp32s3  # ESP-IDF configuration file
├── idf_component.yml           # ESP-IDF component configuration
├── CMakeLists.txt              # Project build configuration
├── README.md                   # This file
```

## How It Works
1. The ESP32 collects accelerometer data from the MPU6050 at 50Hz.
2. The data is preprocessed and fed into a TensorFlow Lite model.
3. The model predicts one of three gestures: `Circle`, `Cross`, or `Pad`.
4. The recognized gesture is sent to a BLE-connected device.

## Communication Between Tasks
- **Gesture recognition task (`gesture_predict`)**:
  - Reads MPU6050 data.
  - Runs TensorFlow Lite inference.
  - Pushes the result to a FreeRTOS queue.
- **BLE transmission task (`ble_provider`)**:
  - Reads the queue and transmits data over Bluetooth.
  
## BLE Communication
- BLE Service UUID: `0x180D`
- Characteristics:
  - `0x2A37`: Gesture data (notify)
  - `0x2A19`: Read rate
  - `0x2A39`: Write command interface
- The device name appears as `BLE-Server`.

## References
- [ESP-IDF Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/)
- [TensorFlow Lite for Microcontrollers](https://ai.google.dev/edge/litert/microcontrollers/overview)
- [NimBLE](https://mynewt.apache.org/latest/network/ble_hs/ble_hs.html)

