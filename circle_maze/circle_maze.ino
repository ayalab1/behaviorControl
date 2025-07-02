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

int open_right = 90;
int closed_right = 0;
int open_middle = 150;
int block_right = 115; //middle door
int block_left = 65; //middle door
int open_left = 50;
int closed_left = 15;

// Set up water pump
#define pump_1 36
unsigned long PUMP_OPEN_TIME = 100;
unsigned long pump_ct;

// Set up IR sensors
int IR_water = A0;
int read_water = 0;
int IR_right = A1;
int read_right = 0;
int IR_middle = A2;
int read_middle = 0;
int IR_left = A3;
int read_left = 0;
int IR_thresh = 500; //this should work for all. >500 is empty, <500 detects mouse

// Set up behavior
int lap_ct = 0;
int prev_loc = 0; //0 = water port, 1 = right IR, 2 = middle IR, 3 = left IR
int water_delay = 5000;

void setup() {
  // put your setup code here, to run once:
  servo1_right.attach(servo_right);
  servo1_middle.attach(servo_middle);
  servo1_left.attach(servo_left);

  //mouse is starting in home port. Depending on trial setup, need to start in a different configuration
  // full circle, CCW
  if ((full_circle) && (CCW))
  {
    servo1_right.write(open_right);
    servo1_middle.write(open_middle);
    servo1_left.write(closed_left);
  }
  // full circle, CW
  else if ((full_circle) && (!CCW))
  {
    servo1_right.write(closed_right);
    servo1_middle.write(open_middle);
    servo1_left.write(open_left);
  }
  // half circle, CCW
  else if ((!full_circle) && (CCW))
  {
    servo1_right.write(closed_right); //permanently closed for this trial type
    servo1_middle.write(block_right);
    servo1_left.write(closed_left);
  }
  // half circle, CW
  else if ((!full_circle) && (!CCW))
  {
    servo1_right.write(closed_right);
    servo1_middle.write(block_left);
    servo1_left.write(closed_left);
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
    if ((read_right < IR_thresh) && ((prev_loc == 0)||(prev_loc == 3))) //passes right IR coming from water port OR left if the mouse doesn't stop at water port
    {
      Serial.print("Right arm");
      servo1_right.write(closed_right);
      servo1_middle.write(open_middle);
      servo1_left.write(open_left);
      prev_loc = 1;
    }
    if ((read_left < IR_thresh) && (prev_loc == 1)) //passes left IR coming from right arm
    {
      Serial.print("Left arm");
      servo1_middle.write(block_left); //block from going back
      prev_loc = 3;
    }
    if ((read_water < IR_thresh) && (prev_loc == 3))
    {
      Serial.print("Water port");
      prev_loc = 0;
      lap_ct = lap_ct + 1;
      if (lap_ct == num_laps)
      {
        Serial.print("Dispensing");
        servo1_left.write(closed_left);
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
        servo1_right.write(open_right);
        lap_ct = 0;
      }
      else
      {
        servo1_right.write(open_right);
      }
    }
  }
  else if ((full_circle) && (!CCW))
  {
    if ((read_left < IR_thresh) && ((prev_loc == 0)||(prev_loc == 1))) //passes left IR coming from water port
    {
      Serial.print("Left arm");
      servo1_left.write(closed_left);
      servo1_middle.write(open_middle);
      servo1_right.write(open_right);
      prev_loc = 3;
    }
    if ((read_right < IR_thresh) && (prev_loc == 3)) //passes right IR coming from left arm
    {
      Serial.print("Right arm");
      servo1_middle.write(block_right); //block from going back
      prev_loc = 1;
    }
    if ((read_water < IR_thresh) & (prev_loc == 1))
    {
      Serial.print("Water port");
      prev_loc = 0;
      lap_ct = lap_ct + 1;
      if (lap_ct == num_laps)
      {
        Serial.print("Dispensing");
        servo1_right.write(closed_right);
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
        servo1_left.write(open_left);
        lap_ct = 0;
      }
      else
      {
        servo1_left.write(open_left);
      }
    }
  }
  else if ((!full_circle) && (CCW)) //always start down middle trajectory, right always blocked
  {
    if ((read_middle < IR_thresh) && ((prev_loc == 0)||(prev_loc == 3))) //passes middle IR coming from water port
    {
      servo1_right.write(closed_right);
      servo1_middle.write(block_right);
      servo1_left.write(open_left);
      prev_loc = 2;
    }
    if ((read_left < IR_thresh) && (prev_loc == 2)) //passes left IR coming from middle
    {
      servo1_middle.write(block_left); //block from going back
      prev_loc = 3;
    }
    if ((read_water < IR_thresh) & (prev_loc == 3))
    {
      lap_ct = lap_ct + 1;
      prev_loc == 0;
      if (lap_ct == num_laps)
      {
        servo1_left.write(closed_left);
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
        lap_ct = 0;
      } // no else here because there isn't a door to block them from continuing
    }
  }
  else if ((!full_circle) && (!CCW))
  {
    if ((read_middle < IR_thresh) && ((prev_loc == 0)||(prev_loc == 1))) //passes middle IR coming from water port
    {
      servo1_left.write(closed_left);
      servo1_middle.write(block_left);
      servo1_right.write(open_right);
      prev_loc = 2;
    }
    if ((read_right < IR_thresh) && (prev_loc == 2)) //passes right IR coming from left arm
    {
      servo1_middle.write(block_right); //block from going back
      prev_loc = 1;
    }
    if ((read_water < IR_thresh) & (prev_loc == 1))
    {
      lap_ct = lap_ct + 1;
      prev_loc == 0;
      if (lap_ct == num_laps)
      {
        servo1_right.write(closed_right);
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
        lap_ct = 0;
      }
    }
  }
  //sensorRead = analogRead(IRsensor);
  // Serial.println(sensorRead);
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
