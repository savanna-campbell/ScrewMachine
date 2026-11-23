#include "HX711.h"
// pins
#define LIMIT_PIN 2
#define STEP_PIN 12
#define DIR_PIN 13
#define BUTTON_PIN 3
#define DT_PIN 10
#define SCK_PIN 9
#define A_PIN 18
#define B_PIN 17
#define BRUSHED_DIR_PIN 7
#define BRUSHED_PWM_PIN 6
#define GREEN_PIN 19
#define RED_PIN 20

// sensor variables
volatile bool limitTriggered = false;
volatile bool buttonTriggered = false;
volatile long encoderCount = 0;

// loadcell variables
HX711 loadCell;
const long FORCE_THRESHOLD = 10000;
long reading = 0;

// stepper variables
const unsigned long interval = 250;
unsigned long lastToggle = 0;
bool stepHigh = false;

// brushed variables
const long TARGET_COUNT = 350;
const int MOTOR_SPEED = 128;
int numRotations = 0;

// LED variables
int color_pin;

// states
enum State {
  HOMING,
  IDLE,
  LOWER,
  VISION,
  ROTATE,
  COLOR

};

State currentState = HOMING;

// interrupt functions
void limitISR() {
  limitTriggered = true;
  // Serial.println(limitTriggered);

}

void buttonISR() {
  buttonTriggered = true;
}

void encoderISR() {
  if(digitalRead(A_PIN) == digitalRead(B_PIN)) {
    encoderCount++;
  }
  else {
    encoderCount--;
  }
}

void setup() {
  Serial.begin(9600);

  // SET PINS
  // sensor pins
  // limit pin
  pinMode(LIMIT_PIN, INPUT_PULLUP);
  // button pin
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  // loadcell pin
  loadCell.begin(DT_PIN, SCK_PIN);

  // wait until the load cell is ready and calibrate
  if (loadCell.wait_ready_timeout(2000)) {
    loadCell.tare(10);
  } else {
    Serial.println("Load cell not ready!");
    // Keep the machine disabled until the sensor works.
  }

  // LEDs
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);

  digitalWrite(RED_PIN, LOW);
  digitalWrite(GREEN_PIN, LOW);

  // motor pins
  // stepper pins
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);

  digitalWrite(DIR_PIN, HIGH);
  digitalWrite(STEP_PIN, LOW);

  // brushed pins
  pinMode(A_PIN, INPUT_PULLUP);
  pinMode(B_PIN, INPUT_PULLUP);
  pinMode(BRUSHED_DIR_PIN, OUTPUT);
  pinMode(BRUSHED_PWM_PIN, OUTPUT);

  digitalWrite(BRUSHED_DIR_PIN, HIGH);
  analogWrite(BRUSHED_PWM_PIN, 0);


  // ATTACH INTERRUPTS

  // limit switch interrupt
  attachInterrupt(
    digitalPinToInterrupt(LIMIT_PIN),
    limitISR,
    RISING
  );
  // button interrupt
  attachInterrupt(
    digitalPinToInterrupt(BUTTON_PIN),
    buttonISR,
    FALLING
  );
  // encoder interrupt
  attachInterrupt(
    digitalPinToInterrupt(A_PIN),
    encoderISR,
    CHANGE
  );

}

void loop() {
  switch (currentState){
    case HOMING:
      if(CheckLimit()) {
        currentState = IDLE;
        StopStepper();
        ResetColors();
        
      }
      else {
        RaiseStepper();
      }
      break;
    
    case IDLE:
      if(CheckButton()) {
        currentState = LOWER;
        buttonTriggered = false;
      }
      break;
    
    case LOWER:
      
      if(CheckLoad()){
        currentState = VISION;
        Serial.println("VISION");
        StopStepper();
      }
      else {
        LowerStepper();
      }
      break;

    case VISION:
      
      if(numRotations >= 4) {
        currentState = COLOR;
        numRotations = 0;
        Serial.println("COLOR");
        
      }
      else if(CheckVision()) {
        currentState = ROTATE;
        
      }
      break;

    case ROTATE:
      if(CheckEncoderOver90()) {
        currentState = VISION;
        StopBrushed();
        ResetEncoder();
        numRotations++;
        Serial.println("VISION");

        
      }
      else {
        RotateBrushed();
      }
      break;
    
    case COLOR:
      if(CheckColor()) {
        SetColor();
        currentState = HOMING;
        ResetButton();
      }
      break;
    
    
  }
}

// event check functions
bool CheckLimit() {
  static unsigned long highSince = 0;
  if (digitalRead(LIMIT_PIN) == HIGH) {
    if (highSince == 0) {
      highSince = millis();
    }

    return millis() - highSince >= 10;
  }

  highSince = 0;
  return false;
}
bool CheckButton() {
  return buttonTriggered;
}
bool CheckLoad() {
  if(loadCell.is_ready()){
    reading = loadCell.get_value(1);
  }
  return reading > FORCE_THRESHOLD;
}
bool CheckVision() {
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();

    if (command == "ROTATE") {
      return true;
    }
  }
  return false;
}
bool CheckEncoderOver90() {
  noInterrupts();
  long count = encoderCount;
  interrupts();
  return abs(count) >= TARGET_COUNT;
}
bool CheckColor() {
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();

    if (command == "RED" || command == "GREEN") {
      if (command == "RED") {
        color_pin = RED_PIN;
      }
      else {
        color_pin = GREEN_PIN;
      }
      return true;
    }
  }
  return false;
}

// service functions
void StopStepper() {
  digitalWrite(STEP_PIN, LOW);
  stepHigh = false;
}

void RaiseStepper() {
  digitalWrite(DIR_PIN, HIGH);

  if (micros() - lastToggle >= interval) {
    lastToggle = micros();

    stepHigh = !stepHigh;
    digitalWrite(STEP_PIN, stepHigh);
  }
}

void LowerStepper() {
  digitalWrite(DIR_PIN, LOW);

  if (micros() - lastToggle >= interval) {
    lastToggle = micros();

    stepHigh = !stepHigh;
    digitalWrite(STEP_PIN, stepHigh);
  }
}

void StopBrushed() {
  analogWrite(BRUSHED_PWM_PIN, 0);
}

void RotateBrushed() {
  digitalWrite(BRUSHED_DIR_PIN, HIGH);
  analogWrite(BRUSHED_PWM_PIN, MOTOR_SPEED);
}

void ResetEncoder() {
  noInterrupts();
  encoderCount = 0;
  interrupts();
}

void SetColor() {
  digitalWrite(color_pin, HIGH);
}

void StopAll() {
  StopStepper();
  StopBrushed();
}

void ResetButton() {
  buttonTriggered = false;
}

void ResetColors(){
  digitalWrite(RED_PIN, LOW);
  digitalWrite(GREEN_PIN, LOW);
}
