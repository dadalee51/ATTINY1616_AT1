#include <Arduino.h>
/***
 * This is the motor driving Code. for IR and light sensor please use other file.
 * AT2
 */
#include <Wire.h>
#include <VL53L0X.h>

VL53L0X sensor;
#define PA0 17 //UPDI
#define PA1 14 //MOSI
#define PA2 15 //MISO
#define PA3 16 //CLK
#define PA4 0  //SS
#define PA5 1  
#define PA6 2  //DAC
#define PA7 3

#define PB0 9 //SCL
#define PB1 8 //SDA
#define PB2 7 //TXD
#define PB3 6 //RXD
#define PB4 5 
#define PB5 4 

#define PC0 10
#define PC1 11
#define PC2 12
#define PC3 13
/****
 * remember to add this line to boards.txt 
 * file located in linux: /home/qq/.arduino15/packages/megaTinyCore/hardware/megaavr/2.6.10
 * or goto preference and locate arduino15 folder in windows.
 * atxy6.bootloader.LOCKBIT=0xC5
 */
 //pwm pins: pb3,4,5, pa3,4,5, pc0/1 : please choose correct settings.
#define FOR(i,end) for(int i=0;i<end;i++)
int LEDW = PA2;
int LEDR = PA3;

int MA1 = PA4;
int MA2 = PA5;
int MB1 = PB4;
int MB2 = PB5;

int S1 = PA1;
int S2 = PA2;
int S3 = PA4;
int S4 = PA4;

int S1 = PA1;
int S2 = PA2;
int S3 = PA4;
int S4 = PA4;


//header function area:
void requestWork();
void turn_motor(int pin1, int pin2, int dir);
void test_motor_A();
void receiveWork(int16_t numBytes);
void signalling(int delaytime);


void setup() {
  pinMode(LED1, OUTPUT);
  //A5, A6, MA
  pinMode(MA1, OUTPUT); 
  pinMode(MA2, OUTPUT);
  //PA7 PB5, MB
  pinMode(MB1, OUTPUT); 
  pinMode(MB2, OUTPUT);
  //B4, C3, MC
  pinMode(MC1, OUTPUT); 
  pinMode(MC2, OUTPUT);
  //S13,S14,S15 input:
  pinMode(S13, INPUT);
  pinMode(S14, INPUT);
  pinMode(S15, INPUT);
  Wire.pins(PB1, PB0);
  Wire.onRequest(requestWork); 
  Wire.onReceive(receiveWork); 
  Wire.begin(0x16); 
  signalling(50);
    sensor.setTimeout(500);
  if (!sensor.init())
  {
    Serial.println("Failed to detect and initialize sensor!");
    signalling(1000);
  }
}

void loop() {
  digitalWrite(LED1, 1);
  delay(500);
//
//  FOR(i, 4)test_motor_A();
//  FOR(i, 3)test_motor_B();
//  FOR(i, 5)test_motor_C();
//  //delay(1000);
//  FOR(i, 3){
//    turn_motor(MA1, MA2, 1);
//    turn_motor(MB1, MB2, 1);
//    turn_motor(MC1, MC2, 1);
//    delay(500);
//    turn_motor(MA1, MA2, 0);
//    turn_motor(MB1, MB2, 0);
//    turn_motor(MC1, MC2, 0);
//    delay(250);
//    turn_motor(MA1, MA2, -1);
//    turn_motor(MB1, MB2, -1);
//    turn_motor(MC1, MC2, -1);
//    delay(500);
//    turn_motor(MA1, MA2, 0);
//    turn_motor(MB1, MB2, 0);
//    turn_motor(MC1, MC2, 0);
//    delay(250);
//  }
//  turn_motor(MA1, MA2, 0);
//  turn_motor(MB1, MB2, 0);
//  turn_motor(MC1, MC2, 0);
  digitalWrite(LED1, 0);
  delay(100);

}

void turn_motor(int pin1, int pin2, int dir){
  //dir = 0 no rotate, -1 anti, 1 clock
  if(dir==0){
    digitalWrite(pin1, 0);
    digitalWrite(pin2, 0);
  }else if(dir==1){
    digitalWrite(pin1, 1);
    digitalWrite(pin2, 0);
  }else if(dir==-1){
    digitalWrite(pin1, 0);
    digitalWrite(pin2, 1);
  }
}

void test_motor_A(){
  turn_motor(MA1, MA2, 1);
  delay(100);
  turn_motor(MA1, MA2, 0);
  delay(100);
  turn_motor(MA1, MA2, -1);
  delay(100);
  turn_motor(MA1, MA2, 0);
}

void test_motor_B(){
  turn_motor(MB1, MB2, 1);
  delay(100);
  turn_motor(MB1, MB2, 0);
  delay(100);
  turn_motor(MB1, MB2, -1);
  delay(100);
  turn_motor(MB1, MB2, 0);
}

void test_motor_C(){
  turn_motor(MC1, MC2, 1);
  delay(100);
  turn_motor(MC1, MC2, 0);
  delay(100);
  turn_motor(MC1, MC2, -1);
  delay(100);
  turn_motor(MC1, MC2, 0);
}

void requestWork() {      // the Wire API tells us how many bytes
  unsigned int s13 = analogRead(S13);
  Wire.write((uint8_t *)&s13, sizeof(s13)); 
}

//receive two bytes: 1 byte for operation, 1 byte for quantity.
void receiveWork(int16_t numBytes) {
  //for (uint8_t i = 0; i < numBytes; i++)
  int operation = Wire.read();
  int quan = Wire.read();
  switch(operation){
    case 1:
      FOR(i,quan)signalling(50);
      break;
    case 0:
    default:
      break;
  }
      
  
}

void signalling(int delaytime){
    FOR(i,10){
    digitalWrite(LED1, 1);//turn off AT1 led (right most)
    delay(delaytime);
    digitalWrite(LED1, 0);//turn off AT1 led (right most)
    delay(delaytime);
  }
}