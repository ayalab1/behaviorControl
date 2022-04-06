
//Declare pin functions on Arduino for BigEasy driver (syringe pump motor)

#include <Servo.h> //library for servo motor.
//Declare pin functions on Arduino for BigEasy driver (syringe pump motor)
Servo servo_left;
Servo servo_right;
#define dirLeft 2
#define stpLeft 3
#define MS3Left 4
#define MS2Left 5
#define MS1Left 6
#define ENLeft  7

#define dirRight 8
#define stpRight 9
#define MS3Right 10
#define MS2Right 11
#define MS1Right 12
#define ENRight  13

#define dirStart 22
#define stpStart 23
#define MS3Start 24
#define MS2Start 25 
#define MS1Start 26
#define ENStart  27

#define StartTTL 52
#define LeftTTL  50
#define RightTTL  48
#define DisplayTTL 46

#define Servoleft 44//29
#define Servoright 45//28

int servostop=90; // 90
int servoopen= 180; // 180
int servoclose= 0; // 0
int rightIRsensor = A0;    // Signal for the right Sensor
int leftIRsensor = A1;    // Signal for the left Sensor
int Startsensor = A2;      // Signal for the start Sensor
int incomingByte = 0;   //Define variable to receive signal from Matlab
boolean RightDoorIsCurrentlyClosing = false;
boolean LeftDoorIsCurrentlyClosing = false;
boolean RightDoorIsCurrentlyOpening = false;
boolean LeftDoorIsCurrentlyOpening = false;
boolean StartShouldOpenLeftDoor = false;
boolean StartShouldOpenRightDoor = false;
boolean EnableWaterRight = false;
boolean EnableWaterLeft = false;
boolean EnableStart = false;
boolean RightWaterIsCurrentlyOpen = false;
boolean LeftWaterIsCurrentlyOpen = false;
boolean StartWaterIsCurrentlyOpen = false;
boolean mousemoving = true;
boolean ImposeDelayBeforeOpeningLeftDoor = false;
boolean ImposeDelayBeforeOpeningRightDoor = false;
unsigned long TimeOfLeftDoorClosing;
unsigned long TimeOfRightDoorClosing;
unsigned long TimeOfLeftDoorOpening;
unsigned long TimeOfRightDoorOpening;
unsigned long TimeOfRightWaterOpening;
unsigned long TimeOfLeftWaterOpening;
unsigned long TimeOfStartWaterOpening;
unsigned long PUMP_OPEN_TIME=0.5;
unsigned long PUMP_OPEN_TIME_START=5;
unsigned long TIME_IT_TAKES_FOR_DOOR_TO_OPEN=250;
unsigned long TIME_IT_TAKES_FOR_DOOR_TO_CLOSE=275;
unsigned long DOOR_WAIT_TIME=10000;
int StartSensorReading = 0;
int RightSensorReading = 0;
int LeftSensorReading = 0;
int waterCycleRight;
int waterCycleLeft;
int x;
int readout;
const int switchPin = A7;     // the number of the switch button pin
struct Data
{
  
  int8_t readstart = 0;
  int8_t readleft = 0;
  int8_t readright = 0;
  int8_t correct_number_start = 0;
  int8_t correct_number_left = 0;
  int8_t correct_number_right = 0;
  int8_t start = 250;
  
};
Data data;


