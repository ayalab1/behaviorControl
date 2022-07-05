#include <Servo.h> //library for servo motor.
#include <math.h> 
Servo servo1; //For door left
Servo servo2; //For door middle
Servo servo3; //For door right
Servo servo4; //For door begin_left
Servo servo5; //For door begin_right
//Decalare pin
#define servo_1 31
#define servo_2 32
#define servo_3 33
#define servo_4 34
#define servo_5 35
#define pump_1 38 // left
#define pump_2 37 // middle
#define pump_3 36 // right
#define portIRsensor_1 A5  // left
#define portIRsensor_2 A6  // middle
#define portIRsensor_3 A7  // right
#define portLED_1 28 // left
#define portLED_2 29 // middle
#define portLED_3 30 // right
#define armIRsensor_1 A4 //For arm_left
#define armIRsensor_2 A3 //For arm_middle
#define armIRsensor_3 A2 //For arm_right
#define armIRsensor_4 A1 //For begin_left
#define armIRsensor_5 A0 //For begin_right
#define TTLport 2 //send TTL to intan with info from mouse port 1 , 2 and 3
#define TTLarm_1 3  //send TTL to intan with info from arm  1 , 2 and 3
#define TTLarm_2 4  //send TTL to intan with info from beginsensor 1 and 2  [armIRsensor_4  and armIRsensor_5 ]
#define TTLperm 5 //send TTL to intan with info from correct or trial_num
int servoopen_1 = 0;
int servoclose_1 = 90;
int servoopen_2 = 105;
int servoclose_2 = 180; 
int servoopen_3 = 110;
int servoclose_3 = 180;
int servoopen_4 = 180;
int servoclose_4 = 130;
int servoopen_5 = 22;
int servoclose_5 = 63;
int position_target_1 = 1000;
int position_target_2 = 1000;
int position_target_3 = 1000;
int position_target_4 = 1000;
int position_target_5 = 1000;
unsigned long PUMP_OPEN_TIME = 100;// 35-40
unsigned long DOOR_WAIT_TIME = 5000;

int portIRsensorReading_1 = 0;
int portIRsensorReading_2 = 0;
int portIRsensorReading_3 = 0;
int armIRsensorReading_1 = 0;
int armIRsensorReading_2 = 0;
int armIRsensorReading_3 = 0;
int armIRsensorReading_4 = 0;
int armIRsensorReading_5 = 0;


boolean EnableWater_1 = false;
boolean EnableWater_2 = false;
boolean EnableWater_3 = false;
boolean water_open_1 = false;
boolean water_open_2 = false;
boolean water_open_3 = false;
unsigned long Water_1;
unsigned long Water_2;
unsigned long Water_3;
static unsigned long t;
int current_state = 0;
int previous_state = 0;
int correct_trial = 0;
int error_trial = 0;
int trial_num = 0;
int val = 254;
void setup()
{
    // put your setup code here, to run once:
    servo1.attach(servo_1);
    servo2.attach(servo_2);
    servo3.attach(servo_3);
    servo4.attach(servo_4);
    servo5.attach(servo_5);
    pinMode(pump_1, OUTPUT);
    pinMode(pump_2, OUTPUT);
    pinMode(pump_3, OUTPUT);
    pinMode(portLED_1, OUTPUT);
    pinMode(portLED_2, OUTPUT);
    pinMode(portLED_3, OUTPUT);
    //pinMode(portIRsensor_1, INPUT);
    //pinMode(portIRsensor_2, INPUT);
    //pinMode(portIRsensor_3, INPUT);
    //pinMode(armIRsensor_1, INPUT);
    //pinMode(armIRsensor_2, INPUT);
    //pinMode(armIRsensor_3, INPUT);
    //pinMode(armIRsensor_4, INPUT);
    //pinMode(armIRsensor_5, INPUT);
    pinMode(TTLport, OUTPUT);
    pinMode(TTLarm_1, OUTPUT);
    pinMode(TTLarm_2, OUTPUT);
    pinMode(TTLperm, OUTPUT);
    current_state = 1; //current state=1 left arm open 2:middle 3: right
    if (position_target_1 != servoopen_1)
    {
        servo1.write(servoopen_1);
        position_target_1 = servoopen_1;
    }

    if (position_target_2 != servoclose_2)
    {
        servo2.write(servoclose_2);
        position_target_2 = servoclose_2;
    }
    if (position_target_3 != servoclose_3)
    {
        servo2.write(servoclose_3);
        position_target_3 = servoclose_3;
    }
    if (position_target_4 != servoclose_4)
    {
        servo4.write(servoclose_4);
        position_target_4 = servoclose_4;
    }
    if (position_target_5 != servoclose_5)
    {
        servo5.write(servoclose_5);
        position_target_5 = servoclose_5;
    }
    digitalWrite(portLED_1, HIGH);
    digitalWrite(portLED_2, HIGH);
    digitalWrite(portLED_3, HIGH);
    Serial.begin(9600);
    digitalWrite(pump_1, HIGH);
    //digitalWrite(pump_2, HIGH);
    //digitalWrite(pump_3, HIGH);
    delay(PUMP_OPEN_TIME);
    digitalWrite(pump_1, LOW);
    //digitalWrite(pump_2, LOW);
    //digitalWrite(pump_3, LOW);
    t = millis();
    Serial.print("t=");
    Serial.println(t);
    Serial.print("trial start");

}


