#include <Arduino.h>
/*** AT1 - slave 0x12
 AT1 of DryBot v2.3a, may 10 2024.
 */
#include <Wire.h>
#define TF_OFF
#define ACC_OFF
#define CLR_OFF

#ifdef ACC_ON
  #include "SparkFun_LIS2DH12.h"
  SPARKFUN_LIS2DH12 accel;
#endif
#ifdef TF_ON
  #include <VL53L0X.h>
  VL53L0X sensor; //0x29
#endif
#ifdef CLR_ON
  #include "veml6040.h"
  VEML6040 RGBWSensor;
#endif

#define FOR(I,N) for(int I=0;I<N;I++)
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

//change PB3/4/5 to pwm pins
//TCA0.SINGLE.CTRLB = (TCA_SINGLE_WGMODE_SINGLESLOPE_gc | TCA_SINGLE_CMP0EN_bm);
//TCA0.SINGLE.CTRLB = (TCA_SINGLE_CMP0EN_bm | TCA_SINGLE_WGMODE_DSBOTTOM_gc);


#ifdef TF_ON
  int head=0;
#endif
#ifdef ACC_ON
  float z_acc=0.0;
#endif
#ifdef CLR_ON
  int red=0; int mred=0;
  int blue=0; int mblue=0; 
  int green=0; int mgreen=0;
#endif

