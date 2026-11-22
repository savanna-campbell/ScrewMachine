// pins
#define LIMIT_PIN 2
#define STEP_PIN 12
#define DIR_PIN 13

volatile bool limitTriggered = false;


// stepper variables
const unsigned long interval = 2000;


// states
enum State {
  HOMING,
  IDLE,
  LOWER,
  ROTATE,
  VISION

};

int currentState = HOMING;

// interrupt functions
void limitISR() {
  limitTriggered = true;
}


void setup() {
  Serial.begin(9600);

  // SET PINS

  // limit pin
  pinMode(LIMIT_PIN, INPUT_PULLUP);

  // stepper pins
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);

  digitalWrite(DIR_PIN, HIGH);



  // ATTACH INTERRUPTS

  // limit switch interrupt
  attachInterrupt(
    digitalPinToInterrupt(LIMIT_PIN),
    limitISR,
    LOW
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
      break;
    
    case LOWER:
      break;
    
    
  }
  

}

// event check functions
bool CheckLimit() {
  Serial.println(limitTriggered);
  return limitTriggered;
}
// service functions
void StopStepper() {
  digitalWrite(STEP_PIN, LOW);
}
void RaiseStepper() {
  Serial.println("raising stepper");
  digitalWrite(DIR_PIN, HIGH);

  static unsigned long lastToggle = 0;
  static bool stepHigh = false;


  if (micros() - lastToggle >= interval) {
    lastToggle = micros();

    stepHigh = !stepHigh;
    digitalWrite(STEP_PIN, stepHigh);
  }
}


