#include <math.h> 
//Decalare pin
#define pump_1 36 // front
#define pump_2 37 // back
#define portIRsensor_1 A5  // front
#define portIRsensor_2 A6  // back
#define armIRsensor_1 A0 // front
#define armIRsensor_2 A1 // back
#define TTLport 1 //send TTL to intan with info from mouse port 1 , 2 and 3
#define TTLarm_1 2  //send TTL to intan with info from arm  1 , 2 and 3
#define TTLarm_2 3  //send TTL to intan with info from beginsensor 1 and 2  [armIRsensor_4  and armIRsensor_5 ]
unsigned long PUMP_OPEN_TIME = 100;

int portIRsensorReading_1 = 0; // front
int portIRsensorReading_2 = 0; // back
int armIRsensorReading_1 = 0; // front
int armIRsensorReading_2 = 0; // back

boolean EnableWater_1 = false;
boolean EnableWater_2 = false;
boolean water_open_1 = false;
boolean water_open_2 = false;
unsigned long Water_1;
unsigned long Water_2;
int current_state = -1; //0 indicates front, 1 indicates back
int previous_state = -1; //0 indicates front, 1 indicates back
int error_trial = 0;
int trial_num = 0;
int val = 254;
void setup()
{
    // put your setup code here, to run once:
    pinMode(pump_1, OUTPUT);
    pinMode(pump_2, OUTPUT);
    pinMode(portIRsensor_1, INPUT);
    pinMode(portIRsensor_2, INPUT);
    pinMode(armIRsensor_1, INPUT);
    pinMode(armIRsensor_2, INPUT);
    pinMode(TTLport, OUTPUT);
    pinMode(TTLarm_1, OUTPUT);
    pinMode(TTLarm_2, OUTPUT);
    Serial.begin(9600);
}

//rule: mouse must alternate ports in order to receive a water reward 

void loop()
{
    // put your main code here, to run repeatedly:

    // mouse must alternate, current_state cannot equal previous_state

    portIRsensorReading_1 = analogRead(portIRsensor_1);
    portIRsensorReading_2 = analogRead(portIRsensor_2);
    armIRsensorReading_1 = analogRead(armIRsensor_1);
    armIRsensorReading_2 = analogRead(armIRsensor_2);
    //analogWrite(TTLport, round(val / 2));
    //analogWrite(TTLport, round(val / 4));
    //analogWrite(TTLport, round(val / 8));
    //analogWrite(TTLarm_1, round(val / 2));
    //analogWrite(TTLarm_1, round(val / 4));
    //analogWrite(TTLarm_1, round(val / 8));
    //analogWrite(TTLarm_2, round(val / 2));
    //analogWrite(TTLarm_2, round(val / 4));
    //analogWrite(TTLarm_2, round(val / 8));


    //It is time for choosing!
   

    // Wrong arm notes: previous state = current state
    if (armIRsensorReading_1 < 100) && (previous_state != 0) // for back arm, 1000 is base, 30 is detecting
    {
        EnableWater_1 = true;
        EnableWater_2 = false;
        previous_state = 0;
    }
    if (armIRsensorReading_2 > 100) && (previous_state != 1) // for front arm, 1000 is base, 30 is detecting
    {
        EnableWater_1 = false;
        EnableWater_2 = true;
        previous_state = 1;
    }
        // Correct trial FRONT
        if (EnableWater_1)&&(portIRsensorReading_1 < 100) // should give water and animal is at lick port (30 is detecting)
        {
            Water_1 = millis();
            EnableWater_1 = false;
            water_open_1 = true;
        }
        while (water_open_1 && ((millis() - Water_1) < PUMP_OPEN_TIME)
        {
            digitalWrite(pump_1, HIGH);
        }
        if (water_open_1 && ((millis() - Water_1) >= PUMP_OPEN_TIME)
        {
          digitalWrite(pump_1, LOW);
          water_open_1 = false;
        }
        // Correct trial BACK
        if (EnableWater_2)&&(portIRsensorReading_2 < 100) // 30 is detecting
        {
            Water_2 = millis();
            EnableWater_2 = false;
            water_open_2 = true;
        }
        while (water_open_2 && ((millis() - Water_2) < PUMP_OPEN_TIME)
        {
            digitalWrite(pump_2, HIGH);
        }
        if (water_open_2 && ((millis() - Water_2) >= PUMP_OPEN_TIME)
        {
          digitalWrite(pump_2, LOW);
          water_open_2 = false;
        }
}
