//  DisplayPanel.h
//  Represents the physical display panel on the burglar alarm.
//  Handles turning on/off, showing messages, menus, and
//  auto-timeout after inactivity - 2 mins.

#ifndef DISPLAY_PANEL_H
#define DISPLAY_PANEL_H

#include <Arduino.h>


class DisplayPanel {
  private:
    bool isOn;
    String currentMessage;
    int Timeout;       // timeout in milliseconds
    unsigned long lastActiveTime;  // tracks inactivity for auto-off

  public:
    // Constructor 
    DisplayPanel(int timeoutMs = 120000) {  // default 2 min 
      isOn = false;
      currentMessage = "";
      Timeout = timeoutMs;
      lastActiveTime = 0;
    }

   
    // Turns the display on and resets the inactivity timer.
    // Called whenever a keypad button is pressed 
    void turnOn() {
      isOn = true;
      lastActiveTime = millis();
      Serial.println("DISPLAY:ON");
    }

    // Turns the display off after specified timeout 
    void turnOff(int Timeout) {
      this->Timeout = Timeout;
      isOn = false;
      currentMessage = "";
      Serial.println("DISPLAY:OFF");
    }
    
    // Shows a message on the display and sends it over serial
    // to MATLAB so the UI updates 
    void displayMessage(String currentMessage) {
      if (!isOn) turnOn();
      this->currentMessage = currentMessage;
      lastActiveTime = millis();

      // Send to MATLAB via serial
      Serial.print("MSG:");
      Serial.println(currentMessage);
    }
    
    // Shows a numbered menu from a list of options.
    // Sends each option over serial for MATLAB to display.
    void displayMenu(String options[], int numOptions) {
      if (!isOn) turnOn();
      lastActiveTime = millis();

      // Send menu header
      Serial.println("MENU:START");
      for (int i = 0; i < numOptions; i++) {
        Serial.print("MENU:");
        Serial.print(i + 1);
        Serial.print(":");
        Serial.println(options[i]);
      }
      Serial.println("MENU:END");
    }

    
    // Clears the current message from the display
    void clear() {
      currentMessage = "";
      Serial.println("MSG: ");
    }

  
    void checkTimeout() {
      if (isOn && (millis() - lastActiveTime >= (unsigned long)Timeout)) {
        turnOff(Timeout);
      }
    }
    
    // Resets inactivity timer  call whenever user interacts
    void resetTimer() {
      lastActiveTime = millis();
    }

    // Getters 
    bool getIsOn() { return isOn; }
    String  getMessage() { return currentMessage; }
    int getTimeout() { return Timeout; }
};

#endif