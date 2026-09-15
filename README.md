# BurglarAlarmSystem-AM

A multi-layer intelligent burglar alarm and access control system integrating **Arduino hardware control**, **Python-based facial recognition**, and a **MATLAB supervisory control interface**.

This project combines physical intrusion detection, biometric verification, digital access management, and event logging into a modular embedded security platform.

---

## Overview

The system monitors a secured environment using:

- **Magnetic door sensors**
- **Passive Infrared (PIR) motion sensors**
- **Physical key authentication**
- **PIN-based keypad access**
- **Facial recognition verification**

When triggered, the system can:

- Activate alarm sirens
- Flash strobe warning lights
- Lock/unlock door access
- Log security events
- Notify the MATLAB monitoring interface
- Authenticate authorized users using face recognition

---

# System Architecture

The project consists of three integrated subsystems:

## 1. Arduino Embedded Controller

Handles real-time hardware monitoring and alarm logic.

### Responsibilities

- Poll intrusion sensors
- Monitor physical key insertion
- Process keypad input
- Control:

  - Sirens
  - Strobe lights
  - Door lock actuator
  - Status LEDs

- Execute state transitions:

  - Armed
  - Disarmed
  - Alarm Triggered
  - Diagnostic Mode
  - Shutdown

- Communicate with MATLAB over serial

---

## 2. Python Facial Recognition Engine

Implements biometric authentication using OpenCV and the `face_recognition` library.

### Features

- Continuous webcam stream
- Triggered face scan mode
- Authorized user recognition
- Unauthorized access detection
- Add/remove user face profiles dynamically
- Status reporting to MATLAB

### Recognition States

- `IDLE`
- `SCANNING`
- `AUTHORIZED:<name>`
- `UNAUTHORIZED`
- `CAPTURING`
- `OFFLINE`

---

## 3. MATLAB Supervisory Interface

Provides a graphical control panel for system management.

### Capabilities

- System monitoring
- Command dispatch to Arduino/Python
- Sensor management
- User management
- PIN control
- Event log viewing
- Grace-period configuration

---

# Hardware Components

## Inputs

- PIR motion sensors
- Magnetic reed door switch
- Physical key switch
- Manual panic button
- Keypad

## Outputs

- Piezo siren
- Strobe warning light
- Solenoid door lock
- LED status indicators

---

# Pin Configuration

| Component | Pin |
|-----------|-----|
| PIR Sensor (Front) | 2 |
| Magnetic Switch | 3 |
| PIR Sensor (Window) | 4 |
| Physical Key | 5 |
| Door Solenoid | 6 |
| Siren | 7 |
| Manual Alarm Button | 10 |
| Green LED | 11 |
| Amber LED | 12 |
| Red LED / Strobe | 13 |

---

# Software Requirements

## Arduino

Install:

- Arduino IDE
- Required board drivers

Upload:

`BurglarAlarm/BurglarAlarm.ino`

---

## Python

Install dependencies:

```bash
pip install face_recognition opencv-python numpy
```

Run:

```bash
python face_recognition_system.py
```

---

## MATLAB

Open:

```matlab
BurglarAlarmApp.mlapp
```

Launch App Designer and run the application.

---

# Authentication Workflow

## Standard Access

1. User enters PIN
2. PIN validated
3. System disarms

---

## Physical Key Access

1. Key inserted
2. Facial verification requested
3. If face recognized:
   - Door unlocks
   - Access granted

Otherwise:

- Alarm triggered

---

# Security Features

## Intrusion Detection

Triggers alarm if:

- Motion detected while armed
- Door breach detected
- Unauthorized facial scan
- Failed authentication sequence

---

## Lockout Protection

After **3 failed PIN attempts**:

- System enters locked state
- Further access denied until reset

---

## Event Logging

Records:

- Alarm activations
- Failed login attempts
- Sensor changes
- PIN updates
- System shutdown/restart events
- User profile modifications

---

# Supported Commands

## MATLAB → Python

- `TRIGGER`
- `ADD:<name>`
- `REMOVE:<name>`
- `QUIT`

## MATLAB → Arduino

- `MENU`
- `VIEW_LOGS`
- `SHUTDOWN`
- `DIAG`
- `ADD_PROFILE`
- `CHANGE_PIN`
- `DISABLE_SENSOR`

---

# Key Design Features

- Modular object-oriented architecture
- Hardware abstraction layer
- Persistent configuration storage
- Real-time event-driven control
- Multi-factor authentication
- Desktop supervisory control integration

---

# Future Improvements

Potential extensions:

- Cloud event notifications
- Mobile app integration
- Database-backed user storage
- Encrypted credential storage

---

# Authors

**Rotimi Dayo as apart of Group 8**

Developed as an embedded systems and intelligent security systems project integrating:

- Embedded C++ (Arduino)
- Python Computer Vision
- MATLAB App Designer
- Hardware security control systems

---

# License

Academic / Educational Use
