//  SystemController.h
//  Central controller - coordinates all subsystems.
#ifndef SYSTEM_CONTROLLER_H
#define SYSTEM_CONTROLLER_H

#include <Arduino.h>
#include "DisplayPanel.h"
#include "Keypad.h"
#include "EventLog.h"
#include "DoorLock.h"
#include "Database.h"
#include "User.h"
#include "Sensor.h"
#include "Siren.h"
#include "StrobeLight.h"
#include "PhysicalKey.h"

enum SystemState {
  ARMED,
  DISARMED,
  ALARM_TRIGGERED,
  DIAGNOSTIC,
  SHUTDOWN
};

class SystemController {
  private:
    SystemState state;
    Sensor* sensor;
    int sensorCount;
    Siren* siren;
    int   sirenCount;
    StrobeLight*  strobelight;
    int  strobeLightCount;
    Keypad keypad;
    DisplayPanel display;
    EventLog eventLog;
    DoorLock doorLock;
    Database database;
    PhysicalKey physicalKey;

    int failedPINCount;
    bool systemLocked;
    unsigned long alarmStartTime;
    String event;

    // Private methods

    void triggerAlarm() {
      if (state == DIAGNOSTIC) return;
      state = ALARM_TRIGGERED;
      alarmStartTime = millis();
      for (int i = 0; i < sirenCount; i++)        siren[i].activate();
      for (int i = 0; i < strobeLightCount; i++)  strobelight[i].flash();
      logEvent("ALARM TRIGGERED: " + event);
      Serial.println("ALARM:TRIGGERED");
      Serial.println("STATE:ALARM");
      display.displayMessage("!! ALARM !!");
    }

    void disarmSystem(String pin) {
      if (systemLocked) {
        display.displayMessage("System locked");
        Serial.println("MSG:System locked - contact support");
        return;
      }
      if (database.validatePIN(pin)) {
        state = DISARMED;
        failedPINCount = 0;
        display.displayMessage("System Disarmed");
        logEvent("System disarmed by: " + database.getLastUserID());
        Serial.println("STATE:DISARMED");
        showMainMenu();
      } else {
        failedPINCount++;
        logEvent("Failed PIN attempt: " + String(failedPINCount));
        if (failedPINCount >= 3) {
          systemLocked = true;
          display.displayMessage("Too many attempts");
          Serial.println("MSG:System locked - contact support");
          logEvent("System locked - too many failed PINs");
        } else {
          display.displayMessage("Wrong PIN - Try again");
          Serial.println("MSG:Wrong PIN - " +
            String(3 - failedPINCount) + " attempts left");
        }
      }
    }

    bool validatePINForMenu(String pin) {
      if (systemLocked) {
        Serial.println("MSG:System locked - contact support");
        return false;
      }
      if (database.validatePIN(pin)) {
        failedPINCount = 0;
        logEvent("Menu accessed by: " + database.getLastUserID());
        Serial.println("STATE:MENU");
        showMainMenu();
        return true;
      } else {
        failedPINCount++;
        if (failedPINCount >= 3) {
          systemLocked = true;
          Serial.println("MSG:System locked - contact support");
          logEvent("System locked - too many failed PINs");
        } else {
          Serial.println("MSG:Wrong PIN - " +
            String(3 - failedPINCount) + " attempts left");
        }
        return false;
      }
    }

    void deactivateAlarm(String pin) {
      if (database.validatePIN(pin)) {
        for (int i = 0; i < sirenCount; i++)    siren[i].stop();
        for (int i = 0; i < strobeLightCount; i++) strobelight[i].stop();
        state = ARMED;
        logEvent("Alarm deactivated");
        display.displayMessage("Alarm Deactivated");
        Serial.println("STATE:ARMED");
      } else {
        display.displayMessage("Wrong PIN");
        Serial.println("MSG:Wrong PIN");
      }
    }

    void checkAuthorisation() {
      if (physicalKey.getIsKeyInserted()) {
        physicalKey.startVerification(15);
      } else {
        triggerAlarm();
      }
    }

    void setSensorState(Sensor sensors[], int count) {
      for (int i = 0; i < count && i < sensorCount; i++) {
        sensor[i] = sensors[i];
      }
      logEvent("Sensor state updated");
    }

    void shutdown(String pin) {
      if (database.validatePIN(pin)) {
        for (int i = 0; i < sensorCount; i++)      sensor[i].disable();
        for (int i = 0; i < sirenCount; i++)        siren[i].stop();
        for (int i = 0; i < strobeLightCount; i++) strobelight[i].stop();
        state = SHUTDOWN;
        logEvent("System shutdown");
        display.displayMessage("System Shutdown");
        Serial.println("STATE:SHUTDOWN");
      } else {
        display.displayMessage("Wrong PIN");
        Serial.println("MSG:Wrong PIN");
      }
    }