//headers
void TCA9548A(uint8_t bus);
void show_RGB(long);
void drive_motor(int,int,int,int);
void signalling(int);
/*** Wire interface **********************************************/
#define MASTER_ADDRESS 0x17 
#define SLAVE_ADDRESS 0x12 
#define BUFFER_SIZE 20 
char receivedData[BUFFER_SIZE]; 
int dataLength = 0; 
int postflag = 0;
//master send
void receiveData(int numBytes) {
  dataLength = numBytes;
  Wire.readBytes(receivedData, numBytes);
  postflag = 1;//mark data ready
}
//master read
void sendData() {
  Wire.write(receivedData, dataLength);
}

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
  //analogWrite(MA1, 0);
  //analogWrite(MA2, 0);
  Wire.begin(SLAVE_ADDRESS); // join i2c bus as slave
  //Wire.begin(); // join i2c bus as master
  Wire.onReceive(receiveData); // callback for receiving data
  Wire.onRequest(sendData); // callback for sending data

  digitalWrite(RLED1, 1); // 1 off, 0 on
  digitalWrite(WLED1, 0); // 1 on , 0 off.
  show_RGB(0xFFFFFF,0); //RGB off

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

  #ifdef CLR_ON
  //switch to first clr sensr
  //TCA9548A(0);
  RGBWSensor.setConfiguration(VEML6040_IT_40MS + VEML6040_AF_AUTO + VEML6040_SD_ENABLE);
  //RGBWSensor.setConfiguration(VEML6040_IT_40MS + VEML6040_TRIG_ENABLE + VEML6040_AF_FORCE + VEML6040_SD_ENABLE);
  delay(500);
  FOR(i,5){
    if(!RGBWSensor.begin()) {
      signalling(30);
      delay(1000);
    }
  }
  //switch to first clr sensr
  // TCA9548A(1);
  // //RGBWSensor.setConfiguration(VEML6040_IT_40MS + VEML6040_AF_AUTO + VEML6040_SD_ENABLE);
  // RGBWSensor.setConfiguration(VEML6040_IT_40MS + VEML6040_TRIG_ENABLE + VEML6040_AF_FORCE + VEML6040_SD_ENABLE);
  // delay(500);
  // FOR(i,5){
  //   if(!RGBWSensor.begin()) {
  //     signalling(30);
  //     delay(1000);
  //   }
  // }
  #endif
  //TCA9548A(0);//back to zero
  //delay(500);
}
int dr = 0;
long data=0;
// arduino long type has 4 bytes, 0xFF FF FF FF, signed. ranged -2,147,483,648 to 2,147483,647
void loop() {  
    //digitalWrite(RGB_R,digitalRead(ENC_MA_A)); // ma_a was working but seems pin has died.?
    //digitalWrite(RGB_B,digitalRead(ENC_MB_B)); 
    //digitalWrite(RGB_R,digitalRead(ENC_MB_A)); 
    //digitalWrite(RGB_R,0);

    if(postflag == true){
      if(receivedData[0]==0x01 && receivedData[1]==0x00){
        //drive RGB
        data = (((long)receivedData[3]&0xff)<<16) |((receivedData[4]&0xff)<<8) | (receivedData[5]&0xff);
        //show_RGB(data, 2);
      }else if(receivedData[0]==0x23 && receivedData[1]==0x00){
        //drive motor A.
        //show_RGB(1, 2);
        drive_motor(MA1, MA2, (char)receivedData[3], (char)receivedData[4]); //only works when bytes.
        //drive_motor(MA1, MA2, 0, 0); //this works but 
      }
      postflag = false;
    }else{
      FOR(i,dataLength) receivedData[i]=0;
    }
    
//    digitalWrite(RLED1, dr);
//    dr = !dr;
    delay(10);
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
  #ifdef CLR_ON
    delay(40);
    TCA9548A(0);
    delay(40);
    //RGBWSensor.setConfiguration(VEML6040_IT_40MS + VEML6040_TRIG_ENABLE + VEML6040_AF_FORCE + VEML6040_SD_ENABLE);
    red = RGBWSensor.getRed();
    green = RGBWSensor.getGreen();
    blue = RGBWSensor.getBlue();
    mred = map(red, 0,30000,254,0); //red is very sensitive, 200 to 900, was already a high amount, how to lower?
    mgreen = map(green, 0,1000,254,0);
    mblue = map(blue, 0,700,254,0); //the lower the range, more amplify
    analogWrite(RGB_R, mred);
    analogWrite(RGB_B, mblue);
    analogWrite(RGB_G, mgreen);
    delay(40);
    // delay(40);
    // TCA9548A(1);
    // delay(40);
    // RGBWSensor.setConfiguration(VEML6040_IT_40MS + VEML6040_TRIG_ENABLE + VEML6040_AF_FORCE + VEML6040_SD_ENABLE);
    // red = RGBWSensor.getRed();
    // green = RGBWSensor.getGreen();
    // blue = RGBWSensor.getBlue();
    // mred = map(red, 0,5000,254,0); //red is very sensitive, 200 to 900, was already a high amount, how to lower?
    // mgreen = map(green, 0,800,254,0);
    // mblue = map(blue, 0,700,254,0); //the lower the range, more amplify
    // analogWrite(RGB_R, mred);
    // analogWrite(RGB_B, mblue);
    // analogWrite(RGB_G, mgreen);
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
    digitalWrite(RLED1, 0);
    delay(delaytime);
    digitalWrite(RLED1, 1);
    delay(delaytime);
  }
}
int rled_flip=0;
//long RGB = 0x000000; //this will be full brightness on all three leds
void show_RGB(long val, int mode){
  if (mode == 0){
    analogWrite(RGB_R,val>>16 & 0xFF);
    analogWrite(RGB_G,val>>8  & 0xFF);
    analogWrite(RGB_B,val     & 0xFF);
  }else if(mode ==1){
    digitalWrite(RGB_R, val>>16 & 0xFF);
    digitalWrite(RGB_G,val>>8  & 0xFF);
    digitalWrite(RGB_B,val     & 0xFF);
  }else if(mode ==2){
    //FOR(i,24){
    FOR(i,8){
      digitalWrite(WLED1,(val>>i)&1);
      digitalWrite(RLED1,rled_flip);
      rled_flip = !rled_flip;
      delay(100);
      digitalWrite(WLED1,0);
      digitalWrite(RLED1,rled_flip);
      rled_flip = !rled_flip;
      delay(100);
    }
  }
}

//This is the Multiplexor control
void TCA9548A(uint8_t bus){
  Wire.beginTransmission(0x70);  // TCA9548A address
  Wire.write(1 << bus);          // send byte to select bus
  Wire.endTransmission();
}