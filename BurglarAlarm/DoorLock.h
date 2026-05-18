//  DoorLock.h
//  Controls the solenoid door lock.

#ifndef DOOR_LOCK_H
#define DOOR_LOCK_H

#include <Arduino.h>


class DoorLock {
  private:
    bool IsLocked;
    int unlockDuration;    
    int CuesoundLevel;     
    int solenoidPin;
    int buzzerPin;

    unsigned long unlockStartTime;
    bool timerActive;

  public:
    DoorLock() {
      IsLocked = true;
      unlockDuration = 5000;   // default 5 seconds
      CuesoundLevel  = 1;
      solenoidPin  = -1;
      buzzerPin = -1;
      unlockStartTime = 0;
      timerActive = false;
    }

    DoorLock(int solenoidPin, int buzzerPin) {
      this->solenoidPin = solenoidPin;
      this->buzzerPin = buzzerPin;
      IsLocked = true;
      unlockDuration = 5000;
      CuesoundLevel  = 1;
      unlockStartTime = 0;
      timerActive   = false;
      pinMode(solenoidPin, OUTPUT);
      digitalWrite(solenoidPin, LOW);
    }

    
    // Locks the door - deactivates solenoid
    void lockDoor() {
      IsLocked    = true;
      timerActive = false;
      if (solenoidPin != -1) digitalWrite(solenoidPin, LOW);
      Serial.println("LOG:Door locked");
      Serial.println("MSG:Door Locked");
    }

  
    // Unlocks door for gracePeriod ms 
    void unlockDoor(int gracePeriod) {
      IsLocked        = false;
      unlockDuration  = gracePeriod;
      unlockStartTime = millis();
      timerActive     = true;
      if (solenoidPin != -1) digitalWrite(solenoidPin, HIGH);
      unlockSoundCue(CuesoundLevel);
      Serial.println("LOG:Door unlocked");
      Serial.println("MSG:Door Unlocked");
    }

    
    // Plays audio cue when door unlocks 
    void unlockSoundCue(int CuesoundLevel) {
      this->CuesoundLevel = CuesoundLevel;
      if (buzzerPin != -1) {
        // Short double beep
        digitalWrite(buzzerPin, HIGH); delay(100);
        digitalWrite(buzzerPin, LOW);  delay(100);
        digitalWrite(buzzerPin, HIGH); delay(100);
        digitalWrite(buzzerPin, LOW);
      }
    }

    
    // Call in main loop  auto locks after unlock duration
    void update() {
      if (timerActive && !IsLocked) {
        if (millis() - unlockStartTime >= (unsigned long)unlockDuration) {
          lockDoor();
        }
      }
    }

    // Wrappers
    void unlock() { unlockDoor(unlockDuration); }
    void lock() { lockDoor(); }

    //  Getters 
    bool getisLocked() { return IsLocked; }
    int  getUnlockDuration() { return unlockDuration; }
    void setUnlockDuration(int ms) { unlockDuration = ms; }
};

#endif