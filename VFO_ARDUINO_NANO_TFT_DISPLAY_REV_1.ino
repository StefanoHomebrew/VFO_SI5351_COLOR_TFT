/***************************************************
  This is our GFX example for the Adafruit ILI9341 Breakout and Shield
  ----> http://www.adafruit.com/products/1651

  Check out the links above for our tutorials and wiring diagrams
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional)
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution

  ******************************************************************************
  * @file    VFO_ARDUINO_NANO_TFT_DISPLAY_REV_1.ino
  * @author  Stefano Homebrew
  * @version V1.0
  * @date    12-01-2024
  *
  ******************************************************************************
  *
  *                     VFO_ARDUINO_NANO_TFT_DISPLAY - PROJECT
  *
  * This project "VFO_LED_DISPLAY-RX-TX with si5351", define an experimental open platform to build
  * a RX/TX Vfo for Radio's Homebrewing 
  * 
  * Library: 
  * Metro library: https://github.com/thomasfredericks/Metro-Arduino-Wiring
  * Tft Display https://github.com/adafruit/Adafruit_ILI9341
  * Tft Display Graphics  https://learn.adafruit.com/adafruit-gfx-graphics-library/overview
  * Rotary Encoder https://github.com/brianlow/Rotary
  * Si5351 https://github.com/etherkit/Si5351Arduino
  * 
  * My projects videos https://www.youtube.com/@stefanohomebrew
  *
  * NOTE: this is an experimental project and the functions can be changed
  * without any advise.
  * The VFO_ARDUINO_NANO_TFT_DISPLAY uses examples and projects freeware and available in the opensource
  * community.
  *
  * The VFO_ARDUINO_NANO_TFT_DISPLAY openSource software is released under the license:
  *              Common Creative - Attribution 3.0
  * ***************************************************************************            
  *                              --VFO Functions--
  * - Analog input: for S-Meter or other...            
  * - Relays output fon transceiver P.T.T. or other...
  *              
  * Buttons:             
  * - STEP MODE (INCLUDE ON ROTARY ENCODER)
  * - BANDS (programmable for your use)
  * - I.F. FREQUENCY (programmable for your use)
  * - B.F.O. ON/OFF
  * - R-TX FOR P.T.T. FOR TRANSCEIVER (OPTIONAL FOR HAM RADIO USERS)
  * - TFT COLORR DISPLAY IL9341 !!!ATTENZIONE DEVE ESSERE ALIMENTATO A 3.3 VOLTS!!!!---ATTENTION: 3.3VOLTS POWER SUPPLY----
  ******************************************************************************
*/
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include <Rotary.h>
#include "Wire.h"
#include <si5351.h>
#include <Metro.h>

// For the Adafruit shield, these are the default.
#define TFT_DC 9
#define TFT_CS 10

/////////////////////////////////////////////////////My color presets
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF
#define MARRONE   0x5041
#define BLUSCURO 0x0004
#define BLUES 0X0008

unsigned long freq_old = 0;
unsigned long freq = 9610000; 
unsigned long freqSi = 455000;
unsigned long freqPLL;


int bottoneStep = A0;
int bottoneBand = A3;
int bottoneMfrq = A2;
int bottoneRxTx = A1;
int bottoneBfo = 8;
int analogIn = A7;

int letturaAnalog = 0.0;
float VoltageIn =0.0;

byte var5= 0;
byte var6= 0;

long bfoPLL = 455000;
byte bfo= 1;
byte varBfo= 0;

byte mdfrq = 0;

byte band=0;
int passo = 10000;
byte pass =0;


/////////////////SMETER
int pos_x_smeter = 10;
int pos_y_smeter = 116;
float uvold = 0; 
int varSmeter = 0;
unsigned long Smeter1_old = 0;
unsigned long Smeter1 = 0; 

