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
  // //tof
  // Wire.begin(); //as master
  // if (!sensor.init()) {
  //   FOR(k,3){
  //   signalling(30);
  //   delay(1000);
  //   }
  // }
  // sensor.setTimeout(500);
  // sensor.startContinuous();
  // delay(100);
  // Wire.end();//quit master here,
  // Serial.begin(9600);
  // Serial.println("Hello From AT1");
  // Wire.setClock(400000);
  Wire.onReceive(receiveData); // callback for receiving data
  Wire.onRequest(sendData); // callback for sending data
  Wire.begin(SLAVE_ADDRESS); // join i2c bus as slave
  // show_RGB(0xFFFFFF,0); //RGB off
  // FOR(i, 0xFF){
  //   show_RGB(0xFFFFFF - i*0x010000,0); //from dark to red
  // }
  // FOR(i, 0xFF){ //red to blue
  //   show_RGB(0x00FFFF - i*0x000001 + i*0x010000,0);
  // }
  // FOR(i, 0xFF){ //blue to green
  //   show_RGB(0xFFFF00 - i*0x000100 + i*0x000001,0);
  // }
  // FOR(i, 0xFF){ //green to red
  //   show_RGB(0xFF00FF - i*0x010000 + i*0x000100,0);
  // }
  // FOR(i, 0xFF){ //red to off
  //   show_RGB(0x00FFFF + i*0x010000,0); //from dark to red
  // }
  show_RGB(0xFFFFFF,0);
}
long data=0xFFFFFF;
// arduino long type has 4 bytes, 0xFF FF FF FF, signed. ranged -2,147,483,648 to 2,147483,647
void loop() {  
  if(postflag == 1){
    if(receivedData[0]=='R' && receivedData[1]=='G'){
      //drive RGB
      data = ((long)receivedData[3]<<16 & 0xFF0000) | (receivedData[4]<<8 & 0xFF00) | (receivedData[5]&0xFF);
      show_RGB(data, 0);
    }else if(receivedData[0]=='M' && receivedData[1]=='A'){
      //drive motor A.
      drive_motor(MA1, MA2, (char)receivedData[3], (char)receivedData[4]); //only works when bytes.
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
    }
    postflag = 0;
  }else{
    FOR(i,dataLength) receivedData[i]=0;
    // //do master work
    // Wire.end();
    // Wire.begin();
    // head=sensor.readRangeContinuousMillimeters();
    // //if (sensor.timeoutOccurred()) FOR(k,3)signalling(50);
    // outgoingData[0] = head >> 8 & 0xFF;
    // outgoingData[1] = head & 0xFF;
    // Wire.end();
    // Wire.onReceive(receiveData); // callback for receiving data
    // Wire.onRequest(sendData); // callback for sending data
    // Wire.begin(SLAVE_ADDRESS); //continue with slave
  }
  delay(1);
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
// three modes: 0=analog all, 1 is digital all, 2 is bitwise least significan bit first.
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
