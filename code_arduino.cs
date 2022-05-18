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
#define pump_1 36 // left
#define pump_2 37 // middle
#define pump_3 38 // right
#define portIRsensor_1 A5  // left
#define portIRsensor_2 A6  // middle
#define portIRsensor_3 A7  // right
#define portLED_1 28 // left
#define portLED_2 29 // middle
#define portLED_3 30 // right
#define armIRsensor_1 A0 //For arm_left
#define armIRsensor_2 A1 //For arm_middle
#define armIRsensor_3 A2 //For arm_right
#define armIRsensor_4 A3 //For begin_left
#define armIRsensor_5 A4 //For begin_right
#define TTLport 1 //send TTL to intan with info from mouse port 1 , 2 and 3
#define TTLarm_1 2  //send TTL to intan with info from arm  1 , 2 and 3
#define TTLarm_2 3  //send TTL to intan with info from beginsensor 1 and 2  [armIRsensor_4  and armIRsensor_5 ]
int servoopen_1 = 0;
int servoclose_1 = 90;
int servoopen_2 = 180;
int servoclose_2 = 90;
int servoopen_3 = 180;
int servoclose_3 = 90;
int servoopen_4 = 180;
int servoclose_4 = 115;
int servoopen_5 = 12;
int servoclose_5 = 63;
unsigned long PUMP_OPEN_TIME = 100;
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
int current_state = 0;
int previous_state = 0;
int corret_trial = 0;
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
    current_state = 1;
    servo1.write(servoopen_1);
    servo2.write(servoopen_2);
    servo3.write(servoopen_3);
    servo4.write(servoclose_4);
    servo5.write(servoclose_5);
    digitalWrite(portLED_1, HIGH);
    digitalWrite(portLED_2, HIGH);
    digitalWrite(portLED_3, HIGH);
    Serial.begin(9600);
}

//rule : mice need to choose correct arm + correct port // could be only correct port only/ correct arm only 

