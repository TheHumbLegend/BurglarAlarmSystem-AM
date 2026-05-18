//  Database.h
//  Stores user profiles and PIN validation.
//  Faces are handled by Python - Database just tracks
//  user IDs and PINs on the Arduino side.
#ifndef DATABASE_H
#define DATABASE_H

#include <Arduino.h>
#include "User.h"


#define MAX_USERS 10

class Database {
  private:
    String  userID;
    User users[MAX_USERS];
    int userCount;
    
  public:
    // storedFaces is public per UML
    // On Arduino side we just store face owner names
    String storedFaces[MAX_USERS];
    int faceCount;
    

    Database() {
      userID   = "";
      userCount = 0;
      faceCount  = 0;
      

      // Default user with PIN 1234 for testing
      users[0]  = User("user", "USER01", "1234");
      users[1] = Technician("TECH01", "0000");
      userCount = 2;
    }

  
    // Adds a new user profile
    bool addUser(String userID, String role, String pin) {
      if (userCount >= MAX_USERS) {
        Serial.println("MSG:Max users reached");
        return false;
      }
      // Check not duplicate
      for (int i = 0; i < userCount; i++) {
        if (users[i].getUserID() == userID) {
          Serial.println("MSG:User already exists");
          return false;
        }
      }
      users[userCount] = User(role, userID, pin);
      userCount++;
      Serial.println("LOG:User added: " + userID);
      Serial.println("MSG:Profile added: " + userID);
      return true;
    }

  
    // Removes a user profile 
    bool removeUser(String userID) {
      for (int i = 0; i < userCount; i++) {
        if (users[i].getUserID() == userID) {
          // Shift array left
          for (int j = i; j < userCount - 1; j++) {
            users[j] = users[j + 1];
          }
          userCount--;
          // Remove face too
          removeFace(userID);
          Serial.println("LOG:User removed: " + userID);
          Serial.println("MSG:Profile removed: " + userID);
          return true;
        }
      }
      Serial.println("MSG:User not found: " + userID);
      return false;
    }

    
    // Checks if PIN matches any user 
    bool validatePIN(String pin) {
      for (int i = 0; i < userCount; i++) {
        if (users[i].validatePIN(pin)) {
          userID = users[i].getUserID();
          return true;
        }
      }
      return false;
    }

   
    // Separate check for technician PIN 
    bool validateTechnicianPIN(String pin) {
      for (int i = 0; i < userCount; i++) {
        if (users[i].getRole() == "technician" &&
            users[i].validatePIN(pin)) {
          return true;
        }
      }
      return false;
    }

     // *Actual face data stored by Python*   
    // Registers a face name on Arduino side 
    bool addFace(String name) {
      if (faceCount >= MAX_USERS) return false;
      storedFaces[faceCount] = name;
      faceCount++;
      Serial.println("LOG:Face registered: " + name);
      return true;
    }
   
    bool removeFace(String name) {
      for (int i = 0; i < faceCount; i++) {
        if (storedFaces[i] == name) {
          for (int j = i; j < faceCount - 1; j++) {
            storedFaces[j] = storedFaces[j + 1];
          }
          faceCount--;
          Serial.println("LOG:Face removed: " + name);
          return true;
        }
      }
      return false;
    }
    
    // Returns userID if face name matches a registered user
    String findUserByFace(String name) {
      for (int i = 0; i < faceCount; i++) {
        if (storedFaces[i] == name) return name;
      }
      return "";
    }

    // Getters 
    int getUserCount() { return userCount; }
    String getLastUserID() { return userID; }
    User&  getUser(int index) { return users[index]; }
};

#endif