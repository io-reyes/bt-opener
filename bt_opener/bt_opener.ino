// Irwin Reyes
// dev@irwinreyes.com
// https://github.com/io-reyes/bt-opener/
// Receives Bluetooth input from ArduDroid app and turns servo upon correct key code. Drives
// additional outputs such as a buzzer when unlocking and LED ready/timeout indicators.
// (Circuit diagram to come)

#include <Servo.h>
#include "passcode.h"  // #defines for BT_PAIRING_CODE, UNLOCK_CODE_COUNT, and UNLOCK CODE (see passcode_example.h)

#define SERVO_PIN 2
#define GREEN_LED_PIN 3
#define RED_LED_PIN 4
#define BUZZER_PIN 5

// Note: Servo consumes the most power at 0, least at 180.
// Need to use a relay to switch power into the servo.
#define SERVO_MIN 0
#define SERVO_MAX 100
#define STEP_DELAY 25
#define ACTIVE_DELAY 5000  // Delay to wait for the servo to move into its initial position, and between user input keystrokes

// Bluetooth constants
#define BAUD_RATE 9600
#define START_CMD_CHAR '*'
#define CMD_DIGITALWRITE 10

#define MAX_FAILURES 5
#define DISABLE_DELAY 300000

Servo servo;

unsigned long lastKeyReceived;  // Time the last key was received
int numKeysReceived;  // Counts the number of keys received in an input cycle
int keysReceived[UNLOCK_CODE_COUNT];  // Array containing keys received
const int unlockCode[UNLOCK_CODE_COUNT] = UNLOCK_CODE;  // Array countaining the unlock code

unsigned long lastFailureTime;  // Time the last wrong code was entered
int failureCount;  // Keeps track of how many failed attempts there have been

void setup()
{
  servo.attach(SERVO_PIN);
  servo.write(SERVO_MIN);
  delay(ACTIVE_DELAY);
  
  pinMode(GREEN_LED_PIN, OUTPUT);
  digitalWrite(GREEN_LED_PIN, HIGH); 
  
  pinMode(RED_LED_PIN, OUTPUT);
  digitalWrite(RED_LED_PIN, LOW);
  
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  
  Serial1.begin(BAUD_RATE);
  Serial1.println("CONNECTED");
  Serial1.flush();
  
  unsigned long time = millis();
  
  lastKeyReceived = time;
  resetInput();
  
  lastFailureTime = time;
  failureCount = 0;
}

void loop()
{
  Serial1.flush();
  unsigned long time = millis();
  
  // Ignore empty or invalid serial data
  if(Serial1.available() < 1 || Serial1.read() != START_CMD_CHAR)
  {
    return;
  }
  
  // If enough time has passed since the last failure, reset the failure count
  if(time < lastFailureTime || time - lastFailureTime > DISABLE_DELAY)
  {
     failureCount = 0; 
  }
    
  // Retrieve command and data
  int command = Serial1.parseInt();
  int pinNum = Serial1.parseInt();
  int pinValue = Serial1.parseInt();
  
  // Parse if received a digitalWrite command
  if(command == CMD_DIGITALWRITE)
  {
    // Reset stored inputs if idle for too long between keystrokes
    unsigned long keyTime = time;
    if(keyTime < lastKeyReceived || keyTime - lastKeyReceived > ACTIVE_DELAY)
    {
     resetInput(); 
    }
            
    keysReceived[numKeysReceived++] = pinNum;
    lastKeyReceived = keyTime;
    
    // Check against key code if the required number of keys has been received
    if(numKeysReceived >= UNLOCK_CODE_COUNT)
    {
      if(validCode())
      {
       Serial1.println("UNLOCKED");
       cycleServo(); 
      }
      else
      {
        lastFailureTime = time;
        failureCount++;
        
        // Disable device for a time if there have been too many failed attempts
        if(failureCount > MAX_FAILURES)
        {
          disable();
        }
        else
        {
          Serial1.println("INVALID CODE");
        }
      }    
     
      resetInput();
    }
    
    // Otherwise just display asterisks as feedback on the device
    else
    {
      for(int n = 0 ; n < numKeysReceived; n++)
      {
        Serial1.print('*');
      }
    }
  }
}
  
void cycleServo()
{ 
  digitalWrite(BUZZER_PIN, HIGH);
  
  for(int pos = SERVO_MIN; pos <= SERVO_MAX; pos++)
  {
    servo.write(pos);
    delay(STEP_DELAY);
  }
  
  for(int pos = SERVO_MAX; pos >= SERVO_MIN; pos--)
  {
    servo.write(pos);
    delay(STEP_DELAY);
  }
  
  digitalWrite(BUZZER_PIN, LOW);
}

void resetInput()
{
  numKeysReceived = 0;
  
  for(int n = 0; n < UNLOCK_CODE_COUNT; n++)
  {
    keysReceived[n] = -1;
  }
}

void disable()
{
  Serial1.println("LOCK OFFLINE");
  digitalWrite(GREEN_LED_PIN, LOW);
  digitalWrite(RED_LED_PIN, HIGH);
  
  delay(DISABLE_DELAY);
  
  Serial1.println("LOCK ONLINE");
  digitalWrite(GREEN_LED_PIN, HIGH);
  digitalWrite(RED_LED_PIN, LOW);  
  failureCount = 0;
}

boolean validCode()
{
  boolean valid = true;
  for(int n = 0; n < UNLOCK_CODE_COUNT; n++)
  {
    valid = valid && (keysReceived[n] == unlockCode[n]);
  }
  
  return valid;
}
