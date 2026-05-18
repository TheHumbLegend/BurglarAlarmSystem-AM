//  Sensor.h
//  Represents a physical sensor (PIR or magnetic switch).

#ifndef SENSOR_H
#define SENSOR_H

#include <Arduino.h>


class Sensor {
  private:
    String sensorID;
    bool isActive;
    bool Breach;
    int pin;
    bool usesPullup;

  public:
    Sensor() {
      sensorID = "";
      isActive = false;
      Breach = false;
      pin = -1;
      usesPullup = false;
    }

    Sensor(String sensorID, int pin, bool usesPullup = false) {
      this->sensorID = sensorID;
      this->pin = pin;
      isActive = true;
      Breach = false;
      this->usesPullup = usesPullup;
      if (usesPullup) {
        pinMode(pin, INPUT_PULLUP);
      } else {
        pinMode(pin, INPUT);
      }
    }

    // Reads the pin and returns breach status.
    // For PIR: HIGH = motion detected
    // For mag switch (INPUT_PULLUP): LOW = door/window opened
    bool detect() {
      if (!isActive || pin == -1) return false;
      int reading = digitalRead(pin);
      // INPUT_PULLUP sensors (mag switch): LOW = triggered
      // Normal INPUT sensors (PIR): HIGH = triggered
      if (usesPullup) {
        Breach = (reading == LOW);
      } else {
        Breach = (reading == HIGH);
      }
      return Breach;
    }

    //  activate(sensorID)
    void activate(String sensorID) {
      if (sensorID == this->sensorID) {
        isActive = true;
        Serial.println("LOG:Sensor activated: " + sensorID);
      }
    }

    //  deactivate(sensorID)
    void deactivate(String sensorID) {
      if (sensorID == this->sensorID) {
        isActive = false;
        Breach   = false;
        Serial.println("LOG:Sensor deactivated: " + sensorID);
      }
    }

    // Getters 
    bool isEnabled() { return isActive; }
    bool isTriggered() { return detect(); }
    bool isBreach() { return Breach; }
    String getSensorID() { return sensorID; }
    String getLocation() { return sensorID; }
    void disable() { isActive = false; }
    void enable() { isActive = true; }
};

#endif