    void restart(String pin) {
      if (database.validatePIN(pin)) {
        for (int i = 0; i < sensorCount; i++) sensor[i].enable();
        armSystem();
      } else {
        Serial.println("MSG:Wrong PIN");
      }
    }

    String logEvent(String eventMsg) {
      eventLog.addEntry(eventMsg);
      return eventMsg;
    }

    String notifyUser() {
      Serial.println("NOTIFY:" + event);
      return event;
    }

    void handleCommand(String cmd) {
      if (cmd == "MENU") {
        showMainMenu();

      } else if (cmd == "VIEW_LOGS") {
        eventLog.getEvents();

      } else if (cmd == "SHUTDOWN") {
        Serial.println("PROMPT:ENTER_PIN_SHUTDOWN");

      } else if (cmd == "DIAG") {
        state = DIAGNOSTIC;
        showDiagnosticMenu();

      } else if (cmd == "EXIT_DIAG") {
        state = ARMED;
        armSystem();

      } else if (cmd == "ADD_PROFILE") {
        logEvent("Add profile initiated");
        Serial.println("MSG:Look at camera and press SPACE");

      } else if (cmd == "DISABLE_SENSOR") {
        for (int i = 0; i < sensorCount; i++) {
          Serial.println("SENSOR_LIST:" + sensor[i].getLocation() +
                        ":" + (sensor[i].isEnabled() ? "ON" : "OFF"));
        }
        Serial.println("PROMPT:SELECT_SENSOR_TO_DISABLE");

      } else if (cmd == "CHANGE_PIN") {
        Serial.println("PROMPT:ENTER_CURRENT_PIN");

      } else if (cmd.startsWith("CHANGE_PIN_VERIFY:")) {
        String params  = cmd.substring(18);
        int    colon   = params.indexOf(':');
        String oldPIN  = params.substring(0, colon);
        String newPIN  = params.substring(colon + 1);
        if (database.validatePIN(oldPIN)) {
          for (int i = 0; i < database.getUserCount(); i++) {
            if (database.getUser(i).validatePIN(oldPIN)) {
              database.getUser(i).changePIN(newPIN);
              logEvent("PIN changed successfully");
              Serial.println("MSG:PIN changed successfully");
              break;
            }
          }
        } else {
          Serial.println("MSG:Wrong current PIN");
        }

      } else if (cmd.startsWith("TOGGLE_SENSOR:")) {
        String sensorName = cmd.substring(14);
        for (int i = 0; i < sensorCount; i++) {
          if (sensor[i].getLocation() == sensorName) {
            if (sensor[i].isEnabled()) {
              sensor[i].disable();
              Serial.println("MSG:" + sensorName + " disabled");
              logEvent("Sensor disabled: " + sensorName);
            } else {
              sensor[i].enable();
              Serial.println("MSG:" + sensorName + " enabled");
              logEvent("Sensor enabled: " + sensorName);
            }
            break;
          }
        }
      }
    }

    void showMainMenu() {
      String options[] = {
        " Add Facial Profile",
        " Remove Facial Profile",
        " Enable/Disable Sensor",
        " View Logs",
        " Exit Session",
        " Change PIN",
        " Set Grace Period"
      };
      display.displayMenu(options, 7);
    }

    void showDiagnosticMenu() {
      String options[] = {
        " View Event Logs",
        " Sensor Check",
        " Siren Test",
        " Strobe Test",
        " Disable Sensors",
        " Update System",
        " Exit Diagnostic"
      };
      display.displayMenu(options, 7);
      Serial.println("STATE:DIAGNOSTIC");
    }

  public:

    SystemController(
      Sensor* sensors,
      int sensorCount,
      Siren* sirens,
      int sirenCount,
      StrobeLight* strobeLights,
      int strobeLightCount,
      DoorLock doorLock,
      PhysicalKey physicalKey
    ) {
      sensor  = sensors;
      this->sensorCount = sensorCount;
      siren = sirens;
      this->sirenCount = sirenCount;
      strobelight = strobeLights;
      this->strobeLightCount = strobeLightCount;
      this->doorLock = doorLock;
      this->physicalKey = physicalKey;
      state = ARMED;
      failedPINCount = 0;
      systemLocked = false;
      alarmStartTime = 0;
      event = "";
    }

    void armSystem() {
      state = ARMED;
      failedPINCount = 0;
      systemLocked = false;
      doorLock.lock();
      display.displayMessage("System Armed");
      logEvent("System armed");
      Serial.println("STATE:ARMED");
    }

    void triggerManualAlarm() {
      event = "Manual alarm button";
      triggerAlarm();
    }

