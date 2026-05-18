//  User.h
//  Represents a system user with PIN and role.
//  Technician inherits from User (UML inheritance arrow).


#ifndef USER_H
#define USER_H

#include <Arduino.h>
#include "EventLog.h"


class User {
  private:
    String userID;
    String pin;
    String role;

  public:
    
    int gracePeriod;

    // Constructor 
    User() {
      userID = "";
      pin = "";
      role = "user";
      gracePeriod = 15;   // default 15s grace period 
    }

    User(String role, String userID, String pin) {
      this->role = role;
      this->userID = userID;
      this->pin = pin;
      this->gracePeriod = 15;
    }

    
    // Changes the user's PIN, returns new PIN 
    String changePIN(String newPin) {
      if (newPin.length() == 4) {
        pin = newPin;
        Serial.println("LOG:PIN changed for user: " + userID);
        return pin;
      }
      return "ERROR:PIN must be 4 digits";
    }

    
    // Sets the grace period for physical key entry 
    int setGracePeriod(int seconds) {
      gracePeriod = seconds;
      return gracePeriod;
    }

    
    // Allows user to view event logs 
    void viewLogs(EventLog& eventLog) {
      eventLog.getEvents();
    }

    // Getters 
    String  getUserID() { return userID; }
    String  getPIN() { return pin; }
    String  getRole() { return role; }
    bool validatePIN(String pin) { return pin == this->pin; }
};


//  Technician inherits from User 
//  should have additional diagnostic mode capabilities lul.
class Technician : public User {
  public:
    Technician() : User("technician", "TECH01", "0000") {}

    Technician(String userID, String pin)
      : User("technician", userID, pin) {}

    
    // Validates technician PIN and enters diagnostic mode
    bool enterDiagnosticMode(String pin) {
      if (validatePIN(pin)) {
        Serial.println("STATE:DIAGNOSTIC");
        Serial.println("MENU:DIAGNOSTIC");
        return true;
      }
      Serial.println("MSG:Invalid technician PIN");
      return false;
    }
   
    void exitDiagnosticMode() {
      Serial.println("STATE:ARMED");
      Serial.println("MSG:Exited diagnostic mode");
    }

    //placeholder for diagonostic functions
    void runDiagnosticTest() {
      Serial.println("MSG:Running diagnostic test...");
      testSensors();
      testAlarmOutputs();
    }
 
    // Tests all sensors [PLACEHOLDER] 
    void testSensors() {
      Serial.println("MSG:Testing sensors...");
      Serial.println("DIAG:SENSOR_TEST");
    }

    // Tests sirens and strobes [PLACEHOLDER] 
    void testAlarmOutputs() {
      Serial.println("MSG:Testing alarm outputs...");
      Serial.println("DIAG:ALARM_TEST");
    }
};

#endif