#include <Arduino.h>
/***
 This is the device driver for AT1 of DryBot v2.3a, may 10 2024.
 */
#include <Wire.h>
#define TF_OFF
#define ACC_OFF

#ifdef ACC_ON
  #include "SparkFun_LIS2DH12.h"
  SPARKFUN_LIS2DH12 accel;
#endif
#ifdef TF_ON
  #include <VL53L0X.h>
  VL53L0X sensor; //0x29
#endif
#define FOR(I,N) for(int i=I;i<N;i++)
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

int ENC_MA_A = PC0;
int ENC_MA_B = PC1;
int ENC_MB_A = PC2;
int ENC_MB_B = PC3;
int ENC_MC_A = PA6;
int ENC_MC_B = PA7;
int MA1 = PA4;
int MA2 = PA5;
int RGB_G = PB4;
int RGB_B = PB5;
//pwm pins: pb3,4,5, pa3,4,5, pc0/1 : please choose correct settings.
int RGB_R = PA3; //dryBot LEDR 1 = off
int WLED1 = PA2; //dryBot LEDW 0 = off
int RLED1 = PA1;
//#define MY_ADDRESS 0x17 // address for this slave device
//#define SLAVE_ADDRESS 0x12

#ifdef TF_ON
  int head=0;
#endif
#ifdef ACC_ON
  float z_acc=0.0;
#endif

void show_RGB(long);
void drive_motor(int,int,int,int);
void signalling(int);

void setup() {
  pinMode(RLED1,OUTPUT);
  pinMode(WLED1,OUTPUT);
  pinMode(ENC_MA_A,INPUT);
  pinMode(ENC_MA_B,INPUT);
  pinMode(ENC_MB_A,INPUT);
  pinMode(ENC_MB_B,INPUT);
  pinMode(ENC_MC_A,INPUT);
  pinMode(ENC_MC_B,INPUT);
  
  pinMode(MA1, OUTPUT); 
  pinMode(MA2, OUTPUT);

  pinMode(RGB_R, OUTPUT);
  pinMode(RGB_G, OUTPUT);
  pinMode(RGB_B, OUTPUT);

// motor test
//  digitalWrite(MA1,0);
//  digitalWrite(MA2,0);
//  delay(1000);
//  digitalWrite(MA1,1);
//  digitalWrite(MA2,1);

  show_RGB(0x00FFFF);
  //analogWrite(MA1, 0);
  //analogWrite(MA2, 0);
  //Wire.begin(MY_ADDRESS); // join i2c bus as slave
  Wire.begin(); // join i2c bus as master
  //Wire.onReceive(receiveData); // callback for receiving data
  //Wire.onRequest(sendData); // callback for sending data
  digitalWrite(RLED1, 0); //off
  digitalWrite(WLED1, 0); // 1 on for light
  
  #ifdef TF_ON
  sensor.setTimeout(500);
  if (!sensor.init()) {
    FOR(3){
    signalling(30);
    delay(100);
    }
  }
  sensor.startContinuous();
  #endif
  
  #ifdef ACC_ON
  if (!accel.begin()) {
    signalling(30);
    delay(100);
  }
  #endif
}

// arduino long type has 4 bytes, 0xFFFFFFFF, signed. ranged -2,147,483,648 to 2,147483,647
void loop() {  
    //digitalWrite(RGB_R,digitalRead(ENC_MA_A)); // ma_a was working but seems pin has died.?
    digitalWrite(RGB_B,digitalRead(ENC_MB_B)); 
    digitalWrite(RGB_R,digitalRead(ENC_MB_A)); 
    show_RGB(0xFFFDFF);
  
  #ifdef TF_ON
  head=sensor.readRangeContinuousMillimeters();
  if (sensor.timeoutOccurred()) FOR(3)signalling(50);
  if(head > 100){
    digitalWrite(WLED1,1); //turn on
  }else{
    digitalWrite(WLED1,0);
  }
  #endif
  #ifdef ACC_ON
  z_acc = accel.getZ();
  if (z_acc < 0){
    digitalWrite(WLED1, 1); //on
  }else{
    digitalWrite(WLED1, 0);//Wled off
  }
  #endif

}

/*
 * power = mimic lego's design: +/-255
 * example drive_motor(MA, 255)
 * drive_motor(MB, -50)
 */
void drive_motor(int p1, int p2, int dir, int speed){
  if(dir==1){
    digitalWrite(p2,0);
    analogWrite(p1,speed);
  }else if (dir==-1){
    digitalWrite(p1,0);
    analogWrite(p2,speed);    
  }else if(dir==0) {
    digitalWrite(p1, 0);
    digitalWrite(p2, 0);
  }else if(dir==10){
    digitalWrite(p1, 1);
    digitalWrite(p2, 0);
  }else if(dir==11){
    digitalWrite(p1, 0);
    digitalWrite(p2, 1);
  }
}

void signalling(int delaytime) {
  // Blink the LED as a signal
  for (int i = 0; i < 3; i++) {
    digitalWrite(RLED1, HIGH);
    delay(delaytime);
    digitalWrite(RLED1, LOW);
    delay(delaytime);
  }
}

//long RGB = 0x000000; //this will be full brightness on all three leds
void show_RGB(long val){
  analogWrite(RGB_R,val>>16 & 0xFF);
  analogWrite(RGB_G,val>>8  & 0xFF);
  analogWrite(RGB_B,val     & 0xFF);
}
