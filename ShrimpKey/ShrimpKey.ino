/*
 ************************************************
 ************ ShrimpKey rev. 2a *****************
 ************************************************
 
 ////////////////////////////////////////////////
 /////////////HOW TO EDIT THE KEYS //////////////
 ////////////////////////////////////////////////
 - Edit keys in the settings.h file
 - That file should be open in a tab above (in Arduino IDE)
 
 There is no need to edit anything below...
 
 ////////////////////////////////////////////////
 /////////// ShrimpKey FIRMWARE /////////////////
 ////////////////////////////////////////////////
 Original version developed by
 Sjoerd Dirk Meijer, info@fromScratchEd.nl
 
 This enhanced version developed by
 Sjoerd Dirk Meijer, info@fromScratchEd.nl
 and Cefn Hoile, shrimping.it@cefn.com
 
 With contributions from and with many thanks to
 Stephan Baerwolf, stephan@matrixstorm.com
 
 Derived from MakeyMakey Firmware v.1.4.1
 by Eric Rosenbaum, Jay Silver, and Jim Lindblom
 and the vusb-for-arduino UsbKeyboard demo
 http://www.practicalarduino.com/projects/virtual-usb-keyboard

 Copyright (C) 2013-2014  Sjoerd Dirk Meijer
 
 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include <UsbKeyboard.h>

/////////////////////////
// DEBUG DEFINITIONS ////               
/////////////////////////

//#define DEBUG  //when using DEBUG, don't use pin D0 and D1 (RX & TX)!!!
#ifdef DEBUG
#define DEBUG_CHANGE
#define DEBUG_TIMING
#define DEBUG_MOUSE
#endif

////////////////////////
// DEFINED CONSTANTS////
////////////////////////
#define BUFFER_LENGTH    3     // 3 bytes gives us 24 samples
//#define TARGET_LOOP_TIME 694   // (1/60 seconds) / 24 samples = 694 microseconds per sample 
//#define TARGET_LOOP_TIME 758  // (1/55 seconds) / 24 samples = 758 microseconds per sample 
#define TARGET_LOOP_TIME 744  // (1/56 seconds) / 24 samples = 744 microseconds per sample 

// id numbers for mouse movement inputs (used in settings.h)
#define MOUSE_MOVE_UP       -1 
#define MOUSE_MOVE_DOWN     -2
#define MOUSE_MOVE_LEFT     -3
#define MOUSE_MOVE_RIGHT    -4
#define MOUSE_RIGHT         -6
#define MOUSE_LEFT          -7

#include "settings.h"

/////////////////////////
// STRUCT ///////////////
/////////////////////////
typedef struct {
  byte pinNumber;
  int keyCode;
  byte measurementBuffer[BUFFER_LENGTH]; 
  boolean oldestMeasurement;
  byte bufferSum;
  boolean pressed;
  boolean prevPressed;
  boolean isMouseMotion;
  boolean isMouseButton;
  boolean isKey;
  boolean isMod;
} 

ShrimpKeyInput;
ShrimpKeyInput inputs[NUM_INPUTS];

///////////////////////////////////
// VARIABLES //////////////////////
///////////////////////////////////
int bufferIndex = 0;
byte byteCounter = 0;
byte bitCounter = 0;
int mouseMovementCounter = 0; // for sending mouse movement events at a slower interval
int pressThreshold;
int releaseThreshold;
boolean inputChanged;
int lastKeyPressed = -1;
int keysPressed = 0;
boolean keyPressed = 0;
int mouseHoldCount[NUM_INPUTS]; // used to store mouse movement hold data
const int ledPin = 13;
int modifier = 0;
int lastModifier = 0;

// timing
unsigned long loopTime = 0;
unsigned long prevTime = 0;
int loopCounter = 0;

///////////////////////////
// FUNCTIONS //////////////
///////////////////////////
void initializeArduino();
void initializeInputs();
void updateMeasurementBuffers();
void updateBufferSums();
void updateBufferIndex();
void updateInputStates();
void sendMouseButtonEvents();
void sendMouseMovementEvents();
void addDelay();

//////////////////////
// SETUP /////////////
//////////////////////
void setup() 
{
  //V-USB
  // Disable timer0 since it can mess with the USB timing. Note that
  // this means some functions such as delay() will no longer work.

#if defined (__AVR_ATmega8__) || defined (__AVR_ATmega8A__) || defined (__AVR_ATmega8HVA__) 
  TIMSK&=!(1<<TOIE0); 
#elif defined (__AVR_ATmega328__) || defined (__AVR_ATmega328P__) 
  TIMSK0&=!(1<<TOIE0); 
#else 
#error unknown AVR 
#endif 

  startupLED();
  initializeArduino();
  initializeInputs();
}

////////////////////
// MAIN LOOP ///////
////////////////////
void loop() 
{
  updateMeasurementBuffers();
  updateBufferSums();
  updateBufferIndex();
  updateInputStates();
  sendMouseButtonEvents();
  sendMouseMovementEvents();
  addDelay();
}

////////////////////
// V-USB  //////////
////////////////////
/*
 * Define our own delay function so that we don't have to rely on
 * operation of timer0, the interrupt used by the internal delay()
 */
