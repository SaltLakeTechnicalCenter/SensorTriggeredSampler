//=== define pins ===//
#define CLOSE_PIN 7
#define OPEN_PIN  8
#define PID       A0
#define LED       13
#define BUZZER    5

#define ON        LOW
#define OFF       HIGH
#define LED_OFF   digitalWrite(LED, LOW)
#define LED_ON    digitalWrite(LED, HIGH)
#define BUZZ_OFF  digitalWrite(BUZZER, LOW)
#define BUZZ_ON   digitalWrite(BUZZER, HIGH)

//=== define variables ===//
bool isArmed=false, isScroll=true, isLog=false;  // boolean variables used to toggle things on & off
unsigned int pid, triggerValue=172;  // PID values in A/D counts
unsigned long triggerTime=0, triggerWindow=500, captureTime=0, captureWindow=5000, logTime=0, logWindow=1000, sampleTime=0, sampleWindow=100, valveTime=0, valveWindow=10; // timing parameters in milliseconds
unsigned long pidSum=0, pidCount=0, logSum=0, logCount=0; // parameters used to average numeric output
char BluetoothData; // the data received from bluetooth serial link


void setup() {
  pinMode(CLOSE_PIN, OUTPUT); // set the solenoid pin as output
  pinMode(OPEN_PIN, OUTPUT);  // set the solenoid pin as output
  closeValve();                    // keep the solenoid valve closed
  pinMode(PID, INPUT);        // set the PID pin as input
  pinMode(LED, OUTPUT);       // set the LED pin as output
  LED_OFF;
  pinMode(BUZZER, OUTPUT);    // set the BUZZER pin as output
  BUZZ_OFF;
  analogReference(EXTERNAL);    // use a 2.5 volt regulator for reference voltage
  Serial.begin(9600);           // initiate serial for bluetooth communication
}

void loop() {
  pid=analogRead(PID);
  if (pid>triggerValue) LED_ON; else LED_OFF; // turn on LED?
  pidSum = pidSum + pid;
  pidCount = pidCount + 1;
  if (millis()-sampleTime >= sampleWindow) {
    pid = pidSum/pidCount;
    if (isArmed) testPID(); // if the system is armed test the PID value against the triggerValued.
    if (captureTime) checkValve(); // if the solenoid valve is open check to see if it is time to close it.
    if (isScroll) Serial.print("*G"+String(pid)+"*"); // publish PID value to graph if scrolling.
    if (logWindow!=0) logPID(); // write PID values.
//    if (Serial.available()) getSerial(); // process any info coming from the bluetooth serial link.
    pidSum = 0;
    pidCount = 0;
    sampleTime = millis();
  }
  if (millis()-valveTime >= valveWindow) valve();
  if (Serial.available()) getSerial(); // process any info coming from the bluetooth serial link.
}

void testPID() {
  if (pid>triggerValue)  // threshold exceeded?
  {
    if (triggerTime == 0) triggerTime = millis();  // is this the initial trigger?
    if (millis()-triggerTime >= triggerWindow)  // has the trigger lasted for the required duration?
    {
      captureTime = millis();
      openValve();
      disarm();
    }
      
  }
  else  // trigger not maintained for duration of window.
  {
    triggerTime = 0;
  }
}

void checkValve() {
  if (millis()-captureTime >= captureWindow)  // is it time to close the solenoid valve?
  {
    closeValve();
    captureTime = 0;
    triggerTime = 0;
  }
}

void logPID() {
  if (millis()-logTime >= logWindow)  // is it time to report PID value?
  {
    if(isLog) Serial.print("\n"+String(millis())+","+String(logSum/logCount)+","+String(captureTime));
    else Serial.print("*D"+String(logSum/logCount)+"*");
    logSum=0;
    logCount=0;
    logTime=millis();
  }
  logSum=logSum+pid;
  logCount++;
}

void getSerial() {
 BluetoothData=Serial.read(); //Get next character from bluetooth
  if(BluetoothData=='S') isScroll=true;
  if(BluetoothData=='s') isScroll=false;
  if(BluetoothData=='L') {isLog=true; isScroll=false;}
  if(BluetoothData=='l') isLog=false;
  if(BluetoothData=='A') {
    if(isArmed) disarm();
    else arm();
  }
  if(BluetoothData=='D') logWindow=Serial.parseInt();
  if(BluetoothData=='T') triggerValue=Serial.parseInt();
  if(BluetoothData=='W') triggerWindow=Serial.parseInt();
  if(BluetoothData=='C') captureWindow=Serial.parseInt();
  if(BluetoothData=='I') sampleWindow=Serial.parseInt();
  if(BluetoothData=='B') BUZZ_ON;
  if(BluetoothData=='b') BUZZ_OFF;
  if(BluetoothData=='O') openValve();
  if(BluetoothData=='o') closeValve();
}

void arm() {
  isArmed = true;
  if(!isLog) Serial.print("*AR0G255B0*");
}

void disarm() {
  isArmed = false;
  if(!isLog) Serial.print("*AR0G0B0*");  
}

void openValve() {
  BUZZ_ON;
  digitalWrite(OPEN_PIN, HIGH);
  valveTime = millis();
}

void closeValve() {
  BUZZ_OFF;
  digitalWrite(CLOSE_PIN, HIGH);
  valveTime = millis();
}

void valve() {
  digitalWrite(OPEN_PIN, LOW);
  digitalWrite(CLOSE_PIN, LOW);
}

