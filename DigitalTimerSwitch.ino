#include <LiquidCrystal.h>

LiquidCrystal lcd(12, 11, 10, 9, 13, A0);

#define BTN_HOUR_UP 2 // PD2/2
#define BTN_HOUR_DOWN 3 // PD3/3
#define BTN_MIN_UP 4 // PD4/4
#define BTN_MIN_DOWN 5 // PD5/5
#define BTN_CONFIRM 6 // PD6/6

// Team 3: LED and Buzzer Test Code
// This code is used to verify that the LED and buzzer are wired correctly in PICSimLab
// Team 2 (Timer Logic) will later control these pins according to the schedule
const int ledPin = 7; // (D7 / PD7/7)
const int buzzerPin = 8; // Buzzer (D8 / PB0/8)

// Initialize Timer
int onHour = 0, onMinute = 0;
int offHour = 0, offMinute = 0;
bool timerActive = false;

int currentHour = 0, currentMinute = 0, currentSecond = 0;
unsigned long lastSecond = 0;
unsigned long lastDisplayUpdate = 0;

enum State { SET_ON_TIME, SET_OFF_TIME, RUNNING };
State currentState = SET_ON_TIME;
int tempHour = 0, tempMinute = 0;

unsigned long lastDebounce[5] = {0, 0, 0, 0, 0};
const unsigned long debounceDelay = 300; // Minimum time that must pass before accepting another press
bool lastButtonState[5] = {HIGH, HIGH, HIGH, HIGH, HIGH};