void resetBEDPinsRight()
{
  digitalWrite(stpRight, LOW);
  digitalWrite(dirRight, LOW);
  digitalWrite(MS1Right, LOW);
  digitalWrite(MS2Right, HIGH);
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
  digitalWrite(MS2Left, HIGH);
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

void resetBEDPinsStart()
{
  digitalWrite(stpStart, LOW);
  digitalWrite(dirStart, LOW);
  digitalWrite(MS1Start, LOW);
  digitalWrite(MS2Start, HIGH);
  digitalWrite(MS3Start, LOW);
  digitalWrite(ENStart, HIGH);
}

//Default microstep mode function
void StepForwardDefaultStart()
{
  digitalWrite(dirStart, HIGH); //Pull direction pin low to move "forward"
  for (x = 1; x < waterCycleLeft; x++) //Loop the forward stepping enough times for motion to be visible
  {
    digitalWrite(stpStart, HIGH); //Trigger one step forward
    delay(1);
    digitalWrite(stpStart, LOW); //Pull step pin low so it can be triggered again
    delay(1);
  }
}

void setup() {
 servo_left.attach(Servoleft); 
 servo_right.attach(Servoright);
 pinMode(rightIRsensor, INPUT);
 pinMode(leftIRsensor, INPUT);
 pinMode(Startsensor, INPUT);
  //pinMode(rightTTL, OUTPUT);
  //pinMode(leftTTL, OUTPUT);
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

  pinMode(stpStart, OUTPUT);// # Farnaz
  pinMode(dirStart, OUTPUT);// # Farnaz
  pinMode(MS1Start, OUTPUT);// # Farnaz
  pinMode(MS2Start, OUTPUT);// # Farnaz
  pinMode(MS3Start, OUTPUT);// # Farnaz
  pinMode(ENStart, OUTPUT);// # Farnaz
  
  pinMode(RightTTL,OUTPUT);
  pinMode(LeftTTL,OUTPUT);
  pinMode(StartTTL,OUTPUT);
  pinMode(DisplayTTL,OUTPUT);
  resetBEDPinsLeft(); //Set step, direction, microstep and enable pins to default states . #Farnaz what is this command?
  resetBEDPinsRight();
  resetBEDPinsStart();
  // initialize serial communication:
  Serial.begin(9600);
  //digitalWrite(LED, LOW);
  data.readstart=0;
  data.readleft=0;
  data.readright=0;

  
}

void loop() {

  digitalWrite(DisplayTTL,LOW);
  RightSensorReading = analogRead(rightIRsensor);
  //RightSensorReading = 190-RightSensorReading;
  LeftSensorReading= analogRead(leftIRsensor);
  //LeftSensorReading = 190-LeftSensorReading;
  StartSensorReading  = analogRead(Startsensor);
  //data.readleft = analogRead(rightIRsensor);
  //data.readright = analogRead(leftIRsensor);
  //data.readstart= analogRead(Startsensor);
  //Serial.println(LeftSensorReading) ;
  //Serial.println(RightSensorReading) ;
  //delay(1000)
  int dialValue = analogRead(switchPin);//?Hongyu: need some explain
  float voltage = dialValue * (5.0/1023.0);
  float waterLevel = (-2 * voltage) + 10;
  waterCycleRight = 1.5 * waterLevel;//need to calibrate this number; originally 33.8
  waterCycleLeft = 1.5 * waterLevel;//need to calibrate this number

  ///////////////////////////////////////////////////////////////////////
  if (RightSensorReading>100)
  {digitalWrite(RightTTL,HIGH);
  data.readright=1;
  }
  else
  {digitalWrite(RightTTL,LOW);
  data.readright=0;}
  
  if (LeftSensorReading>100)
  {digitalWrite(LeftTTL,HIGH);
  data.readleft=1;
  }
  else
  {digitalWrite(LeftTTL,LOW);
  data.readleft=0;}

  if (StartSensorReading<10)
  {digitalWrite(StartTTL,HIGH);
  data.readstart=1;
  }
  else
  {digitalWrite(StartTTL,LOW);
  data.readstart=0;
  mousemoving=true;
  }


 //*************************************************************************
  if (Serial.available()>0)
  {
  incomingByte=Serial.read();}
   //open right port
 
   
if (incomingByte==6)//close all port
{
  EnableWaterRight=false;
  EnableWaterLeft=false;
  EnableStart= false; 
  servo_right.write(servoopen);
  servo_left.write(servoopen);
  delay(TIME_IT_TAKES_FOR_DOOR_TO_OPEN);
  servo_right.write(servostop);
  servo_left.write(servostop);
  delay(30000);
  return;
}
    
   if (incomingByte==3)//Matlab sent "3": right
   { EnableWaterRight=true;
     EnableStart=false;
     EnableWaterLeft=false;
     Serial1.flush();
  }

   if(EnableWaterRight&&RightSensorReading>100)
   {
     EnableWaterRight=false; // do this only once
     data.correct_number_right++;
     EnableStart=true; // the mouse deserves a reward at the start port as well
     RightWaterIsCurrentlyOpen=true;
     TimeOfRightWaterOpening= millis();
     digitalWrite(ENRight, LOW);        //Pull enable pin low to set FETs active and allow motor control
     StepForwardDefaultRight(); //CONSIDER change it to slower water delivery!!! ASK BRIAN  
     servo_left.write(servoclose); //close the left door
     delay(TIME_IT_TAKES_FOR_DOOR_TO_CLOSE);
     servo_left.write(servostop);
     StartShouldOpenLeftDoor=true;//wait the mouse to go back to startport to initiate the trial
  }
   
  // if the port is open and has been open longer that PORT_OPEN_TIME, close it
   if(RightWaterIsCurrentlyOpen == true && millis() - TimeOfRightWaterOpening >= PUMP_OPEN_TIME)//The time can also be written in matlab.
   {
   resetBEDPinsRight();
   incomingByte=0;
   RightWaterIsCurrentlyOpen = false;}

     if(EnableWaterRight&&LeftSensorReading>100) //ERROR//close both doors
   {
    EnableWaterRight = false; // do this only once. Raly: I added this, but maybe I shouldn't have. Is it used somewhere else?
    RightDoorIsCurrentlyClosing=true;//RightDoorIsCurrentlyClosing=RightDoorStartOpen
    LeftDoorIsCurrentlyClosing=true;//LeftDoorIsCurrentlyClosing=LefttDoorStartOpen
    servo_left.write(servoclose); //we need to close both doors
    servo_right.write(servoclose);
    TimeOfLeftDoorClosing= millis();
    TimeOfRightDoorClosing= millis();
    StartShouldOpenRightDoor=true;// the right door should eventually open only when the animal starts a new trial
    ImposeDelayBeforeOpeningLeftDoor = true; // this blocks the mouse in the left arm for the delay
   }

    if (ImposeDelayBeforeOpeningLeftDoor && LeftDoorIsCurrentlyClosing==false) // if the door finished closing, we should wait for the punishment waiting time
    {
      delay(DOOR_WAIT_TIME); // Open left door after the punishment waiting time
    servo_left.write(servoopen);
    delay(TIME_IT_TAKES_FOR_DOOR_TO_OPEN);
    servo_left.write(servostop);
    ImposeDelayBeforeOpeningLeftDoor = false; // do this only once
    }

//    // I am commenting these out because I have the exact same thing lower down
//    if (LeftDoorIsCurrentlyClosing && millis() - TimeOfLeftDoorClosing >= TIME_IT_TAKES_FOR_DOOR_TO_CLOSE)
//    {
//    servo_left.write(servostop);
//    LeftDoorIsCurrentlyClosing = false; // do this only once
//    }
//    if (RightDoorIsCurrentlyClosing && millis() - TimeOfRightDoorClosing >= TIME_IT_TAKES_FOR_DOOR_TO_CLOSE)
//    {
//    servo_right.write(servostop);
//    RightDoorIsCurrentlyClosing = false; // do this only once
//    }
   
  if(StartShouldOpenRightDoor &&StartSensorReading<10)//Restart the trial
  {
    StartShouldOpenRightDoor= false; // only do this once
    servo_right.write(servoopen);
    RightDoorIsCurrentlyOpening = true;
    TimeOfRightDoorOpening = millis();
  }
  
  if (RightDoorIsCurrentlyOpening && millis() - TimeOfRightDoorOpening >= TIME_IT_TAKES_FOR_DOOR_TO_OPEN)
  {
    servo_right.write(servostop);
    RightDoorIsCurrentlyOpening = false; // do this only once
  } 
   
 //*************************************************************************
   //open left port
   if (incomingByte==2)//Matlab send "2": left
   { EnableWaterLeft=true;
     EnableStart=false;
     EnableWaterRight=false;
     Serial1.flush();
   }
   if(EnableWaterLeft&&LeftSensorReading>100)//correct
   {
     data.correct_number_left ++;
     EnableStart=true; // the mouse deserves a reward at the start port as well
     LeftWaterIsCurrentlyOpen=true;
     TimeOfLeftWaterOpening= millis();
     digitalWrite(ENLeft, LOW);        //Pull enable pin low to set FETs active and allow motor control
     StepForwardDefaultLeft(); //CONSIDER change it to slower water delivery!!! ASK BRIAN  
     EnableWaterLeft=false;
     servo_right.write(servoclose); //close the right door
     delay(TIME_IT_TAKES_FOR_DOOR_TO_CLOSE);
     servo_right.write(servostop); // stop closing the right door
     StartShouldOpenRightDoor=true;//wait the mouse to go back to startport to initiate the trial
     }
  // if the port is open and has been open longer that PORT_OPEN_TIME, close it
   if(LeftWaterIsCurrentlyOpen == true && millis() - TimeOfLeftWaterOpening >= PUMP_OPEN_TIME)
   {
   resetBEDPinsLeft();
       incomingByte=0;
       LeftWaterIsCurrentlyOpen = false;
   //water_open = false;
   }
  
//if error
      if(EnableWaterLeft&&RightSensorReading>100)//ERROR//close both door
   {
    EnableWaterLeft = false; // do this only once. Raly: I added this, but maybe I shouldn't have. Is it used somewhere else?
    servo_left.write(servoclose); //we need to close both doors
    servo_right.write(servoclose);
    LeftDoorIsCurrentlyClosing=true;
    RightDoorIsCurrentlyClosing=true;
    TimeOfLeftDoorClosing = millis();
    TimeOfRightDoorClosing = millis();
    StartShouldOpenLeftDoor=true;// the left door should eventually open only when the animal starts a new trial
    ImposeDelayBeforeOpeningRightDoor = true; // this blocks the mouse in the right arm for the delay
   }

   if (LeftDoorIsCurrentlyClosing && millis() - TimeOfLeftDoorClosing >= TIME_IT_TAKES_FOR_DOOR_TO_CLOSE)
   {
    servo_left.write(servostop);
    LeftDoorIsCurrentlyClosing = false; // do this only once
   }
   
   if (RightDoorIsCurrentlyClosing && millis() - TimeOfRightDoorClosing >= TIME_IT_TAKES_FOR_DOOR_TO_CLOSE)
   {
    servo_right.write(servostop);
    RightDoorIsCurrentlyClosing = false; // do this only once
   }
   if (ImposeDelayBeforeOpeningRightDoor && RightDoorIsCurrentlyClosing==false) // if the door finished closing, we should wait for the punishment waiting time
    {
      delay(DOOR_WAIT_TIME); // Open right door after the punishment waiting time
    servo_right.write(servoopen);
    delay(TIME_IT_TAKES_FOR_DOOR_TO_OPEN);
    servo_right.write(servostop);
    ImposeDelayBeforeOpeningRightDoor = false; // do this only once
    }
        
  if(StartShouldOpenLeftDoor&&StartSensorReading<10)//Restart the trial
  {
    StartShouldOpenLeftDoor= false; // only do this once
    servo_left.write(servoopen);
    LeftDoorIsCurrentlyOpening = true;
    TimeOfLeftDoorOpening = millis();
    }

  if (LeftDoorIsCurrentlyOpening && millis() - TimeOfLeftDoorOpening >= TIME_IT_TAKES_FOR_DOOR_TO_OPEN)
  {
    servo_left.write(servostop);
    LeftDoorIsCurrentlyOpening = false; // do this only once
  }
  
 if (incomingByte==1)//Matlab sent "1": start arm

   { // EnableStart=true; // I put this as true only in correct trials (when incomingByte==2 or 3)
     EnableWaterRight=false;
     EnableWaterLeft=false;
     Serial1.flush();
   }
   
   if(EnableStart&&StartSensorReading<10&&mousemoving)
   {
     data.correct_number_start ++;
     StartWaterIsCurrentlyOpen=true;
     TimeOfStartWaterOpening= millis();
     digitalWrite(ENStart, LOW);        //Pull enable pin low to set FETs active and allow motor control
     StepForwardDefaultStart(); //CONSIDER change it to slower water delivery!!! ASK BRIAN
     EnableStart=false;  
  }
  // if the port is open and has been open longer that PORT_OPEN_TIME, close it
   if(StartWaterIsCurrentlyOpen == true && millis() - TimeOfStartWaterOpening >= PUMP_OPEN_TIME_START)
   {
  
   resetBEDPinsStart();
       incomingByte=0;
   StartWaterIsCurrentlyOpen=false;
   mousemoving=false;
   //water_open = false; 
   }

 //*************************************************************************
 if (incomingByte==4)//sending data to matlab as 16 digit byte
  {
      Serial.write((byte*)&data,1*7);
      incomingByte==0;
      data.readleft=0;
      data.readright=0;
      data.readstart=0;
      Serial1.flush();
   }

if (incomingByte==5)//screen has been changed
  {
     digitalWrite(DisplayTTL,HIGH);
     incomingByte==0;
     Serial1.flush();
    }
    }