void delayMs(unsigned int ms)
{
  for (int i = 0; i < ms; i++) {
    delayMicroseconds(1000);
  }
}

//////////////////////////
// LED
//////////////////////////
void startupLED() {
#ifdef EXTRA_LED    //use pin13 unless EXTRA_LED is defined
  ledPin = extraLedPin;
#endif

  pinMode(ledPin, OUTPUT);
  for (int i=0;i<2;i++){
    delayMs(100);
    digitalWrite(ledPin, HIGH);
    delayMs(100);
    digitalWrite(ledPin, LOW);
  }
  delayMs(100);
  digitalWrite(ledPin, HIGH);
  delayMs(500);
  digitalWrite(ledPin, LOW); 
}

//////////////////////////
// INITIALIZE ARDUINO
//////////////////////////
void initializeArduino() {
#ifdef DEBUG
  Serial.begin(9600);  // Serial for debugging
#endif

  /* Set up input pins 
   DEactivate the internal pull-ups, since we're using external resistors */
  for (int i=0; i<NUM_INPUTS; i++)
  {
    pinMode(pinNumbers[i], INPUT);
    digitalWrite(pinNumbers[i], LOW);
  }

#ifdef DEBUG
  delayMs(4000); // allow us time to reprogram in case things are freaking out
#endif
}

///////////////////////////
// INITIALIZE INPUTS
///////////////////////////
void initializeInputs() {

  float thresholdPerc = SWITCH_THRESHOLD_OFFSET_PERC;
  float thresholdCenterBias = SWITCH_THRESHOLD_CENTER_BIAS/50.0;
  float pressThresholdAmount = (BUFFER_LENGTH * 8) * (thresholdPerc / 100.0);
  float thresholdCenter = ( (BUFFER_LENGTH * 8) / 2.0 ) * (thresholdCenterBias);
  pressThreshold = int(thresholdCenter + pressThresholdAmount);
  releaseThreshold = int(thresholdCenter - pressThresholdAmount);

#ifdef DEBUG
  Serial.println(pressThreshold);
  Serial.println(releaseThreshold);
#endif

  for (int i=0; i<NUM_INPUTS; i++) {
    inputs[i].pinNumber = pinNumbers[i];
    inputs[i].keyCode = keyCodes[i];

    for (int j=0; j<BUFFER_LENGTH; j++) {
      inputs[i].measurementBuffer[j] = 0;
    }
    inputs[i].oldestMeasurement = 0;
    inputs[i].bufferSum = 0;

    inputs[i].pressed = false;
    inputs[i].prevPressed = false;

    inputs[i].isMouseMotion = false;
    inputs[i].isMouseButton = false;
    inputs[i].isKey = false;

    if ((inputs[i].keyCode == MOD_CONTROL_LEFT) or
     (inputs[i].keyCode == MOD_SHIFT_LEFT) or
     (inputs[i].keyCode == MOD_ALT_LEFT) or
     (inputs[i].keyCode == MOD_GUI_LEFT) or
     (inputs[i].keyCode == MOD_CONTROL_RIGHT) or
     (inputs[i].keyCode == MOD_SHIFT_RIGHT) or
     (inputs[i].keyCode == MOD_ALT_RIGHT) or
     (inputs[i].keyCode == MOD_GUI_RIGHT)) {
      inputs[i].isMod = true;
    } else if (inputs[i].keyCode < -5) {
      inputs[i].isMouseButton = true;
    } 
    else if (inputs[i].keyCode < 0) {
      inputs[i].isMouseMotion = true;
    } 
    else {
      inputs[i].isKey = true;
    }
#ifdef DEBUG
    Serial.println(i);
#endif
  }
}

