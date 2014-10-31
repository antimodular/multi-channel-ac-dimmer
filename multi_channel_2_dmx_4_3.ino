//http://www.alfadex.com/dimming-230v-ac-with-arduino-2/

//http://forum.pjrc.com/threads/25231-ac-dimming-via-inMojo-AC-Dimmer-Module-Lite?p=43143&viewfull=1#post43143/


// 1hz = 1000 ms per periode
// 1000 / 40 ms = 25 hz
// 1000 / 40 mircos = 25khz



#include <DmxReceiver.h>
DmxReceiver DMX2 = DmxReceiver();

//boolean bDebug = true;
//boolean bDebug = false; //

//#define DEBUG

#define NUM_LEDS 22
#define NUM_LEDS_USED 22

//---------------------------------------DMX-------------------------------------

volatile unsigned char dmx_data[NUM_LEDS];
//volatile unsigned char dmx_data_old[DMX_NUM_CHANNELS];
volatile unsigned char data;
int dmxStartAddress = 1; //30; //15;

unsigned long last_dmx_update;
unsigned long dura;


//---------------------------------------DIMMER-------------------------------------

#define timer0_duration 30 //40 //40

int zeroCrossPin = 2;

//int AC_LOAD[26] = {
//  1,3,4,5,6,7,8,9,10,11,12,13,24,25,26,27,28,29,30,31,32,33,17,16,15,14};    // Output to Opto Triac pin

int AC_LOAD[NUM_LEDS] = {
  25,24, 12,11,10,9,8,7,5,4,3, 26,27,28,29,30,31,32,  14, 15, 16, 17 }; //20,21,13,14,15,16,17};    // Output to Opto Triac pin
//
//int AC_LOAD[NUM_LEDS] = {
//  3,4,5,7,8,9,10,11,12, 25,24, 26,27,28,29,30,31,32,  14, 15, 16, 17 }; //20,21,13,14,15,16,17};    // Output to Opto Triac pin



elapsedMicros  sinceInterrupt;
unsigned long thePeriod = 0;

volatile unsigned int timerFire_cnt = 0;

int fadeDirs[NUM_LEDS];
int fadeValues[NUM_LEDS];
int fadeSteps[NUM_LEDS];

int maxVal = 255; //200; //250 -> in australia
int minVal = 0;

unsigned long fadeTimer;

IntervalTimer timer0;

//--------------------------------------dip switch--------------------------------
byte myGroupID = 0; //default will be changed once DIP switch is read
int startAddress;

boolean initDone = false;
unsigned long initTimer;

unsigned long pulse;

int ledPin = 13;
boolean ledState = 0;

int buttonPin1 = A12;
int buttonPin2 = A13;

void setup(){


#ifdef DEBUG
  Serial.begin(9600);
#endif

  pinMode(zeroCrossPin, INPUT);
  attachInterrupt(zeroCrossPin, zero_crosss_int, RISING);  // Choose the zero cross interrupt # from the table above


  timer0.begin(timerFire, timer0_duration);

  setup_dipSwitch();

  for(int i=0; i <NUM_LEDS;i++){
    pinMode(AC_LOAD[i], OUTPUT);	      // Set the AC Load as output

  }
  pinMode(ledPin, OUTPUT);

  // pinMode(buttonPin1, INPUT_PULLUP);
  // pinMode(buttonPin2, INPUT_PULLUP);

  //  randomSeed(analogRead(13));

  for(int i=0; i<NUM_LEDS; i++){
    fadeValues[i] = 255; // 255 = lights off at start-up

    int r = random(0,2);
    r = (r * 2) - 1;
    fadeDirs[i] = r;
    // Serial.println(r);
    fadeSteps[i] = random(1,5);//20);
  }

  DMX2.begin();

  initDone = false;
  initTimer = millis();
  pulse = millis();
}


void zero_crosss_int(){
  // Ignore spuriously short interrupts
  if (sinceInterrupt < 2000){
#ifdef DEBUG
    Serial.println("sinceInterrupt < 2000 ");
#endif

    return;
  }

  thePeriod = sinceInterrupt; //ca. 10012 at 50Hz location, 8300 at 60 hz in montreal

  sinceInterrupt = 0;

#ifdef DEBUG
  Serial.print("timerFire_cnt ");
  Serial.println(timerFire_cnt);
  timerFire_cnt = 0;
#endif
  // Serial.print("----------thePeriod----------");
  // Serial.println(thePeriod);

  for(int i=0; i<NUM_LEDS_USED;i++){
    digitalWrite(AC_LOAD[i], LOW);
  }
}


void timerFire(void){

  /*
  if(bDebug){
   Serial.print(timerFire_cnt);
   Serial.print(" ");
   Serial.print(fadeValues[0]);
   Serial.println();
   }
   */
  //  if(timerFire_cnt > maxVal-10){
  //    //at the end / near-the-end of 1/2 sinus curve turn pin off that triggers triac
  //
  //    for(int i=0; i<NUM_LEDS;i++){
  //     // digitalWrite(AC_LOAD[i], LOW);
  //    }
  //  } 
  //  else{
  //int temp_sinceInterrupt = sinceInterrupt;
  for(int i=0; i<NUM_LEDS_USED;i++){
    if(sinceInterrupt >= fadeValues[i] ){
      // if(temp_sinceInterrupt >= fadeValues[i] ){
      //for first part of 1/2 sinus curve triac is off, then we turn triac on by setting pin high
      //triac automatically turns on next zero crossing
      //but not arduino pin
      digitalWrite(AC_LOAD[i], HIGH);

    }

  }//end for
  // }

 // timerFire_cnt++;


}


void loop(){
  //    for(int i=0; i<NUM_LEDS; i++){
  //         Serial.println(fadeDirs[i]);
  //   }

  dmxFading();



#ifdef DEBUG
  if(millis() - pulse > 300){
    digitalWrite(ledPin, ledState);
    ledState = !ledState;

    pulse = millis();
    //  Serial.println("++++++++++++++++++++++alive");
  }
#endif
}

void dmxFading(){
  DMX2.bufferService();
  // Serial.println("DMX2.bufferService");
  if (DMX2.newFrame()) {

    //   Serial.println("DMX2.newFrame");

    dura = millis()- last_dmx_update;
    last_dmx_update = millis();
    for(int i=0; i<NUM_LEDS; i++){
      // dmx_data_old[i] = dmx_data[i];
      dmx_data[i] = DMX2.getDimmer(dmxStartAddress + i+1);

      // fadeValues[i] = map(dmx_data[i], 0, 255, maxVal, minVal);

      fadeValues[i] = map(dmx_data[i], 0, 255, 8333, 0);
      // fadeValues[i] = map(dmx_data[i], 0, 255, 0, 8333);
      // int temp = map(sinceInterrupt,0,8333,0,255);
      //    temp = constrain(temp,0,255);

      //    Serial.println(fadeValues[i]);

      //fadeValues[i] = dmx_data[i] 
    }

  } 
}

void autoFading(){
  if(millis() - fadeTimer > 20){
    fadeTimer = millis();

    for(int i=0; i<NUM_LEDS; i++){
      fadeValues[i] += (fadeDirs[i]*fadeSteps[i]);
      // if( i == 0) Serial.println(fadeValues[i]);

      if(fadeValues[i] >= maxVal){
        //  if( i == 0) Serial.println(">");
        fadeDirs[i] = -1;
        fadeValues[i] = maxVal;
      }
      if(fadeValues[i] <= minVal){
        //  if( i == 0) Serial.println("<");
        fadeDirs[i] = 1;
        fadeValues[i] = minVal;
      }

    }

  }
}





























