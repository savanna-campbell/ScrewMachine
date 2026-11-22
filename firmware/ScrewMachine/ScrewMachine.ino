// pins
#define LIMIT_PIN 2
#define STEP_PIN 12
#define DIR_PIN 13
#define BUTTON_PIN 10

volatile bool limitTriggered = false;
volatile bool buttonTriggered = false;

// stepper variables
const unsigned long interval = 2000;
unsigned long lastToggle = 0;
bool stepHigh = false;

// states
enum State {
  HOMING,
  IDLE,
  LOWER,
  ROTATE,
  VISION

};

State currentState = HOMING;

// interrupt functions
void limitISR() {
  limitTriggered = true;
}

void buttonISR() {
  buttonTriggered = true;
}


void setup() {
  Serial.begin(9600);

  // SET PINS
  // sensor pins

  // limit pin
  pinMode(LIMIT_PIN, INPUT_PULLUP);
  // button pin
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // motor pins
  // stepper pins
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);

  digitalWrite(DIR_PIN, HIGH);
  digitalWrite(STEP_PIN, LOW);



  // ATTACH INTERRUPTS

  // limit switch interrupt
  attachInterrupt(
    digitalPinToInterrupt(LIMIT_PIN),
    limitISR,
    FALLING
  );
  // button interrupt
  attachInterrupt(
    digitalPinToInterrupt(BUTTON_PIN),
    buttonISR,
    FALLING
  );

}

void loop() {
  switch (currentState){
    case HOMING:
      if(CheckLimit()) {
        currentState = IDLE;
        StopStepper();
      }
      else {
        RaiseStepper();
      }
      break;
    
    case IDLE:
      if(CheckButton()) {
        currentState = LOWER;
        Lower();
      }
      break;
    
    case LOWER:
      break;
    
    
  }
  

}

// event check functions
bool CheckLimit() {
  return limitTriggered || digitalRead(LIMIT_PIN) == LOW;
}
bool CheckButton() {
  return buttonTriggered;
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

void Lower() {
  Serial.println("Lowering!");
}