byte unita,decine,centinaia,migliaia,decinemigliaia,centmigliaia,milioni ;  //variabili lettura frequenza sul display TFT
int var = 0;

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
Rotary r = Rotary(3, 2);
Si5351 si5351;
Metro smeterMetro = Metro(20);
/////////////////////////////////////////SETUP 
void setup() {
    Serial.begin(9600);
    pinMode(bottoneStep,INPUT_PULLUP);
    pinMode(bottoneBand,INPUT_PULLUP);
    pinMode(bottoneMfrq,INPUT_PULLUP);
    pinMode(bottoneBfo,INPUT_PULLUP);
    pinMode(bottoneRxTx,INPUT_PULLUP);
    pinMode(5,OUTPUT);
    digitalWrite(5, LOW);
///////////////////////////TFT INIT SETUP
    tft.begin();
    tft.setRotation(3); 
    tft.fillScreen(ILI9341_BLACK);
/////////////////////////////////////ROTARY ENCODER INTERRUPT     
    PCICR |= (1 << PCIE2);
    PCMSK2 |= (1 << PCINT18) | (1 << PCINT19);
    sei();
//////////////////////////////////////////////SI5351 INIT VALUE
    si5351.init(SI5351_CRYSTAL_LOAD_8PF, 0,0);
    si5351.set_correction(8000, SI5351_PLL_INPUT_XO);
    si5351.drive_strength(SI5351_CLK1, SI5351_DRIVE_2MA);
    si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_8MA);
    si5351.output_enable(SI5351_CLK1, 1);///////////////TX OUT (OPTIONAL)
    si5351.output_enable(SI5351_CLK0, 0);///////////////RX OUT
    /////////////////////////////////////////////////////////////////////
    stampafrq();
    bfo1();
    grafica();
    S_Meter();
 }
////////////////////////////////ROTARY ENCODER //////////////  
ISR(PCINT2_vect) {
  unsigned char result = r.process();
  if (result == DIR_NONE) {
    // do nothing
  }
  else if (result == DIR_CW) {  
    freq = (freq + passo);

   stampafrq();
  }
  else if (result == DIR_CCW) {   
    freq = (freq - passo);
  
   stampafrq();
  } 
  }
//////////////////////////////////////////////////
void loop() { 
////////////////////////////S-METER INPUT (OPTIONAL)   
    letturaAnalog = analogRead(analogIn);
    letturaAnalog = map(letturaAnalog, 0, 255, 0, 11);
    VoltageIn = letturaAnalog*100;
    S_Meter();      
    /////////////////////////////////////IF ROTARY ENCODER ROTATION         
     if (freq != freq_old)  {   
    inviapll();
     freq_old = freq;
     }  

/////////////////////////////////////////////BUTTONS
if (digitalRead(bottoneRxTx) == LOW){  
    var5= 1; //memorizza sulla variabile il bottone 
    if (var5 != var6){   /////////riconosce se viene premuto il tasto
    trasmetti(); 
    var6 = var5;  
  }
}
  
if (digitalRead(bottoneRxTx) == HIGH){
    var5 = 0;
    if (var5!= var6){   /////////riconosce se viene premuto il tasto     
    ricevi();
    var6 = var5; 
   }
}
//////////////////////////////END BOTTONE RTX 
    if (digitalRead(bottoneStep) == LOW) {
    delay(200); 
    Step();
    }
    ////////////////////////////////START OPTIONAL BANDS SWITCH
 
    if (digitalRead(bottoneBand) == LOW) {
    delay(200); 
    Band();
    }
   
    ////////////////////////////////END OPTIONAL BANDS SWITCH
    if (digitalRead(bottoneMfrq) == LOW) {
    delay(200); 
    MediaFrq(); 
    }
    
    ///////////////////////////////BFO ON OFF
    if (digitalRead(bottoneBfo) == LOW) {
    delay(200); 
    bfo1();
    } 

        
  }
