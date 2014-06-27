// Cycling a servo from the neutral position to the "unlock door"
// position back to neutral

#include <Servo.h>

#define SERVO_PIN 9
#define BUZZER_PIN 10

#define STEP_DELAY 25
#define ACTIVE_DELAY 5000

// Note: Servo consumes the most power at 0, least at 180.
// Need to use a relay to switch power into the servo.
#define SERVO_MIN 0
#define SERVO_MAX 100

#define BAUD_RATE 9600

// Bluetooth constants
#define START_CMD_CHAR '*'
#define END_CMD_CHAR '#'
#define DIV_CMD_CHAR '|'
#define CMD_DIGITALWRITE 10
#define CMD_ANALOGWRITE 11
#define CMD_TEXT 12
#define CMD_READ_ARDUDROID 13
#define MAX_COMMAND 20  // max command number code. used for error checking.
#define MIN_COMMAND 10  // minimum command number code. used for error checking. 
#define IN_STRING_LENGHT 40
#define MAX_ANALOGWRITE 255
#define PIN_HIGH 3
#define PIN_LOW 2

Servo servo;
String text;

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
  
  // Parse if received a text command
  if(command == CMD_TEXT)
  {
    text = "";

    while(Serial1.available())
    {
      char c = Serial1.read();
      delay(5);
      
      if(c == END_CMD_CHAR)
      {
        digitalWrite(BUZZER_PIN, HIGH);
        cycleServo();
        digitalWrite(BUZZER_PIN, LOW);
        break;
      }
      else if(c != DIV_CMD_CHAR)
      {
        text += c;
        delay(5);
      }
    }
  }
}
  
void cycleServo()
{ 
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
}
  