//////////////////////////////
// UPDATE MEASUREMENT BUFFERS
//////////////////////////////
void updateMeasurementBuffers() {
  for (int i=0; i<NUM_INPUTS; i++) {
    // store the oldest measurement, which is the one at the current index,
    // before we update it to the new one 
    // we use oldest measurement in updateBufferSums
    byte currentByte = inputs[i].measurementBuffer[byteCounter];
    inputs[i].oldestMeasurement = (currentByte >> bitCounter) & 0x01; 

    // make the new measurement
    boolean newMeasurement = digitalRead(inputs[i].pinNumber);

    // invert so that true means the switch is closed
    newMeasurement = !newMeasurement; 

    // store it    
    if (newMeasurement) {
      currentByte |= (1<<bitCounter);
    } 
    else {
      currentByte &= ~(1<<bitCounter);
    }
    inputs[i].measurementBuffer[byteCounter] = currentByte;
  }
}

///////////////////////////
// UPDATE BUFFER SUMS
///////////////////////////
void updateBufferSums() {
  // the bufferSum is a running tally of the entire measurementBuffer
  // add the new measurement and subtract the old one

  for (int i=0; i<NUM_INPUTS; i++) {
    byte currentByte = inputs[i].measurementBuffer[byteCounter];
    boolean currentMeasurement = (currentByte >> bitCounter) & 0x01; 
    if (currentMeasurement) {
      inputs[i].bufferSum++;
    }
    if (inputs[i].oldestMeasurement) {
      inputs[i].bufferSum--;
    }
  }  
}

///////////////////////////
// UPDATE BUFFER INDEX
///////////////////////////
void updateBufferIndex() {
  bitCounter++;
  if (bitCounter == 8) {
    bitCounter = 0;
    byteCounter++;
    if (byteCounter == BUFFER_LENGTH) {
      byteCounter = 0;
    }
  }
}

///////////////////////////
// UPDATE INPUT STATES
///////////////////////////
void updateInputStates() {
  UsbKeyboard.update();
  inputChanged = false;
  lastModifier = modifier; //store previous modifier
  for (int i=0; i<NUM_INPUTS; i++) {    
    inputs[i].prevPressed = inputs[i].pressed; // store previous pressed state (only used for mouse buttons)
    if (inputs[i].pressed) {
      if (inputs[i].bufferSum < releaseThreshold) {
        inputChanged = true;
        inputs[i].pressed = false;
        if (inputs[i].isKey) {
          UsbKeyboard.releaseKeyStroke();
          keysPressed = 0;
        }
        if (inputs[i].isMouseMotion) {  
          mouseHoldCount[i] = 0;  // input becomes released, reset mouse hold
          UsbKeyboard.releaseMouse();
        }
        if (inputs[i].isMod) {
          modifier -= keyCodes[i];
        }
      }
      else if (inputs[i].isMouseMotion) {  
        mouseHoldCount[i]++; // input remains pressed, increment mouse hold
      }
      if ((lastKeyPressed != i) && (inputs[i].isKey)) {
        inputs[lastKeyPressed].pressed = false;
        keysPressed = 0;
        inputChanged = false;
        UsbKeyboard.releaseKeyStroke();
      } 
    }
    else if (!inputs[i].pressed) {
      if (inputs[i].bufferSum > pressThreshold) {  // input becomes pressed
        inputChanged = true;
        inputs[i].pressed = true;
        if (inputs[i].isMod) {
          modifier += keyCodes[i];
        }
        if ((lastKeyPressed != i) && (inputs[i].isKey)) {
          keysPressed += 1;
          inputs[lastKeyPressed].pressed = false;
          UsbKeyboard.sendKeyStroke(keyCodes[i], modifier);
          lastKeyPressed = i;
        }
      }
    }
    if ((lastKeyPressed == i) && (inputs[i].isKey) && !(lastModifier == modifier) && (keysPressed > 0)) {
      //Resend keystroke when modifier is changed
      UsbKeyboard.sendKeyStroke(keyCodes[i], modifier);
    }
    if ((keysPressed == 0) && (inputs[i].isKey)) {
      if (inputs[i].pressed) {
        inputs[i].pressed = false;
        lastKeyPressed = -1;
      }
    }
  }
#ifdef DEBUG_CHANGE
  if (inputChanged) {
    Serial.println("change");
  }
#endif
}