/////////////////////////////BANDS 
void Band(){  
   band++;
   if (band == 1) {
      freq = 1860000;
      tft.fillRoundRect(1, 10, 99, 70,5,ILI9341_RED); //CHANNEL CORNICE
      tft.drawRoundRect(1, 10, 99, 70,5,ILI9341_WHITE); //CHANNEL CORNICE
      tft.setCursor(24,22); 
      tft.setTextColor(ILI9341_GREEN);
      tft.setTextSize(2);
      tft.print("BAND");
      tft.setCursor(18,50); 
      tft.setTextColor(WHITE);
      tft.setTextSize(2);
      tft.print("160Mt.");    
      inviapll();    
      passo = 1000;
    }
    if (band == 2) {
      freq = 3600000;
      tft.fillRoundRect(1, 10, 99, 70,5,ILI9341_RED); //CHANNEL CORNICE
      tft.drawRoundRect(1, 10, 99, 70,5,ILI9341_WHITE); //CHANNEL CORNICE
      tft.setCursor(24,22); 
      tft.setTextColor(ILI9341_GREEN);
      tft.setTextSize(2);
      tft.print("BAND");
      tft.setCursor(18,50); 
      tft.setTextColor(WHITE);
      tft.setTextSize(2);
      tft.print("80 Mt.");    
      inviapll();    
      passo = 1000;
    }   
   if (band == 3) {
    freq = 7100000;
      tft.fillRoundRect(1, 10, 99, 70,5,ILI9341_RED); //CHANNEL CORNICE
      tft.drawRoundRect(1, 10, 99, 70,5,ILI9341_WHITE); //CHANNEL CORNICE
      tft.setCursor(24,22); 
      tft.setTextColor(ILI9341_GREEN);
      tft.setTextSize(2);
      tft.print("BAND");
      tft.setCursor(18,50); 
      tft.setTextColor(WHITE);
      tft.setTextSize(2);;
      tft.print("40 Mt.");    
    inviapll();     
      passo = 1000;
    }   
   if (band == 4) {
    freq = 9610000;
      tft.fillRoundRect(1, 10, 99, 70,5,ILI9341_RED); //CHANNEL CORNICE
      tft.drawRoundRect(1, 10, 99, 70,5,ILI9341_WHITE); //CHANNEL CORNICE
      tft.setCursor(24,22); 
      tft.setTextColor(ILI9341_GREEN);
      tft.setTextSize(2);
      tft.print("BAND");
      tft.setCursor(18,50); 
      tft.setTextColor(WHITE);
      tft.setTextSize(2);
      tft.print("31 Mt.");    
    inviapll();     
      passo = 1000;
    }
     stampafrq();
       if (band == 4) {
        band = 0;
       }
   //    Serial.println(band);
}
////////////////////////////////SEND TO SI5351
void inviapll() {
  //  Serial.println(freq);
    freqPLL = (freq + freqSi);///////////////////////+ or -
    si5351.set_freq ((freqPLL * 100), SI5351_CLK0);
  // Serial.println (freqPLL);
}
////////////////////////////////STEP PASSO   
void Step(){   
    pass++;
    tft.setTextSize(2);
    if (pass > 3) {
      (pass = 1);
    }
    if (pass < 1) {
      (pass = 1);
    }
   if (pass == 1) {
      tft.fillRoundRect(10, 130, 60, 20,5, BLUE);//BOTTONE 
      tft.drawRoundRect(10, 130, 60, 20,5, WHITE);//BOTTONE 
      tft.setTextColor(WHITE);
      tft.setCursor(20,133);
      tft.print("1  k");
    
      passo = 1000;
    }
   if (pass == 2) {
      tft.fillRoundRect(10, 130, 60, 20,5, BLUE);//BOTTONE 
      tft.drawRoundRect(10, 130, 60, 20,5, WHITE);//BOTTONE 
      tft.setTextColor(WHITE);
      tft.setCursor(20,133);
      tft.print("10 k");
      
      passo = 10000;
    }
   if (pass == 3) {
      tft.fillRoundRect(10, 130, 60, 20,5, BLUE);//BOTTONE 
      tft.drawRoundRect(10, 130, 60, 20,5, WHITE);//BOTTONE 
      tft.setTextColor(WHITE);
      tft.setCursor(20,133);
      tft.print("100");
     
      passo = 100;    
    }
            
 Serial.println(passo);   
}
//////////////////////////////////////////////////////// 
void stampafrq(){  
      milioni = int(freq/1000000);
      centmigliaia = ((freq/100000)%10);
      decinemigliaia = ((freq/10000)%10);
      migliaia = ((freq/1000)%10);
      centinaia = ((freq/100)%10);
      decine = ((freq/10)%10);
      unita = ((freq/1)%10);
      tft.fillRect(112, 44, 183, 32, BLACK);
      tft.setTextColor(ILI9341_WHITE);
      tft.setCursor(135,44);
      tft.setTextSize(3);
      tft.print(milioni);//
      tft.print(".");
      tft.print(centmigliaia);
      tft.print(decinemigliaia);
      tft.print(migliaia);
      tft.print(".");
      tft.print(centinaia);
      tft.print(decine); 
}     
///////////////////////////////////////////////BFO OUT - 0N/OFF USCITA CLOCK 1 SI 5351
void bfo1(){ 
     bfo++;
     switch (bfo) {  
case 1:
      si5351.output_enable(SI5351_CLK1, 1);
      si5351.set_freq ((bfoPLL * 100), SI5351_CLK1);// ATTIVA USCITA CLOCK 1 SI 5351   
      tft.fillRoundRect(170, 130, 60, 20,5, RED);//BOTTONE 
      tft.drawRoundRect(170, 130, 60, 20,5, WHITE);//BOTTONE 
      tft.setTextColor(WHITE);     
      tft.setTextSize(2);
      tft.setCursor(184,133);
      tft.print("BFO");
break;

case 2:
      si5351.output_enable(SI5351_CLK1, 0);//DISATTIVA USCITA CLOCK 1 SI 5351
      tft.fillRoundRect(170, 130, 60, 20,5, BLUE);//BOTTONE 
      tft.drawRoundRect(170, 130, 60, 20,5, WHITE);//BOTTONE 
      tft.setTextColor(WHITE); 
      tft.setTextSize(2);    
      tft.setCursor(184,133);
      tft.print("BFO"); 
      bfo=0;
break;     
    } 
} 
/////////////////////////////////IMPOSTA MEDIA FREQUENZA IN RX
void MediaFrq(){  
      mdfrq++;
      if (mdfrq == 1) {
      freqSi = 0;
      inviapll();
   
      tft.fillRoundRect(90, 130, 60, 20,5, BLUE);//BOTTONE 
      tft.drawRoundRect(90, 130, 60, 20,5, WHITE);//BOTTONE 
      tft.setTextColor(WHITE);
      tft.setTextSize(2);
      tft.setCursor(100,133);
      tft.print("000");
     

    }   
   if (mdfrq == 2) {
      freqSi = 455000;
      inviapll();
      tft.fillRoundRect(90, 130, 60, 20,5, BLUE);//BOTTONE 
      tft.drawRoundRect(90, 130, 60, 20,5, WHITE);//BOTTONE 
      tft.setTextColor(WHITE);
      tft.setTextSize(2);
      tft.setCursor(100,133);
      tft.print("455");
      
      passo = 1000;
    }   
   if (mdfrq == 3) {
      freqSi = 450000;
      inviapll();

      tft.fillRoundRect(90, 130, 60, 20,5, BLUE);//BOTTONE 
      tft.drawRoundRect(90, 130, 60, 20,5, WHITE);//BOTTONE 
      tft.setTextColor(WHITE);
      tft.setTextSize(2);
      tft.setCursor(100,133);
      tft.print("450");
    

    }
       if (mdfrq == 3) {
        mdfrq = 0;
       }
      // Serial.println(mdfrq);
}
/////////////////////BARRA S-METER SUL TFT

