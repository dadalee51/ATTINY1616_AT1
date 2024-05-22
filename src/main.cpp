#include <Arduino.h>
/*** AT1 - slave 0x12
 AT1 of DryBot v2.3a, may 10 2024.
 Drybot v2.5b may19th 2024. update pin positions.
 */
#include <Wire.h>
#include <VL53L0X.h>
VL53L0X sensor; //0x29

#define FOR(I,N) for(int I=0;I<N;I++)
#define ENC_MA_A PIN_PC0
#define ENC_MA_B PIN_PC1
#define ENC_MB_A PIN_PC2
#define ENC_MB_B PIN_PC3
#define ENC_MC_A PIN_PA6
#define ENC_MC_B PIN_PA7
#define MA1 PIN_PA4
#define MA2 PIN_PA5
#define RGB_G PIN_PB4
#define RGB_B PIN_PB5
#define RGB_R PIN_PA3 
#define WLED1 PIN_PA2 //dryBot LEDW 0 = off
#define RLED1 PIN_PA1 //dryBot LEDR 1 = off
//v2.5b added MB1 and MB2 pins
#define MB1 PIN_PB2
#define MB2 PIN_PB3

//headers
void show_RGB(long val,int mode);
void drive_motor(int,int,int,int);
void signalling(int);
void receiveData(int numBytes);
void sendData();
void debugData(long data, int numdig);
/*** Wire interface **********************************************/
#define SLAVE_ADDRESS 0x12
#define BUFFER_SIZE 20 
char receivedData[BUFFER_SIZE]; 
char outgoingData[BUFFER_SIZE];
int dataLength = 0; 
int postflag = 0;

int head=0; //tof

//master send
void receiveData(int numBytes) {
  //postflag = 0;
  dataLength = numBytes;
  Wire.readBytes(receivedData, numBytes);
  postflag = 1;//mark data ready
}
//master read
void sendData() {
  Wire.write(outgoingData, 2);
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
  digitalWrite(RLED1, 0); // 1 off, 0 on
  digitalWrite(WLED1, 0); // 1 on , 0 off.
  Wire.setClock(400000);
  Wire.onReceive(receiveData); // callback for receiving data
  Wire.onRequest(sendData); // callback for sending data
  Wire.begin(SLAVE_ADDRESS); // join i2c bus as slave
  show_RGB(0xFFFFFF,0); //turn off RGB
  // drive_motor(MA1,MA2,1,100);
  // delay(300);
  // drive_motor(MA1,MA2,0,0);
  // delay(300);
  // drive_motor(MA1,MA2,-1,100);
  // delay(300);
  // drive_motor(MA1,MA2,2,100);
  // delay(300);
  // drive_motor(MA1,MA2,0,0);
}
long data=0xFFFFFF;
// arduino long type has 4 bytes, 0xFF FF FF FF, signed. ranged -2,147,483,648 to 2,147483,647
void loop() {  
  int rled_flip=0;
  delayMicroseconds(500);
  if(postflag == 1){
    if(receivedData[0]=='R' && receivedData[1]=='G'){
      //drive RGB
      data = ((long)receivedData[3]<<16 & 0xFF0000) | (receivedData[4]<<8 & 0xFF00) | (receivedData[5]&0xFF);
      show_RGB(data, 0);
    }else if(receivedData[0]=='M' && receivedData[1]=='A'){
      //drive motor A.
      // debugData((char)receivedData[2],8);
      drive_motor(MA1, MA2, (char)receivedData[2], (char)receivedData[3]); //only works when bytes.
    }else if(receivedData[0]=='M' && receivedData[1]=='B'){
      //drive motor B
      drive_motor(MB1, MB2, (char)receivedData[2], (char)receivedData[3]); //only works when bytes.
    }else if(receivedData[0]=='W' && receivedData[1]=='L'){
      //drive WLED1
      digitalWrite(WLED1, receivedData[3]=='A'?1:0);
    }else if(receivedData[0]=='C' && receivedData[1]=='h'){
      //received Char array
      int lngth = (int)receivedData[3];
    }else if(receivedData[0]=='L' && receivedData[1]=='o'){
      //received long value
      long rec = *(long*)(&receivedData[3]);
    }else if(receivedData[0]=='I' && receivedData[1]=='n'){
      //received int value
      int rec = *(int*)(&receivedData[3]);
    }else{
      //not in spec.
      digitalWrite(RLED1,rled_flip);
      rled_flip = !rled_flip;
    }
    postflag = 0;
  }else{
    FOR(i,dataLength) receivedData[i]=0;
  }
}

/*
 * dir 1/ 0 / -1 / 10 / 11 / 2
 * power = mimic lego's design: 0~127
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
  }else if(dir==2) {
    digitalWrite(p1, 1);
    digitalWrite(p2, 1);
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

//long RGB = 0x000000; //this will be full brightness on all three leds
//modes: 0=analog all, 1 is digital all
void show_RGB(long val, int mode){
  if (mode == 0){
    analogWrite(RGB_R,(val>>16) & 0xFF);
    analogWrite(RGB_G,(val>>8)  & 0xFF);
    analogWrite(RGB_B,val       & 0xFF);
  }else if(mode ==1){
    digitalWrite(RGB_R,(val>>16) & 0xFF);
    digitalWrite(RGB_G,(val>>8)  & 0xFF);
    digitalWrite(RGB_B,val      & 0xFF);
  }
}
/**
 * Display any byte, ex, 0b00001111, numdig=8
 * wled blink first 4 times.
 * rled blink all 8 times.
*/
void debugData(long val, int numdig){
  int rled_flip=0;
  FOR(i,numdig){
    if(i%8==0)delay(500);
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
