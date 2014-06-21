// Cycling a servo from the neutral position to the "unlock door"
// position back to neutral

#include <Servo.h>

Servo servo;

const int BUTTON_PIN = 8;
const int SERVO_PIN = 9;
const int BUZZER_PIN = 10;

const int STEP_DELAY = 25;
const int ACTIVE_DELAY = 5000;

// Note: Servo consumes the most power at 0, least at 180
const int SERVO_MIN = 0;
const int SERVO_MAX = 100;

void setup()
{
  pinMode(BUTTON_PIN, INPUT);
  
  servo.attach(SERVO_PIN);
  servo.write(SERVO_MIN);
  delay(ACTIVE_DELAY);
  
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
}

void loop()
{
  // Debounce the pushbutton
  int buttonState = digitalRead(BUTTON_PIN);
  delay(STEP_DELAY);
  
  if(buttonState && digitalRead(BUTTON_PIN))
  {
    // Cycle the buzzer and servo
    digitalWrite(BUZZER_PIN, HIGH);
    cycleServo();
    digitalWrite(BUZZER_PIN, LOW);
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
  
