//  Keypad.h
//  Represents the physical keypad on the burglar alarm.
//  Handles PIN input buffering, debounce, and input retrieval.

#ifndef KEYPAD_H
#define KEYPAD_H
#include <Arduino.h>

class Keypad {
  private:
    String inputBuffer;   // accumulates key presses
    bool isActive;      // whether keypad is accepting input

  public:
    // Constructor
    Keypad() {
      inputBuffer = "";
      isActive = true;
    }

    void keyPressed(char key) {
      if (!isActive) return;

      if (key == 'C') {
        // Clear key
        clearInput();
        Serial.println("DISPLAY:CLEAR");
        return;
      }

      if (inputBuffer.length() < 4) {
        inputBuffer += key;
        // Send asterisk to MATLAB display for each digit
        Serial.print("KEYPAD:*");
        Serial.println(inputBuffer.length());
      }

      // Auto-submit when 4 digits entered
      if (inputBuffer.length() == 4) {
        Serial.print("PIN:");
        Serial.println(inputBuffer);
        
      }
    }
    
    // Returns current contents of input buffer
    String getInput() {
      return inputBuffer;
    }

    
    // Clears the input buffer
    void clearInput() {
      inputBuffer = "";
      Serial.println("KEYPAD:CLEARED");
    }

    
    // Enable or disable keypad input
    void setActive(bool state) {
      isActive = state;
    }

    // Getters 
    bool getisActive() { return isActive; }
    int getBufferLength(){ return inputBuffer.length(); }
};

#endif