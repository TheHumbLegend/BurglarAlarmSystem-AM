//  StrobeLight.h
//  Represents the strobe LED output.

#ifndef STROBE_LIGHT_H
#define STROBE_LIGHT_H

#include <Arduino.h>

class StrobeLight {
  private:
    String lightID;
    int flashFrequency;   // in Hz
    int intensity;
    bool isFlashing;
    int pin;

    unsigned long lastFlashTime;
    bool pinState;

  public:
    StrobeLight() {
      lightID = "";
      flashFrequency = 1;
      intensity = 0;
      isFlashing = false;
      pin = -1;
      lastFlashTime  = 0;
      pinState = false;
    }

    StrobeLight(String lightID, int pin) {
      this->lightID = lightID;
      this->pin = pin;
      flashFrequency = 1;   // default 1Hz 
      intensity = 0;
      isFlashing = false;
      lastFlashTime  = 0;
      pinState = false;
      pinMode(pin, OUTPUT);
      digitalWrite(pin, LOW);
    }

    
    // Starts flashing at given frequency 
    void flash(int flashFrequency, int intensity) {
      this->flashFrequency = flashFrequency;
      this->intensity = intensity;
      isFlashing = true;
      Serial.println("LOG:Strobe activated: " + lightID);
    }
    
    // MUST be called every loop() iteration to produce flash.
    void update() {
      if (!isFlashing || pin == -1) return;
      unsigned long interval = 1000 / (flashFrequency * 2);
      if (millis() - lastFlashTime >= interval) {
        pinState = !pinState;
        lastFlashTime = millis();
        digitalWrite(pin, pinState ? HIGH : LOW);
      }
    }

    //stops flashing and turns off light
    void stop() {
      isFlashing = false;
      pinState = false;
      if (pin != -1) digitalWrite(pin, LOW);
      Serial.println("LOG:Strobe stopped: " + lightID);
    }

    // Convenience wrappers 
    void flash() { flash(1, 100); }   // default 1Hz

    // Getters 
    bool getisFlashing() { return isFlashing; }
    String getLightID() { return lightID; }
};

#endif