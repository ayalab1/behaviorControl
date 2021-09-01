/*
  Analog Input

  Demonstrates analog input by reading an analog sensor on analog pin 0 and
  turning on and off a light emitting diode(LED) connected to digital pin 13.
  The amount of time the LED will be on and off depends on the value obtained
  by analogRead().

  The circuit:
  - potentiometer
    center pin of the potentiometer to the analog input 0
    one side pin (either one) to ground
    the other side pin to +5
  - LED
    anode (long leg) attached to digital output 13
    cathode (short leg) attached to ground

  - Note: because most Arduinos have a built-in LED attached to pin 13 on the
    board, the LED is optional.

  created by David Cuartielles
  modified 30 Aug 2011
  By Tom Igoe

  This example code is in the public domain.

  http://www.arduino.cc/en/Tutorial/AnalogInput
*/

//  #Farnaz: The difference between int and const int is that int is read/write while const int is read-only.
// #Farnaz: If you want the compiler to catch invalid attempts to write to a variable, make it const.

int rightIRsensor = A0;    // Signal for the right Sensor
int leftIRsensor = A1;    // Signal for the left Sensor
const int rightTTL = 52;      // select the pin for the intan   
const int leftTTL = 53;      // select the pin for the intan
boolean EnableWaterRight = true;
boolean EnableWaterLeft = false;
int CurrentRightInfraState = 0;
int CurrentLeftInfraState = 0;
int PreviousRightInfraState = 0;
int PreviousLeftInfraState = 0;
int trialNum = 0;

//Declare pin functions on Arduino for user input to change water amount
const int switchPin = A7;     // the number of the switch button pin
int switchState = 0;         // variable for reading the switch button status
int waterCycleRight;
int waterCycleLeft;

//Declare pin functions on Arduino for BigEasy driver (syringe pump motor)

#define dirLeft 2// # Farnaz
#define stpLeft 3// # Farnaz
#define MS3Left 4// # Farnaz
#define MS2Left 5// # Farnaz
#define MS1Left 6// # Farnaz
#define ENLeft  7// # Farnaz

#define dirRight 8
#define stpRight 9
#define MS3Right 10
#define MS2Right 11
#define MS1Right 12
#define ENRight  13

char user_input;
int x;
int y;
int state;

#include <LiquidCrystal.h> // includes the LiquidCrystal Library
LiquidCrystal lcd(33, 31, 29, 27, 25, 23); // Creates an LC object. Parameters: (rs, enable, d4, d5, d6, d7)

//Declare pin functions on Arduino for BigEasy driver (syringe pump motor)
// # Farnaz : everything exept than Ground, should be pin in setup?
void setup() {
  pinMode(rightIRsensor, INPUT);
  pinMode(leftIRsensor, INPUT);
  pinMode(rightTTL, OUTPUT);
  pinMode(leftTTL, OUTPUT);
  pinMode(switchPin, INPUT);
  pinMode(stpRight, OUTPUT);
  pinMode(dirRight, OUTPUT);
  pinMode(MS1Right, OUTPUT);
  pinMode(MS2Right, OUTPUT);
  pinMode(MS3Right, OUTPUT);
  pinMode(ENRight, OUTPUT);
  pinMode(stpLeft, OUTPUT);// # Farnaz
  pinMode(dirLeft, OUTPUT);// # Farnaz
  pinMode(MS1Left, OUTPUT);// # Farnaz
  pinMode(MS2Left, OUTPUT);// # Farnaz
  pinMode(MS3Left, OUTPUT);// # Farnaz
  pinMode(ENLeft, OUTPUT);// # Farnaz

  resetBEDPinsLeft(); //Set step, direction, microstep and enable pins to default states . #Farnaz what is this command?
  resetBEDPinsRight();
  // initialize serial communication:
  Serial.begin(9600);
  lcd.begin(16, 2); // Initializes the interface to the LCD screen, and specifies the dimensions (width and height) of the display }
}

