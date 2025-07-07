// Trial conditions
int num_laps = 2;
boolean CCW = true;
boolean full_circle = true; //if using half circle, should always start down middle

// Set up servos
#include <Servo.h>
#include <math.h>

Servo servo1_left;
Servo servo1_middle;
Servo servo1_right;

#define servo_right 30
#define servo_middle 31
#define servo_left 32

int open_right = 95;
int closed_right = 40;
int open_middle = 155;
int block_right = 105; //middle door
int block_left = 60; //middle door
int open_left = 60;
int closed_left = 0;

// Set up water pump
#define pump_1 36
unsigned long PUMP_OPEN_TIME = 100;
unsigned long pump_ct;

// Set up IR sensors
int IR_water = A10;
int read_water = 0;
int IR_right = A11;
int read_right = 0;
int IR_middle = A12;
int read_middle = 0;
int IR_left = A13;
int read_left = 0;
int IR_thresh = 500; //this should work for all. >500 is empty, <500 detects mouse

// Set up behavior
int arm_ct = 0;
int atWater = 0;
int atRight = 1;
int atMiddle = 2;
int atLeft = 3; 
int prev_loc = 0; 
int next_loc = 0;

int water_delay = 5000;

void setMotors(int setRight, int setMiddle, int setLeft){
  servo1_right.write(setRight);
  servo1_middle.write(setMiddle);
  servo1_left.write(setLeft);
}

void setup() {
  // put your setup code here, to run once:
  servo1_right.attach(servo_right);
  servo1_middle.attach(servo_middle);
  servo1_left.attach(servo_left);

  //mouse is starting in home port. Depending on trial setup, need to start in a different configuration
  // full circle, CCW
  if ((full_circle) && (CCW))
  {
    setMotors(open_right, open_middle, closed_left);
    next_loc = atRight;
  }
  // full circle, CW
  else if ((full_circle) && (!CCW))
  {
    setMotors(closed_right, open_middle, open_left);
    next_loc = atLeft;
  }
  // half circle, CCW
  else if ((!full_circle) && (CCW))
  {
    setMotors(closed_right, block_right, closed_left);//right permanently closed for this trial type
    next_loc = atMiddle;
  }
  // half circle, CW
  else if ((!full_circle) && (!CCW))
  {
    setMotors(closed_right, block_left, closed_left);//left permanently closed for this trial type
    next_loc = atMiddle;
  }

  pinMode(pump_1, OUTPUT);
  digitalWrite(pump_1, LOW);

  pinMode(IR_water, INPUT);
  pinMode(IR_right, INPUT);
  pinMode(IR_middle, INPUT);
  pinMode(IR_left, INPUT);

  Serial.begin(9600);
}

