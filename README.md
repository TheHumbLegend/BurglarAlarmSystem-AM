# BurglarAlarmSystem-AM

A multi-layer intelligent burglar alarm and access control system integrating Arduino hardware control, Python-based facial biometric authentication, and a MATLAB supervisory control and user interface.

The system combines physical intrusion detection, biometric authentication, PIN-based access control, physical-key authentication, alarm handling, and event logging into a modular embedded security platform made using OOP techniques.

---

<img width="518" height="609" alt="Burglar Alarm System" src="https://github.com/user-attachments/assets/b4b9d15b-f4eb-4210-835c-2b7fbedeb5c4" />

## Overview

The system monitors a secured environment using:

* Magnetic door sensors
* Passive Infrared (PIR) motion sensors
* Physical key authentication
* PIN-based keypad authentication
* Camera-based facial recognition

The system operates using multiple authentication and security paths depending on how access is initiated.

### Normal Operation

During normal operation, the system continuously monitors the environment for movement or changes in the secured entry points.

When motion or another relevant security event is detected:

1. The camera is activated for facial recognition.
2. The system attempts to identify the person.
3. If an **authorised user** is recognised:

   * Access is granted.
   * The solenoid door lock is opened.
4. If the person is **not authorised**:

   * Access is denied.
   * The door remains locked.

### Physical Key Authentication - Using a button to simulate a physical key

The physical key provides an alternative access route.

When the physical key is used:

1. The physical key switch is detected.
2. The door lock is opened.
3. A **15-second authentication window** begins.
4. The user must enter the correct PIN using the keypad.
5. If the PIN is correct:

   * Authentication succeeds.
   * Normal operation continues.
6. If the PIN is incorrect or the 15-second window expires:

   * Authentication fails.
   * The alarm is triggered.

This provides a second layer of authentication even when physical-key access is used.

---

# System Architecture

The project consists of three integrated subsystems:

## 1. Arduino Embedded Controller

The Arduino acts as the primary hardware controller and handles real-time sensor monitoring, authentication inputs, alarm logic, and physical security hardware.

### Responsibilities

* Monitor intrusion sensors
* Detect physical key usage
* Process keypad input
* Control:

  * Siren
  * Strobe light
  * Solenoid door lock
  * Status LEDs
* Manage authentication timers
* Execute system state transitions
* Maintain security and alarm logic
* Communicate with MATLAB over serial

### System States

The Arduino manages several system states, including:

* **Armed**
* **Disarmed**
* **Authentication**
* **Alarm Triggered**
* **Diagnostic Mode**
* **Shutdown**

---

## 2. Python Facial Recognition Engine

The Python subsystem provides camera-based biometric authentication using **OpenCV** and the `face_recognition` library.

### Features

* Continuous camera monitoring when required
* Motion-triggered facial recognition
* Authorised user recognition
* Unauthorised access detection
* Dynamic user profile management
* Status reporting to MATLAB

### Recognition States

The facial recognition system reports states such as:

* `IDLE`
* `SCANNING`
* `AUTHORIZED:<name>`
* `UNAUTHORIZED`
* `CAPTURING`
* `OFFLINE`

### Facial Authentication Process

When the system detects a relevant access or motion event:

* Camera is activated
* Face detection is performed
* Detected face is compared against authorised user profiles
* If the user is **authorised**:

  * Authentication is successful
  * Door lock opens
  * Access is granted
* If the user is **not authorised**:

  * Authentication fails
  * Door remains locked
  * Security response is initiated

---

## 3. MATLAB Supervisory Interface

MATLAB provides the supervisory graphical user interface for monitoring and controlling the complete system.

### Capabilities

* Real-time system monitoring
* Arduino command dispatch
* Python facial-recognition control
* Sensor management
* User profile management
* PIN management
* Event-log viewing
* System diagnostics
* Grace-period configuration
* System shutdown

---

# Hardware Components

## Inputs

* PIR motion sensors
* Magnetic reed switch
* Physical key switch
* Manual panic/alarm button
* Keypad

## Outputs

* Piezo siren
* Strobe warning light
* Solenoid door lock
* Green status LED
* Amber status LED
* Red status LED

---

# Pin Configuration

| Component           | Arduino Pin |
| ------------------- | ----------- |
| PIR Sensor (Front)  | 2           |
| Magnetic Switch     | 3           |
| PIR Sensor (Window) | 4           |
| Physical Key        | 5           |
| Door Solenoid       | 6           |
| Siren               | 7           |
| Manual Alarm Button | 10          |
| Green LED           | 11          |
| Amber LED           | 12          |
| Red LED / Strobe    | 13          |

---

# Authentication System

The system uses multiple authentication methods depending on the access method.

## Facial Recognition Authentication

Facial recognition is the primary automated authentication mechanism.

### Process

1. PIR or another relevant sensor detects activity.
2. The camera is activated.
3. Python performs facial recognition.
4. The detected face is compared against authorised profiles.
5. If the user is recognised:

   * Authentication state changes to `AUTHORIZED:<name>`.
   * Access is granted.
   * The solenoid door lock opens.
6. If the user is not recognised:

   * Authentication is denied.
   * The solenoid remains locked.
   * The system can initiate the configured security response.

---

## Physical Key + PIN Authentication

The physical key provides an alternative method of entering the secured area, but does **not** provide complete authentication by itself.

