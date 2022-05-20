#include <math.h> 
//Decalare pin
#define pump_1 36 // back
#define pump_2 37 // front
#define portIRsensor_1 A0  // back
#define portIRsensor_2 A3  // front

//#define portLED_1 28 // back
//#define portLED_2 29 // front
#define armIRsensor_1 A1 //For arm_back
#define armIRsensor_2 A2 //For arm_front
#define TTLport 1 //send TTL to intan with info from mouse port 1 , 2 and 3
#define TTLarm_1 2  //send TTL to intan with info from arm  1 , 2 and 3
#define TTLarm_2 3  //send TTL to intan with info from beginsensor 1 and 2  [armIRsensor_4  and armIRsensor_5 ]

unsigned long PUMP_OPEN_TIME = 100;

int portIRsensorReading_1 = 0;
int portIRsensorReading_2 = 0;
int armIRsensorReading_1 = 0;
int armIRsensorReading_2 = 0;

boolean EnableWater_1 = false;
boolean EnableWater_2 = false;
boolean water_open_1 = false;
boolean water_open_2 = false;
unsigned long Water_1;
unsigned long Water_2;
int current_state = 0;
int previous_state = 0;
int corret_trial = 0;
int error_trial = 0;
int trial_num = 0;
int val = 254;

void setup() {
  // put your setup code here, to run once:
    pinMode(pump_1, OUTPUT);
    pinMode(pump_2, OUTPUT);
    pinMode(portLED_1, OUTPUT);
    pinMode(portLED_2, OUTPUT);
    pinMode(portIRsensor_1, INPUT);
    pinMode(portIRsensor_2, INPUT);
    pinMode(armIRsensor_1, INPUT);
    pinMode(armIRsensor_2, INPUT);
    pinMode(TTLport, OUTPUT);
    pinMode(TTLarm_1, OUTPUT);
    pinMode(TTLarm_2, OUTPUT);
    current_state = 1;
    digitalWrite(portLED_1, HIGH);
    digitalWrite(portLED_2, HIGH);
    Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  portIRsensorReading_2 = analogRead(portIRsensor_2);
  Serial.println(portIRsensorReading_2);
}
