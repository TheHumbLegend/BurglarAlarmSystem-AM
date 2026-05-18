//  Siren.h
//  Represents the buzzer/siren output.

#ifndef SIREN_H
#define SIREN_H
#include <Arduino.h>


class Siren {
  private:
    String sirenID;
    int soundLevel;
    bool isActive;
    int pin;
    unsigned long lastToggleTime;
    bool pinState;
    int flashFrequency;

  public:
    Siren() {
      sirenID = "";
      soundLevel = 0;
      isActive = false;
      pin = -1;
    }

    Siren(String sirenID, int pin) {
      this-> sirenID = sirenID;
      this-> pin = pin;
      soundLevel = 0;
      isActive = false;
      lastToggleTime = 0;
      pinState = false;
      flashFrequency = 1;
      pinMode(pin, OUTPUT);
      digitalWrite(pin, LOW);
    }

    
    // Activates the siren at the given sound level 
    void sound(int soundLevel) {
      this-> soundLevel = soundLevel;
      isActive = true;
      Serial.println("LOG:Siren activated: " + sirenID);
    }

    // stop siren and turns off sound
    void stop() {
      isActive = false;
      pinState = false;
      digitalWrite(pin, LOW);
      Serial.println("LOG:Siren stopped: " + sirenID);
    }

    
    void update() {
      if (!isActive || pin == -1) return;
      unsigned long interval = 1000 / (flashFrequency * 2);
     if (millis() - lastToggleTime >= interval) {
      pinState = !pinState;
      lastToggleTime = millis();
      digitalWrite(pin, pinState ? HIGH : LOW);
  }
}

    
    // Convenience wrappers used by SystemController
    void activate() { sound(100); }
    void deactivate() { stop(); }

    // Getters
    bool getisActive() { return isActive; }
    String getSirenID() { return sirenID; }
};

#endif