/* use timeout instead of delay between
  sending AT command and receiving reply over serial
  put MCU in sleep mode 2 (power down mode) at end of test, only INT pin change wakes up MCU
  previously program goes to while(1) at beginning of test,
  short press start test, while long press resets MCU
  this is not a desired behaviour
  MCU should only resets after test
*/

#include <UTFT.h>
#include <avr/pgmspace.h>
#include <SPI.h>
#include <avr/sleep.h>
#include <avr/wdt.h>

#include "test.hpp"
#include "status.hpp"

#define DEBUG_ENABLED   1

#define _VERSION        "V2022A"
#define _MODEL          "MODEL D"
#define color_primary   (244,81,30)
#define color_warning   (0,35,255)

extern uint8_t Retro8x16[];
extern unsigned int arm3[];

UTFT myDisplay(ST7735, 11, 13, 3, 2, 4);

char *commands[] =   {"SN",
                      "HWREV",
                      "MODEL",
                      "EERAM1",
                      "EERAM2",
                      "RTC1",
                      "RTC2",
                      "FLASH",
                      "VSD",
                      "BAS",
                      "CAN",
                      "BATT",
                      "ACCEL",
                      "LCD",
                      "ETH",
                      "WIFI"};

char test_results[sizeof(commands)/sizeof(char*)][20] = {0};      // initialize to empty strings
int ok_index=0;

float currentOffset = 0.120; //0.105
int current = 0;
float voltage5V = 0;
float voltage3v3 = 0;
float voltage1v2 = 0;
float voltage24 = 0;
float current24 = 0;
float vcc = 0.0;
float tp60 = 0.0;
float tp6 = 0.0;                                                  // moved from insdie loop()
float tp7 = 0.0;                                                  // Serial.print() these two variables break the code?!

float fail5V = 0;
float fail3V3 = 0;
float fail1V2 = 0;
float fail24V = 0;
float fail24A = 0;
float failVcc = 0.0;

float r1 = 10000.0;
float r2 = 2200.0;

int measureCount = 0;
int buttonhold = 0;
int state = HIGH;
int state2 = HIGH;
int button = 1;
int failflag = 0;

int powerfail = 0;
int lcdfail = 0;
int lcdfail2 = 0;
int rtcfail = 0;

char buf[12];
char voltData[30];


void(* resetFunc) (void) = 0;

void setup() {
  // Serial3.begin(19200);
  Serial.begin(19200);

  set_sleep_mode(2);              // put MCU in power-down mode

  pinMode(18, INPUT);
  // pinMode(19, INPUT);
  pinMode(21, OUTPUT);
  pinMode(20, INPUT); 

  digitalWrite(21, LOW);

  myDisplay.InitLCD();
  myDisplay.clrScr();
  myDisplay.setFont(Retro8x16);

  attachInterrupt(digitalPinToInterrupt(18), next, LOW);
  // attachInterrupt(digitalPinToInterrupt(19), next2, LOW);
  //myDisplay.fillScr(255, 255, 255);
  myDisplay.drawBitmap(20, 1, 126, 126, arm3, 1);
  myDisplay.setColor(color_warning);
  myDisplay.print(_VERSION, LEFT, 0);
  myDisplay.print(_MODEL, RIGHT, 0);
}