void setup() {
  lcd.begin(16, 2);
  lcd.clear();
  // setting up pins of the buttons
  pinMode(BTN_HOUR_UP, INPUT_PULLUP);
  pinMode(BTN_HOUR_DOWN, INPUT_PULLUP);
  pinMode(BTN_MIN_UP, INPUT_PULLUP);
  pinMode(BTN_MIN_DOWN, INPUT_PULLUP);
  pinMode(BTN_CONFIRM, INPUT_PULLUP);

  // setting up pins of the buzzer and led pin
  pinMode(ledPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  
  digitalWrite(ledPin, LOW);
  digitalWrite(buzzerPin, LOW);
  
  // starting page
  lcd.setCursor(0, 0);
  lcd.print("Timer Setup");
  delay(3000);
  lcd.clear();
  
  // Set On time page
  lcd.setCursor(0, 0);
  lcd.print("Set ON Time:");
  lcd.setCursor(0, 1);
  lcd.print("00:00 [Confirm]");
}

// Team 2: Timer Logic Functions
// Add these functions to your DigitalTimerSwitch.ino file

// Function to update the current time (increments every second)
void updateTime() {
  // Only update time when timer is running
  if (!timerActive) {
    return;
  }
  
  unsigned long currentMillis = millis();
  
  // Check if one second has passed
  if (currentMillis - lastSecond >= 1000) {
    lastSecond = currentMillis;
    
    // Increment seconds
    currentSecond++;
    
    // If seconds reach 60, reset and increment minutes
    if (currentSecond >= 60) {
      currentSecond = 0;
      currentMinute++;
      
      // If minutes reach 60, reset and increment hours
      if (currentMinute >= 60) {
        currentMinute = 0;
        currentHour++;
        
        // If hours reach 24, reset to 0 (24-hour cycle)
        if (currentHour >= 24) {
          currentHour = 0;
        }
      }
    }
  }
}

// Function to control devices (LED and Buzzer) based on schedule
void controlDevices() {
  // Only control devices when timer is running
  if (!timerActive) {
    digitalWrite(ledPin, LOW);
    digitalWrite(buzzerPin, LOW);
    return;
  }
  
  // Convert current time to minutes since midnight for easier comparison
  int currentTimeInMinutes = (currentHour * 60) + currentMinute;
  int onTimeInMinutes = (onHour * 60) + onMinute;
  int offTimeInMinutes = (offHour * 60) + offMinute;
  
  bool shouldBeOn = false;
  
  if (onTimeInMinutes == offTimeInMinutes) {
      shouldBeOn = false;
    } else if (onTimeInMinutes < offTimeInMinutes) {
      shouldBeOn = (currentTimeInMinutes >= onTimeInMinutes && currentTimeInMinutes < offTimeInMinutes);
    } else {
      shouldBeOn = (currentTimeInMinutes >= onTimeInMinutes || currentTimeInMinutes < offTimeInMinutes);
    }
    
    digitalWrite(ledPin, shouldBeOn ? HIGH : LOW);
    digitalWrite(buzzerPin, shouldBeOn ? HIGH : LOW);
 }

void loop() {
  handleButtons();      // Team 1: Handle button inputs
  updateTime();         // Team 2: Update current time
  controlDevices();     // Team 2: Control LED and buzzer based on schedule
  updateDisplay();      // Team 3: Update LCD display
}



void handleButtons() {
  unsigned long now = millis(); // current time
  
  int buttons[5] = {BTN_HOUR_UP, BTN_HOUR_DOWN, BTN_MIN_UP, BTN_MIN_DOWN, BTN_CONFIRM}; // Array with all buttons
  
  // Loop through buttons
  for (int i = 0; i < 5; i++) {
    bool currentButtonState = digitalRead(buttons[i]); // Current state [High or Low]
    
    if (currentButtonState == LOW && lastButtonState[i] == HIGH && (now - lastDebounce[i]) > debounceDelay) {
      lastDebounce[i] = now;
      
      // check that buttons are not working
      if (currentState != RUNNING || i == 4) {
        // Determine which button was pressed
        switch(i) {
          case 0: // Hour UP
            if (currentState != RUNNING) {
              tempHour = (tempHour + 1) % 24; // Increase hour by 1 and wrap around at 24
            }
            break;
            
          case 1: // Hour DOWN
            if (currentState != RUNNING) {
              tempHour = (tempHour == 0) ? 23 : tempHour - 1; // Decrease hour by 1 
            }
            break;
            
          case 2: // Minute UP
            if (currentState != RUNNING) {
              tempMinute = (tempMinute + 1) % 60; // Increase minute by 1 and wrap around at 60
            }
            break;
            
          case 3: // Minute DOWN
            if (currentState != RUNNING) {
              tempMinute = (tempMinute == 0) ? 59 : tempMinute - 1; // Decrease minute by 1 and check if it is 0
            }
            break;
            
          case 4: // Confirm
            confirmAction(); // Save settings and change state
            break;
        }
      }
    }
    
    lastButtonState[i] = currentButtonState; // changing to the current state
  }
}

void confirmAction() {
  switch (currentState) {
    case SET_ON_TIME:
      onHour = tempHour;
      onMinute = tempMinute;
      currentState = SET_OFF_TIME; // Convert into OFF state
      tempHour = 0;
      tempMinute = 0;
      break;
      
    case SET_OFF_TIME:
      offHour = tempHour;
      offMinute = tempMinute;
      currentState = RUNNING; // convert into running state
      timerActive = true;
      currentHour = 0;
      currentMinute = 0;
      currentSecond = 0;
      lastSecond = millis(); // Calculate current time
      break;
      
    case RUNNING:
      currentState = SET_ON_TIME; // Convert into ON state
      timerActive = false;
      tempHour = 0;
      tempMinute = 0;
      onHour = 0;
      onMinute = 0;
      offHour = 0;
      offMinute = 0;
      digitalWrite(ledPin, LOW); // turn led off
      digitalWrite(buzzerPin, LOW); // turn buzzer off
      break;
  }
}

void updateDisplay() {
  // Used to detect changes
  static State lastState = SET_ON_TIME;     
  static int lastTempHour = -1;             
  static int lastTempMinute = -1;           
  static int lastCurrentSecond = -1;        
  
  // Flag to determine if LCD needs updates
  bool needsUpdate = false;
  
  // Check if state changed
  if (lastState != currentState) {
    needsUpdate = true;
    lastState = currentState;
  }
  
  // Check if time values changed during setup modes
  if (currentState != RUNNING) {
    // If user changed hour or minute using buttons
    if (lastTempHour != tempHour || lastTempMinute != tempMinute) {
      needsUpdate = true;           
      lastTempHour = tempHour;
      lastTempMinute = tempMinute;
    }
  } 
  // Check if seconds changed during RUNNING mode
  else {
    // Timer is running, check if a second passed
    if (lastCurrentSecond != currentSecond) {
      needsUpdate = true;            
      lastCurrentSecond = currentSecond; // Remember new second value
    }
  }
  
  // If nothing changed -> exit
  if (!needsUpdate) return;
  
  // Clear the LCD
  lcd.clear();
  
  // Display different screens based on current state
  switch (currentState) {
    case SET_ON_TIME:
      // Setting the ON time
      lcd.setCursor(0, 0);           // First line
      lcd.print("Set ON Time:");     // Title
      lcd.setCursor(0, 1);           // Second line
      displayTime(tempHour, tempMinute); // (HH:MM)
      lcd.print(" [Confirm]");       // Remind user to press Confirm button
      break;
      
    case SET_OFF_TIME:
      // Setting the OFF time
      lcd.setCursor(0, 0);           // First line
      lcd.print("Set OFF Time:");    // Title
      lcd.setCursor(0, 1);           // Second line
      displayTime(tempHour, tempMinute); // (HH:MM)
      lcd.print(" [Confirm]");       // Remind user to press Confirm button
      break;
      
    case RUNNING:
      // Running screen: Timer is active, show current time and schedule
      // First line
      lcd.setCursor(0, 0);           // First line
      lcd.print("Time: ");           // Label
      displayTime(currentHour, currentMinute); // Show current hours and minutes
      lcd.print(":");                // Separator
      if (currentSecond < 10) lcd.print("0"); // Add leading zero for seconds 0-9
      lcd.print(currentSecond);      // Show seconds
      
      // Second line
      lcd.setCursor(0, 1);           // Second line
      lcd.print("ON:");              // Label for ON time
      displayTime(onHour, onMinute); // Show when devices turn ON
      lcd.print("OFF:");             // Label for OFF time
      displayTime(offHour, offMinute); // Show when devices turn OFF
      break;
  }
}

// Displaying time on LCD
void displayTime(int h, int m) {
  if (h < 10) lcd.print("0"); // (0h:mm)
  lcd.print(h);
  lcd.print(":"); // separator
  if (m < 10) lcd.print("0"); // (hh:0m)
  lcd.print(m);
}