void loop() {
  // get initial IR sensor values
  read_water = analogRead(IR_water);
  read_right = analogRead(IR_right);
  read_middle = analogRead(IR_middle);
  read_left = analogRead(IR_left);

  if ((full_circle) && (CCW))
  {
    if (((read_right < IR_thresh) && ((prev_loc == atWater)||(prev_loc == atLeft))) && (next_loc == atRight)) //passes right IR coming from water port OR left if the mouse doesn't stop at water port
    {
      Serial.print('\n');
      Serial.print("Right arm");
      arm_ct = arm_ct + 1;
      setMotors(closed_right, open_middle, open_left);
      prev_loc = atRight; 
      next_loc = atLeft;
    }
    if (((read_left < IR_thresh) && (prev_loc == atRight) && (next_loc == atLeft))) //passes left IR coming from right arm
    {
      //Serial.print("Left arm");
      arm_ct = arm_ct + 1;
      prev_loc = atLeft;
      if (round(arm_ct / 2) < num_laps)
      {
        Serial.print('\n');
        Serial.print("Left arm no water");
        setMotors(open_right, block_left, open_left);
        next_loc = atRight;
      }
      else
      {
        Serial.print('\n');
        Serial.print("Left arm ready for water");
        setMotors(closed_right, block_left, open_left);
        next_loc = atWater;
      }
    }
    if ((read_water < IR_thresh) && (prev_loc == atLeft) && (next_loc == atWater))
    {
      //Serial.print("Water port");
      prev_loc = atWater;
      next_loc = atRight;
      if (round(arm_ct / 2) == num_laps)
      {
        //Serial.print("Dispensing");
        Serial.print('\n');
        Serial.print("Water, dispensing");
        setMotors(closed_right, open_middle, closed_left);
        pump_ct = millis();
        while ((millis() - pump_ct) < PUMP_OPEN_TIME)
        {
          digitalWrite(pump_1, HIGH);
        }
        if ((millis() - pump_ct) >= PUMP_OPEN_TIME)
        {
          digitalWrite(pump_1, LOW);
        }
        delay(water_delay);
        setMotors(open_right, open_middle, closed_left);
        Serial.print('\n');
        Serial.print("Released from water");
        arm_ct = 0;
      }
      else
      {
        setMotors(open_right, open_middle, closed_left);
        Serial.print('\n');
        Serial.print("Water, not dispensing");
      }
    }
  }
  else if ((full_circle) && (!CCW))
  {
    if (((read_left < IR_thresh) && ((prev_loc == atWater)||(prev_loc == atRight)) && (next_loc == atLeft))) //passes left IR coming from water port
    {
      //Serial.print("Left arm");
      arm_ct = arm_ct + 1;
      setMotors(open_right, open_middle, closed_left);
      prev_loc = atLeft;
      next_loc = atRight;
    }
    if ((read_right < IR_thresh) && (prev_loc == atLeft) && (next_loc == atRight)) //passes right IR coming from left arm
    {
      //Serial.print("Right arm");
      arm_ct = arm_ct + 1;
      prev_loc = atRight;      
      if (round(arm_ct / 2) < num_laps)
      {
        setMotors(open_right, block_right, open_left);
        next_loc = atLeft;
      }
      else
      {
        setMotors(open_right, block_right, closed_left);
        next_loc = atWater;
      }
    }
    if ((read_water < IR_thresh) && (prev_loc == atRight) && (next_loc == atWater))
    {
      //Serial.print("Water port");
      prev_loc = atWater;
      next_loc = atLeft;
      if (round(arm_ct / 2) == num_laps)
      {
        //Serial.print("Dispensing");
        setMotors(closed_right, open_middle, closed_left);
        pump_ct = millis();
        while ((millis() - pump_ct) < PUMP_OPEN_TIME)
        {
          digitalWrite(pump_1, HIGH);
        }
        if ((millis() - pump_ct) >= PUMP_OPEN_TIME)
        {
          digitalWrite(pump_1, LOW);
        }
        delay(water_delay);
        setMotors(closed_right, open_middle, open_left);
        arm_ct = 0;
      }
      else
      {
        setMotors(closed_right, open_middle, open_left);
      }
    }
  }
  else if ((!full_circle) && (CCW)) //always start down middle trajectory, right always blocked
  {
    if ((read_middle < IR_thresh) && ((prev_loc == atWater)||(prev_loc == atLeft)) && (next_loc = atMiddle)) //passes middle IR coming from water port
    {
      arm_ct = arm_ct + 1;
      setMotors(closed_right, block_right, open_left);
      prev_loc = atMiddle;
      next_loc = atLeft;
    }
    if ((read_left < IR_thresh) && (prev_loc == atMiddle) && (next_loc = atLeft)) //passes left IR coming from middle
    {
      setMotors(closed_right, block_left, open_left); //block from going back
      prev_loc = atLeft;
      arm_ct = arm_ct + 1;
      if (round(arm_ct / 2) == num_laps)
      {
        next_loc = atWater;
      }
      else if (round(arm_ct / 2) < num_laps)
      {
        next_loc = atMiddle;
      }
    }
    if ((read_water < IR_thresh) && (prev_loc == atLeft) && (next_loc == atWater))
    {
      prev_loc == atWater;
      next_loc = atMiddle;
      if (round(arm_ct / 2) == num_laps)
      {
        setMotors(closed_right, block_right, closed_left);
        pump_ct = millis();
        while ((millis() - pump_ct) < PUMP_OPEN_TIME)
        {
          digitalWrite(pump_1, HIGH);
        }
        if ((millis() - pump_ct) >= PUMP_OPEN_TIME)
        {
          digitalWrite(pump_1, LOW);
        }
        //no water delay here
        arm_ct = 0;
      } // no else here because there isn't a door to block them from continuing
    }
  }
  else if ((!full_circle) && (!CCW))
  {
    if ((read_middle < IR_thresh) && ((prev_loc == atWater)||(prev_loc == atRight)) && (next_loc == atMiddle)) //passes middle IR coming from water port
    {
      setMotors(open_right, block_left, closed_left);
      prev_loc = atMiddle;
      next_loc = atRight;
      arm_ct = arm_ct+1;
    }
    if ((read_right < IR_thresh) && (prev_loc == atMiddle) && (next_loc == atRight)) //passes right IR coming from left arm
    {
      setMotors(open_right, block_right, closed_left);
      prev_loc = atRight;
      arm_ct = arm_ct+1;
      if (round(arm_ct / 2) == num_laps)
      {
        next_loc = atWater;
      }
      else if (round(arm_ct / 2) < num_laps)
      {
        next_loc = atMiddle;
      }
    }
    if ((read_water < IR_thresh) & (prev_loc == atRight) && (next_loc == atWater))
    {
      prev_loc == atWater;
      next_loc == atMiddle;
      if (round(arm_ct / 2) == num_laps)
      {
        setMotors(closed_right, block_left, closed_left);
        pump_ct = millis();
        while ((millis() - pump_ct) < PUMP_OPEN_TIME)
        {
          digitalWrite(pump_1, HIGH);
        }
        if ((millis() - pump_ct) >= PUMP_OPEN_TIME)
        {
          digitalWrite(pump_1, LOW);
        }
        //no water delay here
        arm_ct = 0;
      }
    }
  }
  //sensorRead = analogRead(IRsensor);
  // //Serial.println(sensorRead);
  // put your main code here, to run repeatedly:
  //servo1_middle.write(open_middle);
  //delay(5000);
  //servo1_middle.write(closed_middle_blockR);
  //delay(5000);
  //servo1_middle.write(closed_middle_blockL);
  //delay(5000);
  //digitalWrite(pump_1,HIGH);
  //delay(5000);
  //digitalWrite(pump_1,LOW);
  //delay(5000);
}
