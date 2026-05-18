//  EventLog.h
//  Stores system events as a circular buffer.
//  Max 500 entries per requirement 10.1.

#ifndef EVENT_LOG_H
#define EVENT_LOG_H

#include <Arduino.h>
 
#define MAX_ENTRIES 50 //testing with 50 to avoid RAM issues
#define MAX_ENTRY_LEN 60

class EventLog {
  public:
    
    char entries[MAX_ENTRIES][MAX_ENTRY_LEN];
    int entryCount;

  private:
    int maxEntries;  // = 500 per UML, capped at MAX_ENTRIES for RAM
    int head;        // circular buffer head index

  public:
    EventLog() {
      maxEntries = 50;
      entryCount  = 0;
      head = 0;
      // Clear buffer
      for (int i = 0; i < MAX_ENTRIES; i++) {
        entries[i][0] = '\0';
      }
    }

    
    // Adds a new event string to the circular buffer (
    void addEntry(String event) {
      event.toCharArray(entries[head], MAX_ENTRY_LEN);
      head = (head + 1) % MAX_ENTRIES;
      if (entryCount < MAX_ENTRIES) entryCount++;
    }

    
    // Sends all log entries over serial to MATLAB 
    void getEvents(String filter = "") {
      Serial.println("LOGS:START");
      for (int i = 0; i < entryCount; i++) {
        String entry = String(entries[i]);
        if (filter == "" || entry.indexOf(filter) >= 0) {
          Serial.print("LOG:");
          Serial.println(entry);
        }
      }
      Serial.println("LOGS:END");
    }
    
    // Returns latest entry as String
    String getLogs() {
      if (entryCount == 0) return "LOG:No entries";
      int latest = (head - 1 + MAX_ENTRIES) % MAX_ENTRIES;
      return String(entries[latest]);
    }

    // Clears all log entries 
    void clearLogs() {
      entryCount = 0;
      head = 0;
      for (int i = 0; i < MAX_ENTRIES; i++) {
        entries[i][0] = '\0';
      }
      Serial.println("LOG:Event log cleared");
    }

    // Getters 
    int getCount() { return entryCount; }
};

#endif