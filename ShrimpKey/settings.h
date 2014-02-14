#include "Arduino.h"

//////////////////////////////////////////////////////////////////////
//  ShrimpKey settings  //////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

//#define EXTRA_LED  //uncomment if you want to use an extra LED on pin 12 (or any other pin)
#ifdef EXTRA_LED
  int extraLedPIN 12;
#endif

//#define SIM_KEYPRESS //uncomment when keypresses should be send simultaneous (max.6 at one time), comment when they should be send repeated (1 at a time)
#ifdef SIM_KEYPRESS
  const int NUM_SIM = 6;  //define number of simultaneous keypresses (size of buffer in UsbKeyboard.h minus 2)
                          //when changing this number, also changes all lines with UsbKeyboard.sendKeyStroke() in ShrimpKey.ino
#endif
// Pin Numbers
#define NUM_INPUTS       16     // how many pins are used, skipping pin 2, 7 and pin 13 (and 6)

int pinNumbers[NUM_INPUTS] = {      // Pin declarations: which pins will be used?
  0, 1, 3, 4, 5, 8, 9, 10, 11, 12, 14, 15, 16, 17, 18, 19
};

char keyCodes[NUM_INPUTS] = {      // Keymappings: which key maps to which pin on the ShrimpKey-board?
  MOD_SHIFT_LEFT,              // pin D0
  MOD_CONTROL_LEFT,              // pin D1
  KEY_S,              // pin D3
  KEY_D,              // pin D4
  KEY_F,              // pin D5
  KEY_G,              // pin D8
  KEY_I,              // pin D9
  KEY_J,              // pin D10
  KEY_K,              // pin D11
  KEY_L,              // pin D12
  MOUSE_LEFT,         // pin A0 = 14
  MOUSE_RIGHT,        // pin A1 = 15
  MOUSE_MOVE_LEFT,    // pin A2 = 16
  MOUSE_MOVE_RIGHT,   // pin A3 = 17
  MOUSE_MOVE_UP,      // pin A4 = 18
  MOUSE_MOVE_DOWN     // pin A5 = 19
};


/////////////////////////
// MOUSE MOTION /////////
/////////////////////////
#define MOUSE_MOTION_UPDATE_INTERVAL  35   // how many loops to wait between 
                                           // sending mouse motion updates
                                           
#define PIXELS_PER_MOUSE_STEP         4     // a larger number will make the mouse
                                           // move faster

#define MOUSE_RAMP_SCALE              150  // Scaling factor for mouse movement ramping
                                           // Lower = more sensitive mouse movement
                                           // Higher = slower ramping of speed
                                           // 0 = Ramping off
                                            
#define MOUSE_MAX_PIXELS              10   // Max pixels per step for mouse movement



///////////////////////////
// NOISE CANCELLATION /////
///////////////////////////
#define SWITCH_THRESHOLD_OFFSET_PERC  5    // number between 1 and 49
                                           // larger value protects better against noise oscillations, but makes it harder to press and release
                                           // recommended values are between 2 and 20
                                           // default value is 5

#define SWITCH_THRESHOLD_CENTER_BIAS 70   // number between 1 and 99
                                          // larger value makes it easier to "release" keys, but harder to "press"
                                          // smaller value makes it easier to "press" keys, but harder to "release"
                                          // recommended values are between 30 and 70
                                          // 50 is "middle" 2.5 volt center
                                          // default value is 55
                                          // 100 = 5V (never use this high)
                                          // 0 = 0 V (never use this low

