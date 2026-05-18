//  PhysicalKey.h
//  Simulates a physical key using a button on the breadboard.
//  When pressed, starts a 15 second PIN verification period.


#ifndef PHYSICAL_KEY_H
#define PHYSICAL_KEY_H

#include <Arduino.h>


class PhysicalKey {
  private:
    bool iskeyInserted;
    int verificationPeriod;  // = 15 seconds 
    bool verificationActive;
    int pin;                 // Arduino pin for button

    unsigned long verificationStart;
    int lastReading;
    int buttonState;
    unsigned long lastDebounceTime;

  public:
    PhysicalKey() {
      iskeyInserted = false;
      verificationPeriod = 15;
      verificationActive = false;
      pin = -1;
      verificationStart  = 0;
      lastReading = HIGH;
      buttonState = HIGH;
      lastDebounceTime = 0;
    }

    PhysicalKey(int pin) {
      this->pin = pin;
      iskeyInserted = false;
      verificationPeriod = 15;
      verificationActive = false;
      verificationStart  = 0;
      lastReading = HIGH;
      buttonState = HIGH;
      lastDebounceTime = 0;
      pinMode(pin, INPUT_PULLUP);
    }

   
    // Checks if button is pressed (simulating key insertion)
    // Uses software debounce. Returns true on press event.
    bool keyInserted() {
      if (pin == -1) return false;

      int reading = digitalRead(pin);

      if (reading != lastReading) {
        lastDebounceTime = millis();
      }
      lastReading = reading;

      if ((millis() - lastDebounceTime) > 50) {
        if (reading != buttonState) {
          buttonState = reading;
          if (buttonState == LOW) {
            // Button pressed - key inserted
            iskeyInserted = true;
            Serial.println("LOG:Physical key used");
            return true;
          }
        }
      }
      return false;
    }

  
    // Starts the 15 second PIN verification countdown 
    void startVerification(int verificationPeriod) {
      this->verificationPeriod = verificationPeriod;
      verificationActive = true;
      verificationStart  = millis();
      Serial.println("PROMPT:ENTER_PIN");
      Serial.println("TIMER:15");  // MATLAB displays countdown
      Serial.println("MSG:Physical key used - Enter PIN");
    }

   
    // Call in main loop. Returns true if verification timed out.
    bool update() {
      if (!verificationActive) return false;

      unsigned long elapsed = millis() - verificationStart;
      int remaining = verificationPeriod - (elapsed / 1000);

      // Send countdown to MATLAB every second
      static int lastRemaining = -1;
      if (remaining != lastRemaining && remaining >= 0) {
        lastRemaining = remaining;
        Serial.println("TIMER:" + String(remaining));
      }

      // Timed out
      if (elapsed >= (unsigned long)verificationPeriod * 1000) {
        verificationActive = false;
        iskeyInserted = false;
        lastRemaining = -1;
        Serial.println("LOG:Verification timeout - alarm triggered");
        return true;  // caller should trigger alarm
      }
      return false;
    }

    
    // Call when correct PIN entered during verification 
    void verifySuccess() {
      verificationActive = false;
      iskeyInserted      = false;
      Serial.println("LOG:Physical key verification successful");
      Serial.println("MSG:Access granted");
    }

    // Getters
    bool getIsVerificationActive() { return verificationActive; }
    bool getIsKeyInserted() { return iskeyInserted; }
    int getVerificationPeriod(){ return verificationPeriod; }
};

#endif