void loop()
{
    // put your main code here, to run repeatedly:

    // mouse run in a order: state 1: left state 2: middle state 3: right

    //Define: Correct=Left
    Serial.println(current_state);
    portIRsensorReading_1 = analogRead(portIRsensor_1);
    portIRsensorReading_2 = analogRead(portIRsensor_2);
    portIRsensorReading_3 = analogRead(portIRsensor_3);
    armIRsensorReading_1 = analogRead(armIRsensor_1);
    armIRsensorReading_2 = analogRead(armIRsensor_2);
    armIRsensorReading_3 = analogRead(armIRsensor_3);
    armIRsensorReading_4 = analogRead(armIRsensor_4);
    armIRsensorReading_5 = analogRead(armIRsensor_5);
    //analogWrite(TTLport, round(val / 2));
    //analogWrite(TTLport, round(val / 4));
    //analogWrite(TTLport, round(val / 8));
    //analogWrite(TTLarm_1, round(val / 2));
    //analogWrite(TTLarm_1, round(val / 4));
    //analogWrite(TTLarm_1, round(val / 8));
    //analogWrite(TTLarm_2, round(val / 2));
    //analogWrite(TTLarm_2, round(val / 4));
    //analogWrite(TTLarm_2, round(val / 8));


    if (current_state == 1)
    {
        EnableWater_1 = true;

    }

    //Define: Correct=middle
    if (current_state == 2)
    {
        EnableWater_2 = true;
    }
    //Define: Correct=right
    if (current_state == 3)
    {
        EnableWater_3 = true;
    }


    //It is time for choosing!
    // All door closed
    //if (armIRsensorReading_1 < 500 || armIRsensorReading_2 < 500 || armIRsensorReading_3 < 500) //If mice always go back without trying the mouse port delete "//"
    if (portIRsensorReading_1 > 500 || portIRsensorReading_2 > 500 || portIRsensorReading_3 > 500)
    {
        servo1.write(servoclose_1);
        servo2.write(servoclose_2);
        servo3.write(servoclose_3);
    }

    // Wrong arm notes: open doors in the begining arm + LED off
    // Strict rule: even a mouse first lick correct port but chooses the wrong arm, we will count it as a failure. Loose rule, after mice lick the correct port they will get water.
    if ((EnableWater_1 && armIRsensorReading_2 < 500) || (EnableWater_1 && armIRsensorReading_3 < 500))
    {
        error_trial += 1;
        EnableWater_1 = false;
        current_state = 0;
        previous_state = 3;//keep run state 1
        servo4.write(servoopen_4);
        servo5.write(servoopen_5);
        digitalWrite(portLED_1, LOW);
        digitalWrite(portLED_2, LOW);
        digitalWrite(portLED_3, LOW);

    }
    if ((EnableWater_2 && armIRsensorReading_1 < 500) || (EnableWater_2 && armIRsensorReading_3 < 500))
    {
        error_trial += 1;
        EnableWater_1 = false;
        current_state = 0;
        previous_state = 1;//keep run state 2
        servo4.write(servoopen_4);
        servo5.write(servoopen_5);
        digitalWrite(portLED_1, LOW);
        digitalWrite(portLED_2, LOW);
        digitalWrite(portLED_3, LOW);
    }
    if ((EnableWater_3 && armIRsensorReading_1 < 500) || (EnableWater_3 && armIRsensorReading_2 < 500))
    {
        error_trial += 1;
        EnableWater_1 = false;
        current_state = 0;
        previous_state = 2;//keep run state 3
        servo4.write(servoopen_4);
        servo5.write(servoopen_5);
        digitalWrite(portLED_1, LOW);
        digitalWrite(portLED_2, LOW);
        digitalWrite(portLED_3, LOW);
    }
    // Wrong port: open doors in the begining arm + LED off
    if (current_state != 0)
    {
        if ((EnableWater_1 && portIRsensorReading_2 > 500) || (EnableWater_1 && portIRsensorReading_3 > 500))
        {
            error_trial += 1;
            EnableWater_1 = false;
            current_state = 0;
            previous_state = 3;//keep run state 1
            servo4.write(servoopen_4);
            servo5.write(servoopen_5);
            digitalWrite(portLED_1, LOW);
            digitalWrite(portLED_2, LOW);
            digitalWrite(portLED_3, LOW);

        }
        if ((EnableWater_2 && portIRsensorReading_1 > 500) || (EnableWater_2 && portIRsensorReading_3 > 500))
        {
            error_trial += 1;
            EnableWater_1 = false;
            current_state = 0;
            previous_state = 1;//keep run state 2
            servo4.write(servoopen_4);
            servo5.write(servoopen_5);
            digitalWrite(portLED_1, LOW);
            digitalWrite(portLED_2, LOW);
            digitalWrite(portLED_3, LOW);
        }
        if ((EnableWater_3 && portIRsensorReading_2 > 500) || (EnableWater_3 && portIRsensorReading_1 > 500))
        {
            error_trial += 1;
            EnableWater_1 = false;
            current_state = 0;
            previous_state = 2;//keep run state 3
            servo4.write(servoopen_4);
            servo5.write(servoopen_5);
            digitalWrite(portLED_1, LOW);
            digitalWrite(portLED_2, LOW);
            digitalWrite(portLED_3, LOW);
        }


        // Correct trail Correct=Left
        if (EnableWater_1 && portIRsensorReading_1 > 500)
        {
            Water_1 = millis();
            EnableWater_1 = false;
            water_open_1 = true;
            digitalWrite(pump_1, HIGH);
            servo4.write(servoopen_4);
            servo5.write(servoopen_5);
            current_state = 0;
            previous_state = 1;//wait until mice learn port 1 is correct
            corret_trial += 1;


        }
        if (water_open_1 && millis() - Water_1 >= PUMP_OPEN_TIME)
        {
            digitalWrite(pump_1, LOW);
            water_open_1 = false;
        }

        // Correct trail Correct=middle
        if (EnableWater_2 && portIRsensorReading_2 > 500)
        {
            Water_2 = millis();
            digitalWrite(pump_2, HIGH);
            EnableWater_2 = false;
            water_open_2 = true;
            servo4.write(servoopen_4);
            servo5.write(servoopen_5);
            current_state = 0;
            previous_state = 2;
        }
        if (water_open_2 && millis() - Water_2 >= PUMP_OPEN_TIME)
        {
            digitalWrite(pump_2, LOW);
            water_open_2 = false;

        }

        //Open right port Correct=right
        if (EnableWater_3 && portIRsensorReading_3 > 500)
        {
            Water_3 = millis();
            digitalWrite(pump_3, HIGH);
            EnableWater_3 = false;
            water_open_3 = true;
            current_state = 0;
            previous_state = 3;
            servo4.write(servoopen_4);
            servo5.write(servoopen_5);
        }
        if (water_open_3 && millis() - Water_3 >= PUMP_OPEN_TIME)
        {
            digitalWrite(pump_3, LOW);
            water_open_3 = false;

        }
    }
    if (current_state == 0) //a mouse goes back and reset the task
    {
        if (armIRsensorReading_4 < 500 || armIRsensorReading_5 < 500)
        {
            trial_num += 1;
            servo4.write(servoclose_4);
            servo5.write(servoclose_5);
            digitalWrite(portLED_1, HIGH);
            digitalWrite(portLED_2, HIGH);
            digitalWrite(portLED_3, HIGH);
            if (previous_state == 1)
            {
                current_state = 2;
                previous_state = 0;
            }
            if (previous_state == 2)
            {
                current_state = 3;
                previous_state = 0;
            }
            if (previous_state == 3)
            {
                current_state = 1;
                previous_state = 0;
            }

        }

    }

}