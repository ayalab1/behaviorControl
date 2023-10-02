// code to manually sync camera with led blinks to intan ephys
// @Praveen Paudel, 2023

#define PIN_LED 2
#define TTL_OUT_PIN 3
#define PULSE_DURATION 1000 // 1 second
#define MIN_DELAY 50 // 50 milliseconds
#define MAX_DELAY 120000 // 2 minutes

void setup() {
    pinMode(PIN_LED, OUTPUT);
    pinMode(TTL_OUT_PIN, OUTPUT);
    randomSeed(analogRead(0));
    Serial.begin(9600);  
}

void loop() {
    unsigned long delayDuration = random(MIN_DELAY, MAX_DELAY);  
    Serial.print("Pulse duration in seconds: "); 
    Serial.println((float)PULSE_DURATION/1000); // Printing pulse duration 
    Serial.print("Delay duration in seconds: "); 
    Serial.println((float)delayDuration/1000); 

    digitalWrite(PIN_LED, HIGH);
    digitalWrite(TTL_OUT_PIN, HIGH);
    delay(PULSE_DURATION);

    digitalWrite(PIN_LED, LOW);
    digitalWrite(TTL_OUT_PIN, LOW);
    delay(delayDuration);
}