    void checkSensors() {
      if (state != ARMED) return;
      for (int i = 0; i < sensorCount; i++) {
        if (sensor[i].isEnabled() && sensor[i].isTriggered()) {
          event = sensor[i].getLocation();
          logEvent("Sensor triggered: " + event);
          Serial.println("SENSOR:" + event);

          if (event == "Front PIR") {
            // PIR1 - trigger camera for facial recognition
            Serial.println("PIR:CAMERA");
          } else if (event == "Window PIR") {
            // PIR2 - straight to alarm
            triggerAlarm();
          } else if (event == "Front Door") {
            // Mag switch - check authorisation
            checkAuthorisation();
          }
          break;
        }
      }
    }

    void checkPhysicalKey() {
    if (physicalKey.getIsKeyInserted() && state == ARMED) {
        doorLock.unlock();
        int grace = database.getUser(0).gracePeriod;
        physicalKey.startVerification(grace);
        display.turnOn();
        display.displayMessage("Enter PIN - " + String(grace) + " seconds");
        Serial.println("PROMPT:ENTER_PIN");  // ← ADD THIS
    }
    if (physicalKey.getIsVerificationActive()) {
        bool timedOut = physicalKey.update();
        if (timedOut) {
            event = "Physical key timeout";
            triggerAlarm();
        }
    }
}

    void handleSerialInput(String msg) {
      msg.trim();
      // PIN loaded from MATLAB file on connect
      if (msg.startsWith("PIN_SET:")) {
        String pin = msg.substring(8);
        for (int i = 0; i < database.getUserCount(); i++) {
          if (database.getUser(i).getRole() == "user") {
            database.getUser(i).changePIN(pin);
            Serial.println("LOG:PIN loaded from MATLAB");
            break;
          }
        }

      // Grace period loaded from MATLAB file on connect 
      } else if (msg.startsWith("GRACE_SET:")) {
        int seconds = msg.substring(10).toInt();
        if (seconds > 0) {
          for (int i = 0; i < database.getUserCount(); i++) {
            if (database.getUser(i).getRole() == "user") {
              database.getUser(i).setGracePeriod(seconds);
              Serial.println("LOG:Grace period set to " + String(seconds) + "s");
              break;
            }
          }
        }

      // PIN entry 
      } else if (msg.startsWith("PIN:")) {
        String pin = msg.substring(4);
        if (state == ALARM_TRIGGERED) {
          deactivateAlarm(pin);
        } else if (physicalKey.getIsVerificationActive()) {
          if (database.validatePIN(pin)) {
            physicalKey.verifySuccess();
            doorLock.unlock();
            logEvent("Physical key access granted");
          } else {
            triggerAlarm();
          }
        } else if (state == SHUTDOWN) {
          restart(pin);
        } else {
          // Check technician PIN first
          if (database.validateTechnicianPIN(pin)) {
            state = DIAGNOSTIC;
            logEvent("Technician accessed diagnostic mode");
            showDiagnosticMenu();
            Serial.println("STATE:DIAGNOSTIC");
          } else {
            validatePINForMenu(pin);
          }
        }

      //  Face recognition results 
      } else if (msg == "FACE:AUTHORIZED") {
        doorLock.unlock();
        logEvent("Face recognised - door unlocked");
        display.displayMessage("Welcome");
        Serial.println("MSG:Door Unlocked");

      } else if (msg == "FACE:UNAUTHORIZED") {
        logEvent("Unknown face detected");
        display.displayMessage("Unknown face");
        Serial.println("PROMPT:ENTER_PIN");

      //  CMD routing 
      } else if (msg.startsWith("CMD:")) {
        handleCommand(msg.substring(4));

      //  User management
      } else if (msg.startsWith("ADD_USER:")) {
        String params = msg.substring(9);
        int    colon  = params.indexOf(':');
        String userID = params.substring(0, colon);
        String pin    = params.substring(colon + 1);
        database.addUser(userID, "user", pin);
        database.addFace(userID);

      } else if (msg.startsWith("REMOVE_USER:")) {
        database.removeUser(msg.substring(12));
      }
    }

    void checkAlarmTimeout() {
      if (state == ALARM_TRIGGERED &&
          millis() - alarmStartTime >= 1200000UL) {
        for (int i = 0; i < sirenCount; i++)       siren[i].stop();
        for (int i = 0; i < strobeLightCount; i++) strobelight[i].stop();
        armSystem();
        Serial.println("MSG:Alarm auto-deactivated after 20 mins");
      }
    }

    void updateOutputs() {
      doorLock.update();
      for (int i = 0; i < sirenCount; i++)   siren[i].update();
      for (int i = 0; i < strobeLightCount; i++) strobelight[i].update();
      display.checkTimeout();
    }

    SystemState getState()  { return state; }
    DisplayPanel& getDisplay()  { return display; }
    EventLog& getEventLog() { return eventLog; }
    Database& getDatabase() { return database; }
};

#endif