/////////////////////////////
// SEND MOUSE BUTTON EVENTS 
/////////////////////////////
void sendMouseButtonEvents() {
  if (inputChanged) {
    for (int i=0; i<NUM_INPUTS; i++) {
      if (inputs[i].isMouseButton) {
        if (inputs[i].pressed) {
          if (inputs[i].keyCode == -7) {
            UsbKeyboard.mouse(0, 0, 1);
          } 
          if (inputs[i].keyCode == -6) {
            UsbKeyboard.mouse(0, 0, 2);
          } 
        } 
        else if (inputs[i].prevPressed) {
          if (inputs[i].keyCode == -7) {
            UsbKeyboard.mouse(0, 0, 0);
          } 
          if (inputs[i].keyCode == -6) {
            UsbKeyboard.mouse(0, 0, 0);
          }           
        }
      }
    }
  }
}

//////////////////////////////
// SEND MOUSE MOVEMENT EVENTS
//////////////////////////////
void sendMouseMovementEvents() {
  byte right = 0;
  byte left = 0;
  byte down = 0;
  byte up = 0;
  byte horizmotion = 0;
  byte vertmotion = 0;

  mouseMovementCounter++;
  mouseMovementCounter %= MOUSE_MOTION_UPDATE_INTERVAL;
  if (mouseMovementCounter == 0) {
   
    for (int i=0; i<NUM_INPUTS; i++) {
#ifdef DEBUG_MOUSE
      Serial.println(inputs[i].isMouseMotion);  
#endif

      if (inputs[i].isMouseMotion) {
        if (inputs[i].pressed) {
          if (inputs[i].keyCode == MOUSE_MOVE_UP) {
            // JL Changes (x4): now update to 1 + a hold factor, constrained between 1 and mouse max movement speed
            up=constrain(1+mouseHoldCount[i]/MOUSE_RAMP_SCALE, 1, MOUSE_MAX_PIXELS);
          }  
          if (inputs[i].keyCode == MOUSE_MOVE_DOWN) {
            down=constrain(1+mouseHoldCount[i]/MOUSE_RAMP_SCALE, 1, MOUSE_MAX_PIXELS);
          }  
          if (inputs[i].keyCode == MOUSE_MOVE_LEFT) {
            left=constrain(1+mouseHoldCount[i]/MOUSE_RAMP_SCALE, 1, MOUSE_MAX_PIXELS);
          }  
          if (inputs[i].keyCode == MOUSE_MOVE_RIGHT) {
            right=constrain(1+mouseHoldCount[i]/MOUSE_RAMP_SCALE, 1, MOUSE_MAX_PIXELS);
          }  
        }
      }
    }

    // left/right cancellation 
    // and diagonal scrolling (does not work at the moment)
    if(left > 0)
    {
      if(right > 0)
      {
        horizmotion = 0; // cancel horizontal motion because left and right are both pushed
      }
      else
      {
        horizmotion = -left; // left yes, right no
      }
    }
    else
    {
      if(right > 0)
      {
        horizmotion = right; // right yes, left no
      }
    }

    if(down > 0)
    {
      if(up > 0)
      {
        vertmotion = 0; // cancel vertical motion because up and down are both pushed
      }
      else
      {
        vertmotion = down; // down yes, up no
      }
    }
    else
    {
      if (up > 0)
      {
        vertmotion = -up; // up yes, down no
      }
    }
    // now move the mouse
    if( !((horizmotion == 0) && (vertmotion==0)) )
    {
      UsbKeyboard.mouse(horizmotion * PIXELS_PER_MOUSE_STEP, vertmotion * PIXELS_PER_MOUSE_STEP, 0);
    }
  }
}

///////////////////////////
// ADD DELAY
///////////////////////////
void addDelay() {

  loopTime = micros() - prevTime;
  if (loopTime < TARGET_LOOP_TIME) {
    int wait = TARGET_LOOP_TIME - loopTime;
    delayMicroseconds(wait);
  }

  prevTime = micros();

#ifdef DEBUG_TIMING
  if (loopCounter == 0) {
    int t = micros()-prevTime;
    Serial.println(t);
  }
  loopCounter++;
  loopCounter %= 999;
#endif
}



