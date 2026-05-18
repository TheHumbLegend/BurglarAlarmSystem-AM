//  BurglarAlarm.ino
//  Burglar Alarm System – Group 8
//  Full implementation

#include "DisplayPanel.h"
#include "Keypad.h"
#include "Sensor.h"
#include "Siren.h"
#include "StrobeLight.h"
#include "DoorLock.h"
#include "Database.h"
#include "EventLog.h"
#include "User.h"
#include "PhysicalKey.h"
#include "SystemController.h"

// Pin definitions
#define PIN_PIR 2
#define PIN_PIR_WINDOW  4
#define PIN_MAG_SWITCH  3
#define PIN_GREEN_LED   11
#define PIN_AMBER_LED   12
#define PIN_RED_LED     13
#define PIN_BUZZER       7
#define PIN_BUTTON      10
#define PIN_SOLENOID     6
#define PIN_PHYSICAL_KEY 5

// Hardware objects
Sensor sensors[] = {
  Sensor("Front Door", PIN_MAG_SWITCH, true),
  Sensor("Front PIR", PIN_PIR, false),
  Sensor("Window PIR", PIN_PIR_WINDOW, false)
};
int sensorCount = 3;

Siren sirens[] = {
  Siren("Main Siren", PIN_BUZZER)
};
int sirenCount = 1;

StrobeLight strobeLights[] = {
  StrobeLight("Main Strobe", PIN_RED_LED)
};
int strobeLightCount = 1;

DoorLock    doorLock(PIN_SOLENOID, -1);
PhysicalKey physicalKey(PIN_PHYSICAL_KEY);

// System Controller
SystemController controller(
  sensors, 
  sensorCount,
  sirens,
  sirenCount,
  strobeLights,
  strobeLightCount,
  doorLock,
  physicalKey
);

// Manual alarm button debounce
int lastButtonReading = HIGH;
int buttonState       = HIGH;
unsigned long lastDebounceTime  = 0;
#define DEBOUNCE_MS 50


void setup() {
  Serial.begin(9600);

  pinMode(PIN_GREEN_LED, OUTPUT);
  pinMode(PIN_AMBER_LED, OUTPUT);
  pinMode(PIN_RED_LED, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_SOLENOID, OUTPUT);
  pinMode(PIN_BUTTON, INPUT_PULLUP);

  allOutputsOff();
  delay(1000);

  controller.armSystem();
  digitalWrite(PIN_SOLENOID, LOW);
  digitalWrite(PIN_GREEN_LED, HIGH);
}

// Main loop
void loop() {
  handleSerial();
  controller.checkSensors();
  controller.checkPhysicalKey();
  handleManualButton();
  controller.checkAlarmTimeout();
  controller.updateOutputs();
  updateLEDs();
}

// Serial handler
void handleSerial() {
  if (Serial.available()) {
    String msg = Serial.readStringUntil('\n');
    msg.trim();
    if (msg.length() > 0) {
      controller.handleSerialInput(msg);
    }
  }
}

// Manual alarm button
void handleManualButton() {
  int reading = digitalRead(PIN_BUTTON);
  if (reading != lastButtonReading) lastDebounceTime = millis();
  lastButtonReading = reading;

  if ((millis() - lastDebounceTime) > DEBOUNCE_MS) {
    if (reading != buttonState) {
      buttonState = reading;
      if (buttonState == LOW) {
        controller.triggerManualAlarm();
      }
    }
  }
}

// LED status indicators
void updateLEDs() {
  switch (controller.getState()) {
    case ARMED:
      digitalWrite(PIN_GREEN_LED, HIGH);
      digitalWrite(PIN_AMBER_LED, LOW);
      break;
    case DISARMED:
      digitalWrite(PIN_GREEN_LED, LOW);
      digitalWrite(PIN_AMBER_LED, HIGH);
      break;
    case ALARM_TRIGGERED:
      digitalWrite(PIN_GREEN_LED, LOW);
      digitalWrite(PIN_AMBER_LED, (millis() / 500) % 2);
      break;
    case DIAGNOSTIC:
      digitalWrite(PIN_GREEN_LED, LOW);
      digitalWrite(PIN_AMBER_LED, HIGH);
      break;
    case SHUTDOWN:
      allOutputsOff();
      break;
  }
}

// All outputs off
void allOutputsOff() {
  digitalWrite(PIN_GREEN_LED, LOW);
  digitalWrite(PIN_AMBER_LED, LOW);
  digitalWrite(PIN_RED_LED,  LOW);
  digitalWrite(PIN_BUZZER,  LOW);
  digitalWrite(PIN_SOLENOID, LOW);
}