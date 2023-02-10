/* Send a TTL pulse on any of the 1-12 trigger-in channel of the OSCLite 1 for
  / controlling the uLED, according to some specification */
// settings
int delayBetweenPulsesFrom = 4000; // in ms, first value on range of for random pulses delay, for example 20
int delayBetweenPulsesTo = 6000; // in ms, last value on range of for random pulses delay, for example 40
int pulseDuration = 600; // in ms, for example 20
boolean continousStimulation = true;// stimulation no dependent of the behavoiur arduino

// Pins
const int s1l1Pin = 34;
const int s1l2Pin = 22;
const int s1l3Pin = 36;

const int s2l1Pin = 24;
const int s2l2Pin = 38;
const int s2l3Pin = 26;

const int s3l1Pin = 40;
const int s3l2Pin = 28;
const int s3l3Pin = 42;

const int s4l1Pin = 30;
const int s4l2Pin = 44;
const int s4l3Pin = 32; 

const int BlockPin = 53; // From the control behaviour arduino

const int useOrder[] = {2, 3, 1};
const int stimShank = 4;

// state variables
int stimBlock = 0; // 0 is no stimulating
int uled_num = 0;

int delayBetweenPulses;

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
    for (int i = 0; i < 3; i++) {
      if (useOrder[i] == 1) {
        // ONLY ONE SHANK SHOULD BE ARMED IN THE NEUROLIGHT SOFTWARE AT ONE TIME - otherwise, manually change code to comment out nonactive shanks
        if (stimShank == 1) {
          digitalWrite(s1l1Pin, HIGH); delay(pulseDuration); digitalWrite(s1l1Pin, LOW);
        } else if (stimShank == 2) {
          digitalWrite(s2l1Pin, HIGH); delay(pulseDuration); digitalWrite(s2l1Pin, LOW);
        } else if (stimShank == 3) {
          digitalWrite(s3l1Pin, HIGH); delay(pulseDuration); digitalWrite(s3l1Pin, LOW);
        } else if (stimShank == 4) {
          digitalWrite(s4l1Pin, HIGH); delay(pulseDuration); digitalWrite(s4l1Pin, LOW);
        }
      } else if (useOrder[i] == 2) {
        if (stimShank == 1) {
          digitalWrite(s1l2Pin, HIGH); delay(pulseDuration); digitalWrite(s1l2Pin, LOW);
        } else if (stimShank == 2) {
          digitalWrite(s2l2Pin, HIGH); delay(pulseDuration); digitalWrite(s2l2Pin, LOW);
        } else if (stimShank == 3) {
          digitalWrite(s3l2Pin, HIGH); delay(pulseDuration); digitalWrite(s3l2Pin, LOW);
        } else if (stimShank == 4) {
          digitalWrite(s4l2Pin, HIGH); delay(pulseDuration); digitalWrite(s4l2Pin, LOW);
        }
      } else if (useOrder[i] == 3) {
        if (stimShank == 1) {
          digitalWrite(s1l3Pin, HIGH); delay(pulseDuration); digitalWrite(s1l3Pin, LOW);
        } else if (stimShank == 2) {
          digitalWrite(s2l3Pin, HIGH); delay(pulseDuration); digitalWrite(s2l3Pin, LOW);
        } else if (stimShank == 3) {
          digitalWrite(s3l3Pin, HIGH); delay(pulseDuration); digitalWrite(s3l3Pin, LOW);
        } else if (stimShank == 4) {
          digitalWrite(s4l3Pin, HIGH); delay(pulseDuration); digitalWrite(s4l3Pin, LOW);
        }
      }
    }
    delayBetweenPulses = random(delayBetweenPulsesFrom, delayBetweenPulsesTo); // random delay for pulses as previously defined
    delay(delayBetweenPulses);
  } else {
    digitalWrite(LED_BUILTIN, LOW);   // turn the LED off
  }
}
