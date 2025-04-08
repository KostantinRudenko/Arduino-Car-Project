#include <QTRSensors.h>
#include <SoftwareSerial.h>

#define LeftMotorMin 0 // -30  //задання мінімальної та максимальної швидкості моторів
#define RightMotorMin 0 // -30
#define LeftMotorMax 180 // 180
#define RightMotorMax 180 // 180
int BaseSpeed = 50;

#define OUT_STBY 8 // Pins
#define OUT_B_PWM 5 
#define OUT_A_PWM 11 
#define OUT_A_IN2 10 
#define OUT_A_IN1 9 
#define OUT_B_IN1 7 
#define OUT_B_IN2 6 

#define BTN 12

#define BLUE 3
#define RED 4
#define GREEN 2

int P;
int I;
int D;

bool IS_MOVING = false;

float KP, KD, KI;

#define LINE_MINIMUM_VALUE 700

SoftwareSerial espSerial(0, 1); // RX, TX

QTRSensors qtr;

void setup() {
  
  KP = 0.06; //0.027    //  0.027 speed 60
  KD = 0.2;//.017//0.017    // 0.020 speed 60
  KI = 0;

  espSerial.begin(9600);

  pinMode(13, OUTPUT);

  /// TB PINS ///
  pinMode(OUT_STBY,OUTPUT);
  pinMode(OUT_A_PWM,OUTPUT);
  pinMode(OUT_A_IN1,OUTPUT);
  pinMode(OUT_A_IN2,OUTPUT);
  pinMode(OUT_B_PWM,OUTPUT);
  pinMode(OUT_B_IN1,OUTPUT);
  pinMode(OUT_B_IN2,OUTPUT);

  pinMode(GREEN, OUTPUT);
  pinMode(RED, OUTPUT);
  pinMode(BLUE, OUTPUT);

  pinMode(BTN, INPUT_PULLUP);

  /// SENSORS ///
  qtr.setTypeAnalog();
  qtr.setSensorPins((const uint8_t[]){A7, A6, A5, A4, A3, A2, A1, A0}, 8);

  digitalWrite(13, HIGH);

  digitalWrite(OUT_STBY, HIGH); //STBY
  digitalWrite(OUT_A_IN1, LOW); //AIN1
  digitalWrite(OUT_A_IN2, HIGH); //AIN2
  digitalWrite(OUT_B_IN1, HIGH); //BIN1
  digitalWrite(OUT_B_IN2, LOW); //BIN2

  digitalWrite(RED, 1);
  Calibration();

  Speed(0,0);
}

int lastError = 0;
unsigned int sensorValues[8];

void loop() {
  int position = qtr.readLineBlack(sensorValues);
  int error = position - 3500;
  P = error;
  I = error + I;
  D = error - lastError;
  lastError = error;

  if (espSerial.available()){
    String msg = espSerial.readStringUntil("\n");
    parseData(msg);
  }

  int motorSpeed = P * KP + I * KI + D * KD;          
  int leftMotorSpeed = BaseSpeed - motorSpeed;
  int rightMotorSpeed = BaseSpeed + motorSpeed;

  CheckBTN();
  CheckIsMoving(leftMotorSpeed, rightMotorSpeed);
}

// ---------- ARDUINO CAR FUNCTIONS ----------

void CheckBTN(){
  if (digitalRead(BTN) == 0) {
    delay(50);

    IS_MOVING = !IS_MOVING;
    delay(500);
  }
}

void CheckIsMoving(int LMotorS, int RMotorS){
  if (IS_MOVING && isOnLine()){
    Speed(LMotorS, RMotorS);
  }
  else{
    IS_MOVING = false;
    Speed(0, 0);
  }
}

void Calibration() { //Функція калібровки через бібліотеку
  digitalWrite(GREEN, 0);
  digitalWrite(RED, 1);
  for (int i = 0; i < 300; i++){
    qtr.calibrate();
    delay(20);
  }
  digitalWrite(RED, 0);
  digitalWrite(GREEN, 1);
}

void Speed(int leftmotorspeed, int rightmotorspeed) { //Функція придання швидкості на мотори
  if (leftmotorspeed > LeftMotorMax ) leftmotorspeed = LeftMotorMax;
  if (rightmotorspeed > RightMotorMax ) rightmotorspeed = RightMotorMax;
  if (leftmotorspeed < LeftMotorMin) leftmotorspeed = LeftMotorMin;
  if (rightmotorspeed < RightMotorMin) rightmotorspeed = RightMotorMin;

  analogWrite(OUT_A_PWM, leftmotorspeed);
  analogWrite(OUT_B_PWM, rightmotorspeed);
}

bool isOnLine(){
  int max_sensors_value = 0;
  for (int i = 0; i < 8; i++){
    if (max_sensors_value < sensorValues[i]){
      max_sensors_value = sensorValues[i];
    }
  }
  return max_sensors_value > 700;
}

// ---------- GETTING DATA FROM WEBSITE ----------

String pidstr("PID");
String STOP("0");

void parseData(String data){
  if (data.startsWith("PID ")){
  String values = data.substring(4);
  values.trim();
  
  int index1 = values.indexOf(' ');
  int index2 = values.indexOf(' ', index1 + 1);
  
  if (index1 != -1 && index2 != -1) {
      KP = values.substring(0, index1).toFloat();
      KI = values.substring(index1 + 1, index2).toFloat();
      KD = values.substring(index2 + 1).toFloat();
  }
  else if (data == STOP){
      IS_MOVING = false;
  }
  // if (data.length() > 0){
  //   if (data.startsWith(pidstr)){
  //     KP = data.substring(firstSpace, secondSpace).toFloat();
  //     KI = data.substring(secondSpace+1, thirdSpace).toFloat();
  //     KD = data.substring(thirdSpace+1).toFloat();
  //   }
  }
}