void loop() {
  // read the value from the sensor:
  CurrentRightInfraState = analogRead(rightIRsensor);
  CurrentLeftInfraState = analogRead(leftIRsensor);
  int dialValue = analogRead(switchPin);
  float voltage = dialValue * (5.0/1023.0);
  float waterLevel = (-2 * voltage) + 10;

  waterCycleRight = 33.8 * waterLevel;//need to calibrate this number 
  waterCycleLeft = 33.8 * waterLevel;//need to calibrate this number
  Serial.println(waterCycleRight);

  lcd.setCursor(0, 1);
  lcd.print(waterLevel); //high amount of water for Misi
  
  if ((CurrentRightInfraState < 100) && (PreviousRightInfraState > 600))  // detect the animal is breaking the IR beam -> goes from Hi->Lo
  {
    if (EnableWaterRight == true)
    {
      trialNum++;
      //Serial.print("Trial: ");
      //Serial.println(trialNum);
      EnableWaterRight = !EnableWaterRight;
      EnableWaterLeft = !EnableWaterLeft;
      // turn the ledPin on
      digitalWrite(rightTTL, HIGH);
      delay(10);
      digitalWrite(rightTTL, LOW);
      // set water amount


      //start motor
      digitalWrite(ENRight, LOW);        //Pull enable pin low to set FETs active and allow motor control
      StepForwardDefaultRight(); //CONSIDER change it to slower water delivery!!! ASK BRIAN
      resetBEDPinsRight();
      lcd.setCursor(0, 0);
      lcd.print("Trial:"); // Prints "Arduino" on the LCD
      lcd.setCursor(7, 0); // Sets the location at which subsequent text written to the LCD will be displayed
      lcd.print(trialNum);
    }
  }

  if ((CurrentLeftInfraState < 100) && (PreviousLeftInfraState > 600)) // detect the animal is breaking the IR beam -> goes from Hi->Lo
  {
    if (EnableWaterLeft == true)
    {
      //Serial.println("Animal is detected on the left");
      EnableWaterRight = !EnableWaterRight;
      EnableWaterLeft = !EnableWaterLeft;
      // turn the ledPin on
      digitalWrite(leftTTL, HIGH);
      delay(10);
      digitalWrite(leftTTL, LOW);

      
      //start motor
      digitalWrite(ENLeft, LOW);        //Pull enable pin low to set FETs active and allow motor control
      StepForwardDefaultLeft(); //CONSIDER change it to slower water delivery!!! ASK BRIAN
      resetBEDPinsLeft();
    }
  }

  PreviousRightInfraState = CurrentRightInfraState; //save the previous IR state
  PreviousLeftInfraState = CurrentLeftInfraState; //save the previous IR state
}


//Reset Big Easy Driver pins to default states
void resetBEDPinsRight()
{
  digitalWrite(stpRight, LOW);
  digitalWrite(dirRight, LOW);
  digitalWrite(MS1Right, LOW);
  digitalWrite(MS2Right, LOW);
  digitalWrite(MS3Right, LOW);
  digitalWrite(ENRight, HIGH);
}

//Default microstep mode function
void StepForwardDefaultRight()
{
  digitalWrite(dirRight, HIGH); //Pull direction pin low to move "forward"
  for (x = 1; x < waterCycleRight; x++) //Loop the forward stepping enough times for motion to be visible
  {
    digitalWrite(stpRight, HIGH); //Trigger one step forward
    delay(1);
    digitalWrite(stpRight, LOW); //Pull step pin low so it can be triggered again
    delay(1);
  }
}



// # Farnaz
//Reset Big Easy Driver pins to default states
void resetBEDPinsLeft()
{
  digitalWrite(stpLeft, LOW);
  digitalWrite(dirLeft, LOW);
  digitalWrite(MS1Left, LOW);
  digitalWrite(MS2Left, LOW);
  digitalWrite(MS3Left, LOW);
  digitalWrite(ENLeft, HIGH);
}

//Default microstep mode function
void StepForwardDefaultLeft()
{
  digitalWrite(dirLeft, HIGH); //Pull direction pin low to move "forward"
  for (x = 1; x < waterCycleLeft; x++) //Loop the forward stepping enough times for motion to be visible
  {
    digitalWrite(stpLeft, HIGH); //Trigger one step forward
    delay(1);
    digitalWrite(stpLeft, LOW); //Pull step pin low so it can be triggered again
    delay(1);
  }
}