* Physical key is used
* Door lock opens
* A **15-second PIN authentication timer** starts
* User must enter the correct PIN within 15 seconds
* If the PIN is correct:

  * Authentication is successful
  * Access is granted
* If the PIN is incorrect or the 15-second timer expires:

  * Authentication fails
  * Alarm is triggered

---

### Successful Authentication

If the correct PIN is entered within **15 seconds**:

* Authentication succeeds.
* The alarm is not triggered.
* The system returns to normal operation.

### Failed Authentication

If the PIN is incorrect or the 15-second authentication period expires:

* Authentication fails.
* The alarm is triggered.
* Access is treated as unauthorised.

---

# Intrusion Detection

The system can detect potential intrusion through multiple inputs.

### Detection Sources

* Front PIR motion sensor
* Window PIR motion sensor
* Magnetic door switch
* Manual alarm button
* Failed authentication

Depending on the current system state, a detected event can trigger the authentication process or directly activate the alarm.

---

# Alarm System

When an intrusion or failed authentication is detected, the system can:

* Activate the siren
* Activate the strobe warning light
* Keep the door locked
* Record the event
* Notify the MATLAB supervisory interface

The alarm provides a physical indication of a security breach while the MATLAB interface provides supervisory visibility.

---

# Lockout Protection

The system provides protection against repeated failed PIN attempts.

After **3 failed PIN attempts**:

* The system enters a locked state.
* Further authentication attempts are denied.
* The system must be reset or returned to an authorised state before access can continue.

---

# Event Logging

The system records important security and operational events, including:

* Alarm activations
* Failed authentication attempts
* Successful authentication
* Sensor state changes
* PIN updates
* Physical-key usage
* User profile additions/removals
* System startup and shutdown events
* Diagnostic events

These events can be viewed through the MATLAB supervisory interface.

---

# Communication

The system uses serial communication to between the Arduino and the MATLAB App **same COM as the Arduino**, and since serial is a one to one connection the MATLAB and Python facial recognition script communicated via `command.txt` and `status.etxt` where both programs poll (read/write) for instructions. The persistent pin `pin.txt` is polled by MATLAB and sent to Arduino on startup when button "Connect" is pressed.

## MATLAB → Python

Supported commands include:

* `TRIGGER`
* `ADD:<name>`
* `REMOVE:<name>`
* `QUIT`

## MATLAB → Arduino

Supported commands include:

* `MENU`
* `VIEW_LOGS`
* `SHUTDOWN`
* `DIAG`
* `ADD_PROFILE`
* `CHANGE_PIN`
* `DISABLE_SENSOR`

---

# Software Requirements

## Arduino

Install:

* Arduino IDE
* Required Arduino board drivers

Upload:

```text
BurglarAlarm/BurglarAlarm.ino
```

---

## Python

Install the required dependencies:

```bash
pip install face_recognition opencv-python numpy
```

Run the facial recognition system:

```bash
python face_recognition_system.py
```

---

## MATLAB

Open:

```text
BurglarAlarmApp.mlapp
```

Launch the application using **MATLAB App Designer**.

---

# Key Design Features

* Modular multi-subsystem architecture
* Embedded C++ hardware control
* Python computer vision and facial recognition
* MATLAB supervisory control interface
* Real-time sensor monitoring
* Event-driven security logic
* Multi-factor authentication
* Physical-key + PIN authentication
* Motion-triggered biometric authentication
* Configurable authentication timeout
* Persistent configuration and user profiles
* Security event logging
* Hardware abstraction and modular control

---

# UML ACTIVITY AND CLASS DIAGRAMS

<img width="1332" height="807" alt="image" src="https://github.com/user-attachments/assets/064f3163-8d43-4f31-a5de-83f6a3ef91dd" />

link: [UML ACTIVITY DIAGRAM](urlhttps://lucid.app/lucidchart/d53ff269-30cf-4212-b2a1-4b61f015c277/edit?viewport_loc=-3483%2C-4026%2C7555%2C4215%2CKFllJB8AmW0b8&invitationId=inv_888bfe6b-80bd-4d2a-af8b-98526591908a)

<img width="1245" height="903" alt="image" src="https://github.com/user-attachments/assets/eb9fe364-ce40-4c1b-9b87-005456c60053" />

link: [UML CLASS DIAGRAM](https://lucid.app/lucidchart/1be6e73a-b649-4059-b941-7e7a2e450517/edit?viewport_loc=-1320%2C483%2C5709%2C3124%2CHWEp-vi-RSFO&invitationId=inv_f920b3eb-c317-4d24-9a84-d1265397ad72)

---

# Future Improvements

Potential extensions include:

** SHOULD NOT HAVE USE MATLAB AS A GUI, very annoying to use for a project like this. USE PYTHON **

* Cloud-based security notifications
* Mobile application integration
* Database-backed user management
* Encrypted credential storage
* Remote system monitoring
* Additional biometric authentication methods
* Improved anti-spoofing measures for facial recognition
* Network-connected security monitoring

---

# Authors

**Rotimi Dayo — Group 8**

Thoughts: Pretty long project, couldn't implement all the things we planned too because of time constraint. Should have used python as the GUI. 

Embedded systems and intelligent security systems project integrating:

* Embedded C++
* Arduino hardware control
* Python computer vision
* OpenCV and facial recognition
* MATLAB App Designer
* Serial communication
* Hardware security and access control

---

# License

**Academic / Educational Use**
