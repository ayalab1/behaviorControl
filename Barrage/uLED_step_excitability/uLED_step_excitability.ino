/* Send a TTL pulse on any of the 1-12 trigger-in channel of the OSCLite 1 for
  / controlling the uLED, according to some specification */
// settings
int delayBetweenPulsesFrom = 50; // in ms, first value on range of for random pulses delay, for example 20
int delayBetweenPulsesTo = 150; // in ms, last value on range of for random pulses delay, for example 40
int pulseDuration = 70; // in ms, for example 20
boolean continousStimulation = true;// stimulation no dependent of the behavoiur arduino
boolean stimulateShank1 = true;
boolean stimulateShank2 = true;
boolean stimulateShank3 = true;
boolean stimulateShank4 = true;

// Pins
const int s1l1Pin = 34;
const int s1l2Pin = 32;
const int s1l3Pin = 36;

const int s2l1Pin = 30;
const int s2l2Pin = 38;
const int s2l3Pin = 28;

const int s3l1Pin = 40;
const int s3l2Pin = 26;
const int s3l3Pin = 42;

const int s4l1Pin = 24;
const int s4l2Pin = 44;
const int s4l3Pin = 22;

const int BlockPin = 53; // From the control behaviour arduino

const int itMax = 100;
const int numLEDs = 12;

// state variables
int stimBlock = 0; // 0 is no stimulating
int uled_channel = 0;
int delayBetweenPulses;

// order tracker
int LEDorder[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
int iteration = 0;
int tempLEDorder[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int tc = 0; //counter for tempLEDorder
int progress = 0;
int maxLED = 11;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);

  pinMode(s1l1Pin, OUTPUT);
  pinMode(s1l2Pin, OUTPUT);
  pinMode(s1l3Pin, OUTPUT);
  pinMode(s2l1Pin, OUTPUT);
  pinMode(s2l2Pin, OUTPUT);
  pinMode(s2l3Pin, OUTPUT);
  pinMode(s3l1Pin, OUTPUT);
  pinMode(s3l2Pin, OUTPUT);
  pinMode(s3l3Pin, OUTPUT);
  pinMode(s4l1Pin, OUTPUT);
  pinMode(s4l2Pin, OUTPUT);
  pinMode(s4l3Pin, OUTPUT);

  pinMode(BlockPin, INPUT);

  delayBetweenPulsesTo = delayBetweenPulsesTo + 1;
  randomSeed(analogRead(A0));
}

void loop() {
  if  (continousStimulation == true) {
    digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on
    uled_channel = random(0, maxLED); // random number from 1 to 12
    if (LEDorder[uled_channel] == 1 & stimulateShank1 == true) {
      digitalWrite(s1l1Pin, HIGH); delay(pulseDuration); digitalWrite(s1l1Pin, LOW);
    } else if (LEDorder[uled_channel] == 2 & stimulateShank1 == true) {
      digitalWrite(s1l2Pin, HIGH); delay(pulseDuration); digitalWrite(s1l2Pin, LOW);
    } else if (LEDorder[uled_channel] == 3 & stimulateShank1 == true) {
      digitalWrite(s1l3Pin, HIGH); delay(pulseDuration); digitalWrite(s1l3Pin, LOW);
    } else if (LEDorder[uled_channel] == 4 & stimulateShank2 == true) {
      digitalWrite(s2l1Pin, HIGH); delay(pulseDuration); digitalWrite(s2l1Pin, LOW);
    } else if (LEDorder[uled_channel] == 5 & stimulateShank2 == true) {
      digitalWrite(s2l2Pin, HIGH); delay(pulseDuration); digitalWrite(s2l2Pin, LOW);
    } else if (LEDorder[uled_channel] == 6 & stimulateShank2 == true) {
      digitalWrite(s2l3Pin, HIGH); delay(pulseDuration); digitalWrite(s2l3Pin, LOW);
    } else if (LEDorder[uled_channel] == 7 & stimulateShank3 == true) {
      digitalWrite(s3l1Pin, HIGH); delay(pulseDuration); digitalWrite(s3l1Pin, LOW);
    } else if (LEDorder[uled_channel] == 8 & stimulateShank3 == true) {
      digitalWrite(s3l2Pin, HIGH); delay(pulseDuration); digitalWrite(s3l2Pin, LOW);
    } else if (LEDorder[uled_channel] == 9 & stimulateShank3 == true) {
      digitalWrite(s3l3Pin, HIGH); delay(pulseDuration); digitalWrite(s3l3Pin, LOW);
    } else if (LEDorder[uled_channel] == 10 & stimulateShank4 == true) {
      digitalWrite(s4l1Pin, HIGH); delay(pulseDuration); digitalWrite(s4l1Pin, LOW);
    } else if (LEDorder[uled_channel] == 11 & stimulateShank4 == true) {
      digitalWrite(s4l2Pin, HIGH); delay(pulseDuration); digitalWrite(s4l2Pin, LOW);
    } else if (LEDorder[uled_channel] == 12 & stimulateShank4 == true) {
      digitalWrite(s4l3Pin, HIGH); delay(pulseDuration); digitalWrite(s4l3Pin, LOW);
    }
    for (int i = 0 ; i < numLEDs; i++) { //reset tempLEDorder to zeros array
      tempLEDorder[i] = 0;
    }
    tc = 0;
    for (int i = 0 ; i < numLEDs; i++) { //fill with valid LED choices from the start (end should be zeros)
      if (i != uled_channel) {
        tempLEDorder[tc] = LEDorder[i];
        tc = tc + 1;
      }
    }
    progress = 0;
    for (int i = 0 ; i < numLEDs; i++) { //make tempLEDorder into LEDorder
      LEDorder[i] = tempLEDorder[i];
      if (LEDorder[i] != 0) { //find how many valid LEDs we have left
        progress = progress + 1;
      }
    }

    if (progress == 0) { //if we've done all the uLEDs, reset
      iteration = iteration + 1;
      for (int i = 0; i < numLEDs; i++) {
        LEDorder[i] = i + 1;
      }
      maxLED = 11;
    } else { //set the new max LED index since we can't change array size
      maxLED = progress - 1;
    }
    delayBetweenPulses = random(delayBetweenPulsesFrom, delayBetweenPulsesTo); // random delay as set in setup
    delay(delayBetweenPulses);
    if (iteration >= itMax) {
      continousStimulation = false;
    }
  } else {
    digitalWrite(LED_BUILTIN, LOW);   // turn the LED off
  }
}