void S_Meter() {  
     if (smeterMetro.check() == 1) {   
     float uv, dbuv, s;//uV, db-uV, s-meter
     uv= (VoltageIn);               //(VoltageIn/10)-OR-(VoltageIn*2)--------VALORE INGRESSO ANALOGICO
     uv = 0.1*uv+0.9*uvold;
     uvold = uv;
     dbuv = 50.0*log10(uv);
     tft.fillRect(5, 103, 132,3,BLUE);
     if (dbuv>1){
     tft.fillRect(5, 103,abs(dbuv),3,YELLOW );
    }
 }
}

///////////////////////////R-TX PLL CHANGE VALUE & SI5351 CLK OUTPUT
void trasmetti(){
      tft.fillRoundRect(250, 130, 60, 20,5, RED);//BOTTONE 
      tft.drawRoundRect(250, 130, 60, 20,5, WHITE);//BOTTONE 
      tft.setTextColor(WHITE);  
      tft.setTextSize(2);    
      tft.setCursor(265,133);
      tft.print("RX");  
    freqPLL = (freq);
      si5351.output_enable(SI5351_CLK1, 0);///////////////tx
      si5351.output_enable(SI5351_CLK0, 1);
      si5351.set_freq ((freqPLL * 100), SI5351_CLK0);  
      digitalWrite(5, HIGH); 
      delay(100);
      Serial.println(freqPLL); 
  }
