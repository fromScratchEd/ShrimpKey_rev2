#include "Arduino.h"

//////////////////////////////////////////////////////////////////////
//  ShrimpKey settings  //////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

#define NUM_INPUTS       16         // How many pins are used as input; max = 17
                                    // Skip the pins for outputPin and extraLed, but include modifiers
                                    
int pinNumbers[NUM_INPUTS] = {      // Pin declarations: which pins are used. ALWAYS skip pins 2, 7 and 13
                                    // Skip the pins for outputPin and extraLed, but include modifiers
                                    // Available pins: 0, 1, 3, 4, 5, 6, 8, 9, 10, 11, 12, 14, 15, 16, 17, 18, 19
  0, 1, 3, 4, 5, 8, 9, 10, 11, 12, 14, 15, 16, 17, 18, 19
};

#define NUM_MODS          1         // How many pins (of NUM_INPUTS) are used as modifier (Shift, Ctrl, Alt, etc.)

int modPinNumbers[NUM_MODS] = {     // When using modifiers, ALSO declare those pins here (separated by commas)
  12
};

char keyCodes[NUM_INPUTS] = {    // Keymappings: which key maps to which pin on the ShrimpKey-board?
                                 // Change the keys here. When using lesser pins, comment or delete lines
                                 // See UsbKeyboard.h for possible values
                                 // The number of lines should be the same as the value of NUM_INPUTS
  KEY_A                   // pin D0
  , KEY_S                 // pin D1
  , KEY_D                 // pin D3
  , KEY_W                 // pin D4
  , KEY_ARROW_LEFT        // pin D5
  //, KEY_ENTER           // pin D6
  , KEY_ARROW_UP          // pin D8
  , MOUSE_LEFT            // pin D9
  , KEY_SPACE             // pin D10
  , KEY_ARROW_DOWN        // pin D11
  , KEY_ARROW_RIGHT       // pin D12
  , MOUSE_RIGHT           // pin A0 = 14
  , MOUSE_MOVE_UP         // pin A1 = 15
  , MOUSE_MOVE_DOWN       // pin A2 = 16
  , MOUSE_MOVE_LEFT       // pin A3 = 17
  , MOUSE_MOVE_RIGHT      // pin A4 = 18
  , MOD_SHIFT_LEFT        // pin A5 = 19
};


/////////////////////////
// EXTRA OPTIONS ////////
/////////////////////////

//#define EXTRA_LED    //uncomment if you want to use an extra LED on pin 12 (or any other pin)
#define SIM_KEYPRESS //uncomment when keypresses should be send simultaneous (max.6 at one time), comment when they should be send repeated (1 at a time)
#define OUTPUTPIN    //uncomment to use one or two pins as output

#ifdef EXTRA_LED
  const int extraLedPIN 12;  //pin for extra LED for ShrimpKey; DON'T use this pin in pinNumbers!! NOT WORKING ATM
#endif

#ifdef OUTPUTPIN
  const int num_output = 1;   //how many outputs are used; can be 1 or 2
                              //when using pin D6; remember to disconnect the attached device before uploading new sketch!!
  const int outputPin1 = 6;   //pin for first output (when num_output = 2 => keyboard); DON'T use this pin in pinNumbers!!
  const int outputPin2 = 5;   //pin for second output (when num_output = 2 => mouse); DON'T use this pin in pinNumbers!!
#endif

#ifdef SIM_KEYPRESS
  const int NUM_SIM = 6;  //define number of simultaneous keypresses (size of buffer in UsbKeyboard.h minus 2)
                          //when changing this number, also changes all lines with UsbKeyboard.sendKeyStroke() in ShrimpKey.ino
#endif

/////////////////////////
// MOUSE MOTION /////////
/////////////////////////
#define MOUSE_MOTION_UPDATE_INTERVAL  35   // how many loops to wait between 
                                           // sending mouse motion updates
                                           
#define PIXELS_PER_MOUSE_STEP         4    // a larger number will make the mouse
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