void loop()
{
  while (1) {
    delay(100);
    if (state == LOW) {
      break;
    }
  }

  myDisplay.clrScr();
  myDisplay.setColor(0, 255, 255);
  myDisplay.print("Voltage Test", CENTER, 0);

  pinMode(20, OUTPUT);
  digitalWrite(20, LOW);
  delay(1000);
  digitalWrite(20, HIGH);
  pinMode(20, INPUT);

  failflag = 0;
  state = HIGH;
  buttonhold = 0;

  while (measureCount < 3) {
    failflag = 0;
    float voltage_value = 0;
    for (int i = 0; i < 1000; i++) {
      voltage_value = (voltage_value + (4.98 / 1024 * analogRead(A0)));
    }
    voltage_value = voltage_value / 1000;
    vcc = voltage_value / (r2 / (r1 + r2));

    if (vcc < 0.1)
    {
      vcc = 0.0;
    }
    voltage5V = analogRead(A1);
    voltage5V = (voltage5V * 5.00) / 1024.0;
    voltage3v3 = analogRead(A2);
    voltage3v3 = (voltage3v3 * 4.99) / 1024.0;
    voltage1v2 = analogRead(A3);
    voltage1v2 = (voltage1v2 * 5.00) / 1024.0;

    for (int i = 0; i < 1000; i++) {
      voltage24 = (voltage24 + (4.80 / 1024 * analogRead(A7)));
    }

    voltage24 = voltage24 / 1000;
    current24 = ((voltage24 - 2.38) / 0.185) - currentOffset;

    myDisplay.setColor(255, 255, 255);
    myDisplay.print(" 24V (V): ", 0, 20);
    //Serial.println(vcc);

    if (vcc >= 22.8 && vcc <= 25.2)
    {
      myDisplay.setColor(0, 255, 0);
      myDisplay.printNumF(vcc, 2, 110, 20);
      //myDisplay.print("  ", 130, 20);
    }
    else {
      {
        myDisplay.setColor(0, 0, 255);
        myDisplay.printNumF(vcc, 2, 110, 20);
        myDisplay.print(" ", 142, 20);
        failflag = 1;
        failVcc = vcc;
      }
    }

    myDisplay.setColor(255, 255, 255);
    myDisplay.print(" +5V: ", 0, 38);//0, 38

    if (voltage5V >= 4.75 && voltage5V <= 5.25)
    {
      myDisplay.setColor(0, 255, 0);
      myDisplay.printNumF(voltage5V, 2, 110, 38);
      //myDisplay.print("  ", 130, 20);
    }
    else {
      {
        myDisplay.setColor(0, 0, 255);
        myDisplay.printNumF(voltage5V, 2, 110, 38);
        myDisplay.print(" ", 142, 56);
        failflag = 1;
        fail5V = voltage5V;
      }
    }

    myDisplay.setColor(255, 255, 255);
    myDisplay.print(" +3.3V: ", 0, 56); //0, 56

    if (voltage3v3 >= 3.135 && voltage3v3 <= 3.465)
    {
      myDisplay.setColor(0, 255, 0);
      myDisplay.printNumF(voltage3v3, 2, 110, 56);
      //myDisplay.print("  ", 130, 20);
    }
    else {
      {
        myDisplay.setColor(0, 0, 255);
        myDisplay.printNumF(voltage3v3, 2, 110, 56);
        myDisplay.print(" ", 142, 74);
        failflag = 1;
        fail3V3 = voltage3v3;
      }
    }

    myDisplay.setColor(255, 255, 255);
    myDisplay.print(" +1.2V: ", 0, 74); //0, 74


    if (voltage1v2 >= 1.14 && voltage1v2 <= 1.26)
    {
      myDisplay.setColor(0, 255, 0);
      myDisplay.printNumF(voltage1v2, 2, 110, 74);
    }
    else {
      {
        myDisplay.setColor(0, 0, 255);
        myDisplay.printNumF(voltage1v2, 2, 110, 74);
        myDisplay.print(" ", 142, 92);
        failflag = 1;
        fail1V2 = voltage1v2;
      }
    }

    if (failflag == 1) {
      myDisplay.setColor(0, 0, 255);
      myDisplay.print("FAIL", CENTER, 113);
      powerfail = 1;
      while (1) {
        button = digitalRead(18);
        #ifdef DEBUG_ENABLED
          break;
        #endif
        if (button == 0) {
        resetFunc();
      }
    }
    }
    else
    {
      myDisplay.setColor(0, 255, 0);
      myDisplay.print("PASS", CENTER, 113);
      powerfail = 0;
    }
    // Serial.println(measureCount);
    measureCount++;
  }

  buttonhold = 0;
  myDisplay.clrScr();
  // float tp6 = 0.0;
  // float tp7 = 0.0;
  measureCount = 0;
  
  while (measureCount < 3) {

    int analog_value2 = analogRead(A4);

    float failtp6 = 0.0;
    float failtp7 = 0.0;
    for (int i = 0; i < 1000; i++) {
      tp7 = (tp7 + (4.97 / 1024 * analogRead(A5)));
    }
    tp7 = tp7 / 1000;

    for (int i = 0; i < 1000; i++) {
      tp6 = (tp6 + (4.97 / 1024 * analogRead(A4)));
    }
    tp6 = tp6 / 1000;
    //    tp7 = analogRead(A5);
    //    tp7 = (tp7 * 4.99) / 1024.0;

    //temp = (analog_value2 * 4.99) / 1024.0;
    tp6 = tp6 / (r2 / (r1 + r2));

    if (tp6 < 0.1)
    {
      tp6 = 0.0;
    }
    // Serial.println(analog_value2);
    myDisplay.setColor(255, 255, 255);
    myDisplay.print("TP6 (V): ", LEFT, 2);

    if (tp6 >= 14.0 && tp6 <= 23.0)
    {
      myDisplay.setColor(0, 255, 0);
      myDisplay.printNumF(tp6, 2, 110, 2);
      //myDisplay.print("  ", 130, 20);
      lcdfail = 0;
    }
    else {
      {
        myDisplay.setColor(0, 0, 255);
        myDisplay.printNumF(tp6, 2, 110, 2);
        myDisplay.print(" ", 142, 2);
        failflag = 1;
        lcdfail = 1;
        failtp6 = tp6;
      }
    }

    myDisplay.setColor(255, 255, 255);
    myDisplay.print("TP7 (V): ", LEFT, 20);

    if (tp7 >= 0.1 && tp7 <= 5.0)
    {
      myDisplay.setColor(0, 255, 0);
      myDisplay.printNumF(tp7, 2, 110, 20);
      //myDisplay.print("  ", 130, 20);
      lcdfail2 = 0;
    }
    else {
      {
        myDisplay.setColor(0, 0, 255);
        myDisplay.printNumF(tp7, 2, 110, 20);
        myDisplay.print(" ", 142, 20);
        failflag = 1;
        lcdfail2 = 1;
        failtp7 = tp7;
      }
    }

    float ftp60 = 0.0;
    for (int i = 0; i < 1000; i++) {
      tp60 = (tp60 + (4.99 / 1024 * analogRead(A6)));
    }

    tp60 = tp60 / 1000;

    myDisplay.setColor(255, 255, 255);
    myDisplay.print("TP60 (V): ", LEFT, 38);

    if (tp60 >= 3.0 && tp60 <= 3.2)
    {
      myDisplay.setColor(0, 255, 0);
      myDisplay.printNumF(tp60, 2, 110, 38);
      //myDisplay.print("  ", 130, 20);
      rtcfail = 0;
    }
    else {
      {
        myDisplay.setColor(0, 0, 255);
        myDisplay.printNumF(tp60, 2, 110, 38);
        myDisplay.print(" ", 142, 38);
        failflag = 1;
        rtcfail = 1;
        ftp60 = tp60;
      }
    }

    if (failflag == 1) {
      myDisplay.setColor(0, 0, 255);
      myDisplay.print("FAIL", CENTER, 110);
      while (1) {
        button = digitalRead(18);
        #ifdef DEBUG_ENABLED
          break;
        #endif
        if (button == 0) {
        resetFunc();
        }
      }
    }
    else
    {
      myDisplay.setColor(0, 255, 0);
      myDisplay.print("PASS", CENTER, 110);
    }
    measureCount++;

  }

  delay(1000);
  measureCount = 0;
  myDisplay.clrScr();
  buttonhold = 0;
  state = HIGH;
  failflag = 0;

  /* RUN TESTS HERE */
  for (auto i=3;i<sizeof(commands)/sizeof(char*);i++) {           // special cases for SN, HWREV, MODEL
    run_test(myDisplay, (char*)commands[i], 1000);
    if (ok_index > 2) {
      myDisplay.setColor(color_warning);
      myDisplay.print("communication lost",CENTER,40);
      myDisplay.setColor(125,125,125);
      myDisplay.print("check serial ...",CENTER,70);
      while (1) {
        //
      };
    }
  }
  

  /* PULL STATUS HERE */
  for (auto i=0;i<sizeof(commands)/sizeof(char*);i++) {
    // if (i == 14) {                                                // special cases for ETH & WIFI, wait till they fail
    //   myDisplay.print("Waiting for ...",CENTER,30);
    //   myDisplay.print(commands[i],CENTER,60);
    //   delay(38000);
    //   myDisplay.clrScr();
    // }
    poll_stat(myDisplay, (char*)commands[i], test_results[i], 1000);
  }

  Serial.print(',');
  Serial.print(test_results[0]);
  Serial.print(',');
  Serial.print(test_results[1]);
  Serial.print(',');
  Serial.print(test_results[2]);
  Serial.print(',');
  Serial.print(vcc);
  Serial.print(',');
  Serial.print("n/a");
  Serial.print(',');
  Serial.print(voltage5V);
  Serial.print(',');
  Serial.print(voltage3v3);
  Serial.print(',');
  Serial.print(voltage1v2);
  Serial.print(',');
  Serial.print(tp6);
  Serial.print(',');
  Serial.print(tp7);
  Serial.print(',');
  Serial.print(tp60);
  Serial.print(',');

  char message_buffer[4] = "n/a";
  char buffer[200];
  sprintf(buffer, "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\r\n", message_buffer,test_results[12],test_results[12],test_results[12],test_results[7],message_buffer,message_buffer,message_buffer,message_buffer,message_buffer,test_results[10],message_buffer,test_results[3],test_results[4],test_results[5],test_results[6],test_results[8],test_results[9],test_results[11],test_results[13],test_results[14],test_results[15]);
  Serial.write(buffer);

  uint8_t j=0;
  for (auto i=3;i<sizeof(commands)/sizeof(char*);i++) {           // skip first 3 (SN, MODEL, REVISION)
    if ((strstr(test_results[i], "FAIL") != NULL) || (strlen(test_results[i])==0)) {
      myDisplay.setColor(color_warning);
      if (j<7) {
        myDisplay.print(commands[i], LEFT, 20+j*15);              // first column
      }
      else {
        myDisplay.print(commands[i], 100, 20+(j-7)*15);           // second column
      }
      j++;
    }
  }

  if (j) {
    myDisplay.setColor(255,255,255);
    myDisplay.print("Test(s) Failed",CENTER,0);
  }
  else {
    myDisplay.setColor(255,255,255);
    myDisplay.print("Test Completed",CENTER,40);
    myDisplay.setColor(125,125,125);
    myDisplay.print("Press to restart",CENTER,70);
  }

  // sleep_mode();                   // in power-down mode, only INT pin change/twi address match/ WTD interrupt wakes up MCU
  for (;;);
}

void next() {
  state = LOW;
  // Serial.println(buttonhold);                                     // WITHOUT THIS LINE, ONE PRESS GENERATES TOO MANY COUNTS, ARDUINO KEEPS GETTTING RESET
  delay(2);                                                       // a delay() is put in place instead
  buttonhold++;                             
  if (buttonhold > 350) {                   
    resetFunc();
  }
}

void next2() {
  state2 = LOW;
}