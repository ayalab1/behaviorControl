char incomingByte;
#define pump_1 36 // left

void setup() {
  // put your setup code here, to run once:
 pinMode(pump_1, OUTPUT);
 Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
    if (Serial.available() > 0) {
    // read the incoming byte:
    incomingByte = Serial.read();
    //open the pump
    if (incomingByte=='h')
    {
      digitalWrite(pump_1, HIGH);
       delay(1000);
      }
      //close the pump
     if (incomingByte=='l')
    {
       digitalWrite(pump_1, LOW);
       delay(1000);
      }
    }

}