//rule : mice need to choose one arm + one port // And come back to trigger task reset.

void loop()
{
    // put your main code here, to run repeatedly:

    // mouse run in a order: state 1: open the port state 2 :close the port

    //Define: Correct=Left

    portIRsensorReading_1 = analogRead(portIRsensor_1);
    portIRsensorReading_2 = analogRead(portIRsensor_2);
    portIRsensorReading_3 = analogRead(portIRsensor_3);
    armIRsensorReading_1 = analogRead(armIRsensor_1);
    armIRsensorReading_2 = analogRead(armIRsensor_2);
    armIRsensorReading_3 = analogRead(armIRsensor_3);
    armIRsensorReading_4 = analogRead(armIRsensor_4);
    armIRsensorReading_5 = analogRead(armIRsensor_5);

    // write TTL to intan

    if (portIRsensorReading_1 > 500)
    {
        analogWrite(TTLport, round(val / 2));
    }

    else if (portIRsensorReading_2 > 500)
    {
        analogWrite(TTLport, round(val / 4));
    }
    else if (portIRsensorReading_3 > 500)
    {
        analogWrite(TTLport, round(val / 8));
    }
    else
    {
        analogWrite(TTLport, 0);
    }
    if (armIRsensorReading_1 < 500)
    {
        analogWrite(TTLarm_1, round(val / 2));
    }

    else if (armIRsensorReading_2 < 500)
    {
        analogWrite(TTLarm_1, round(val / 4));
    }
    else if (armIRsensorReading_3 < 500)
    {
        analogWrite(TTLarm_1, round(val / 8));
    }
    else
    {
        analogWrite(TTLarm_1, 0);
    }
    if (armIRsensorReading_4 < 500)
    {
        analogWrite(TTLarm_2, round(val / 2));
    }
    else if (armIRsensorReading_5 < 500)
    {
        analogWrite(TTLarm_2, round(val / 8));
    }
    else
    {
        analogWrite(TTLarm_2, 0);

    }
    //   
    if (current_state == 1)
    {
        EnableWater_1 = true;
        EnableWater_2 = false;
        EnableWater_3 = false;

    }
    if (current_state == 2)
    {
        EnableWater_1 = false;
        EnableWater_2 = true;
        EnableWater_3 = false;

    }
    if (current_state == 3)
    {
        EnableWater_1 = false;
        EnableWater_2 = false;
        EnableWater_3 = true;

    }


    //It is time for choosing!
    //if (armIRsensorReading_1 < 500 || armIRsensorReading_2 < 500 || armIRsensorReading_3 < 500) //If mice always go back without trying the mouse port delete "//"
    if (portIRsensorReading_1 > 500 || portIRsensorReading_2 > 500 || portIRsensorReading_3 > 500)
    {
        if (position_target_1 != servoclose_1)
        {
            servo1.write(servoclose_1);
            position_target_1 = servoclose_1;
        }
        if (position_target_2 != servoclose_2)
        {
            servo2.write(servoclose_2);
            position_target_2 = servoclose_2;
        }
        if (position_target_3 != servoclose_3)
        {
            servo3.write(servoclose_3);
            position_target_3 = servoclose_3;
        }
        if (position_target_4 != servoopen_4)
        {
            servo4.write(servoopen_4);
            position_target_4 = servoopen_4;
        }
        if (position_target_5 != servoopen_5)
        {
            servo5.write(servoopen_5);
            position_target_5 = servoopen_5;
        }
    }


    //wrong trial: a mouse has chose one arm but tried the other ports.
    // Wrong arm/port notes: open doors in the begining arm + LED off
    // Strict rule: even a mouse first lick correct port but chooses the wrong arm, we will count it as a failure. Loose rule, after mice lick the correct port they will get water.
    if (EnableWater_1 && (portIRsensorReading_2 > 500 || portIRsensorReading_3 > 500 ))
    {

        error_trial += 1;
        EnableWater_1 = false;
        current_state = 0;
        previous_state = 3;// hold current_state=1
        digitalWrite(portLED_1, LOW);
        digitalWrite(portLED_2, LOW);
        digitalWrite(portLED_3, LOW);
    }
    if (EnableWater_2 && ( portIRsensorReading_1 > 500 || portIRsensorReading_3 > 500 ))
    {
        error_trial += 1;
        EnableWater_2 = false;
        previous_state = 1;  // hold current_state=2
        current_state = 0;
        digitalWrite(portLED_1, LOW);
        digitalWrite(portLED_2, LOW);
        digitalWrite(portLED_3, LOW);
    }
    if (EnableWater_3 &&( portIRsensorReading_1 > 500 || portIRsensorReading_2 > 500))
    {
        error_trial += 1;
        EnableWater_3 = false;
        digitalWrite(portLED_1, LOW);
        digitalWrite(portLED_2, LOW);
        digitalWrite(portLED_3, LOW);
        current_state = 0;
        previous_state = 2;// hold current_state=3
    }


    // Correct trail a mouse chooses left arm, Correct=Left
    if (EnableWater_1 && portIRsensorReading_1 > 500)
    {
        correct_trial += 1;
        Water_1 = millis();
        EnableWater_1 = false;
        water_open_1 = true;
        digitalWrite(pump_1, HIGH);
        current_state = 0;
        previous_state = 1;//will change state to 2
        analogWrite(TTLperm, round(val / 2));


    }
    if (water_open_1 && millis() - Water_1 >= PUMP_OPEN_TIME)
    {
        digitalWrite(pump_1, LOW);
        water_open_1 = false;
        analogWrite(TTLperm, round(0));
    }

    // Correct trail a mouse chooses middle arm, Correct=middle
    if (EnableWater_2 && portIRsensorReading_2 > 500)
    {
        correct_trial += 1;
        Water_2 = millis();
        digitalWrite(pump_2, HIGH);
        EnableWater_2 = false;
        water_open_2 = true;
        current_state = 0;
        previous_state = 2;//will change state to 3
        analogWrite(TTLperm, round(val / 2));

    }
    if (water_open_2 && millis() - Water_2 >= PUMP_OPEN_TIME)
    {
        digitalWrite(pump_2, LOW);
        water_open_2 = false;
        analogWrite(TTLperm, round(0));

    }

    // Correct trail a mouse chooses right arm, Correct=right
    if (EnableWater_3 && portIRsensorReading_3 > 500)
    {
        correct_trial += 1;
        Water_3 = millis();
        digitalWrite(pump_3, HIGH);
        EnableWater_3 = false;
        water_open_3 = true;
        current_state = 0;
        previous_state = 3;//will change state to 1
        analogWrite(TTLperm, round(val / 2));
    }
    if (water_open_3 && millis() - Water_3 >= PUMP_OPEN_TIME)
    {
        digitalWrite(pump_3, LOW);
        water_open_3 = false;
        analogWrite(TTLperm, round(0));

    }
    if (current_state == 0) //a mouse goes back and reset the task
    {
        if (armIRsensorReading_4 < 500 || armIRsensorReading_5 < 500)
        {
            analogWrite(TTLperm, round(val / 4));
            delay(500);
            if (position_target_1 != servoopen_1)
            {
                servo1.write(servoopen_1);
                
                position_target_1 = servoopen_1;
            }
            if (position_target_2 != servoopen_2)
            {
                servo2.write(servoopen_2);
                position_target_2 = servoopen_2;
            }
            if (position_target_3 != servoopen_3)
            {
                servo3.write(servoopen_3);
                position_target_3 = servoopen_3;
            }
            if (position_target_4 != servoclose_4)
            {
                servo4.write(servoclose_4);
                position_target_4 = servoclose_4;
            }
            if (position_target_5 != servoclose_5)
            {
                servo5.write(servoclose_5);
                position_target_5 = servoclose_5;
            }
            digitalWrite(portLED_1, HIGH);
            digitalWrite(portLED_2, HIGH);
            digitalWrite(portLED_3, HIGH);

            if (previous_state == 2)
            {
                current_state = 1;
                previous_state = 0;
            }

            if (previous_state == 1)
            {
                current_state = 3;
                previous_state = 0;
            }

            if (previous_state == 3)
            {
                current_state = 2;
                previous_state = 0;
            }

            trial_num += 1;
            t = millis();
            Serial.print("t=");
            Serial.println(t);
            Serial.print("trial_num=");
            Serial.println(trial_num);
            Serial.print("correct_trial=");
            Serial.println(correct_trial);
            Serial.print("error_trial=");
            Serial.println(error_trial);
            Serial.print("current_state=");
            Serial.println(current_state);
            analogWrite(TTLperm, round(0));
        }
    }



}