void ricevi(){
      tft.fillRoundRect(250, 130, 60, 20,5, BLUE);//BOTTONE 
      tft.drawRoundRect(250, 130, 60, 20,5, WHITE);//BOTTONE 
      tft.setTextColor(WHITE);     
      tft.setTextSize(2); 
      tft.setCursor(265,133);
      tft.print("RX");  
   
      freqPLL = (freq - freqSi);
      si5351.output_enable(SI5351_CLK1, 0);///////////////rx
      si5351.output_enable(SI5351_CLK0, 1);
      si5351.set_freq ((freqPLL * 100), SI5351_CLK0);
      digitalWrite(5, LOW);
      delay(100); 
      Serial.println(freqPLL);
    
     }
     ///////////////////////////////////GRAFICA TFT COME SI PRESENTA ALL'ACCENSIONE
void grafica(){
  ///smeter
      tft.setTextSize(1);
      tft.setTextColor(WHITE);
      tft.setCursor(pos_x_smeter - 5, pos_y_smeter);
      tft.print("S");
      tft.setCursor(pos_x_smeter + 8, pos_y_smeter);
      tft.print("1");
      tft.setCursor(pos_x_smeter + 28, pos_y_smeter);
      tft.print("3");
      tft.setCursor(pos_x_smeter + 48, pos_y_smeter);
      tft.print("5");
      tft.setCursor(pos_x_smeter + 68, pos_y_smeter);
      tft.print("7");
      tft.setCursor(pos_x_smeter + 88, pos_y_smeter);
      tft.print("9");
      tft.setCursor(pos_x_smeter + 125, pos_y_smeter);
      tft.print("+20dB");

      
      tft.setCursor(2, 232);
      tft.setTextColor(WHITE);
      tft.setTextSize(1);
      tft.print("File:VFO_ARDUINO_NANO_TFT_DISPLAY_REV_1"); // VERSIONE DEL SOFTWARE CARICATO
      
      tft.fillRoundRect(2, 155, 310, 60,5,ILI9341_RED); 
      tft.setCursor(40,162); 
      tft.setTextColor(YELLOW);
      tft.setTextSize(3);
      tft.print("SWL I 4554/VE");///QUI IL VOSTRO CALL

      tft.setCursor(7,190); 
      tft.setTextColor(YELLOW);
      tft.setTextSize(2);
      tft.print("Arduino Nano & si5351 PLL");

      tft.setCursor(60, 89);
      tft.setTextColor(WHITE);
      tft.setTextSize(1);
      tft.print("RX-SIGNAL");
      
      tft.setCursor(157,22); 
      tft.setTextColor(ILI9341_GREEN);
      tft.setTextSize(2);
      tft.print("FREQUENCY");
     
      tft.fillRoundRect(1, 10, 99, 70,5,ILI9341_RED); //CHANNEL CORNICE
      tft.drawRoundRect(1, 10, 99, 70,5,ILI9341_WHITE); //CHANNEL CORNICE
      tft.setCursor(24,22); 
      tft.setTextColor(ILI9341_GREEN);
      tft.setTextSize(2);
      tft.print("BAND");
      
      tft.setCursor(18,50); 
      tft.setTextColor(WHITE);
      tft.setTextSize(2);
      tft.print("31 Mt."); 
     
      tft.drawRoundRect(103, 10, 212, 70,5,ILI9341_WHITE); //CORNICEFREQ
      tft.setTextSize(2);
      tft.fillRoundRect(10, 130, 60, 20,5, BLUE);//BOTTONE 
      tft.drawRoundRect(10, 130, 60, 20,5, WHITE);//BOTTONE 
      
      tft.setTextColor(WHITE);
      tft.setCursor(20,133);
      tft.print("STEP");

      tft.fillRoundRect(90, 130, 60, 20,5, BLUE);//BOTTONE 
      tft.drawRoundRect(90, 130, 60, 20,5, WHITE);//BOTTONE 
      tft.setTextColor(WHITE);
      tft.setCursor(100,133);
      tft.print("455");

      tft.fillRoundRect(170, 130, 60, 20,5, BLUE);//BOTTONE 
      tft.drawRoundRect(170, 130, 60, 20,5, WHITE);//BOTTONE 
      tft.setTextColor(WHITE);     
      tft.setCursor(184,133);
      tft.print("BFO");

      tft.fillRoundRect(250, 130, 60, 20,5, BLUE);//BOTTONE 
      tft.drawRoundRect(250, 130, 60, 20,5, WHITE);//BOTTONE 
      tft.setTextColor(WHITE);      
      tft.setCursor(265,133);
      tft.print("RX");
}
