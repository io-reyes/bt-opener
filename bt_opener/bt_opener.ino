// Cycling a servo from the neutral position to the "unlock door"
// position back to neutral

#include <Servo.h>
#include "passcode.h"  // #defines for BT_PAIRING_CODE, UNLOCK_CODE_COUNT, and UNLOCK CODE (see passcode_example.h)

#define SERVO_PIN 9
#define BUZZER_PIN 10

#define STEP_DELAY 25
#define ACTIVE_DELAY 5000  // Delay to wait for the servo to move into its initial position, and between user input keystrokes

// Note: Servo consumes the most power at 0, least at 180.
// Need to use a relay to switch power into the servo.
#define SERVO_MIN 0
#define SERVO_MAX 100

#define BAUD_RATE 9600

// Bluetooth constants
#define START_CMD_CHAR '*'
#define CMD_DIGITALWRITE 10

Servo servo;

unsigned long lastKeyReceived;  // Time the last key was received
int numKeysReceived;  // Counts the number of keys received in an input cycle
int keysReceived[UNLOCK_CODE_COUNT];  // Array containing keys received
const int unlockCode[UNLOCK_CODE_COUNT] = UNLOCK_CODE;  // Array countaining the unlock code

void setup()
{
  servo.attach(SERVO_PIN);
  servo.write(SERVO_MIN);
  delay(ACTIVE_DELAY);
  
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  
  Serial1.begin(BAUD_RATE);
  Serial1.println("Bluetooth connected");
  Serial1.flush();
  
  lastKeyReceived = millis();
  resetInput();
}

void loop()
{
  Serial1.flush();
  
  // Ignore empty serial data
  if(Serial1.available() < 1)
  {
    return;
  }
  
  // Ignore unstarted device
  char getChar = Serial1.read();
  if(getChar != START_CMD_CHAR)
  {
    return;
  }
    
  // Retrieve command and data
  int command = Serial1.parseInt();
  int pinNum = Serial1.parseInt();
  int pinValue = Serial1.parseInt();
  
  // Parse if received a digitalWrite command
  if(command == CMD_DIGITALWRITE)
  {
    // Reset stored inputs if idle for too long between keystrokes
    unsigned long keyTime = millis();
    if(keyTime > lastKeyReceived && keyTime - lastKeyReceived > ACTIVE_DELAY)
    {
     resetInput(); 
    }
    
    lastKeyReceived = keyTime;
        
    keysReceived[numKeysReceived++] = pinNum;
    
    // Check against key code if the required number of keys have been received
    if(numKeysReceived >= UNLOCK_CODE_COUNT)
    {
      boolean unlocked = true;
      for(int n = 0; n < UNLOCK_CODE_COUNT; n++)
      {
       unlocked = unlocked && (keysReceived[n] == unlockCode[n]);
      }
      
      if(unlocked)
      {
       Serial1.println("UNLOCKED!");
       cycleServo(); 
      }
      else
      {
        Serial1.println("INVALID CODE");
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
  
