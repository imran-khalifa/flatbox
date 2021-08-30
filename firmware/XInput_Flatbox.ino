/*
 * Project:     Flatbox XInput
 * Author:      Imran Khalifa
 * Description: Replacement Firmware for Flatbox (https://github.com/jfedor2/flatbox) which
 *              uses the Arduino XInput Library. Plugging the Flatbox into a PC will detect
 *              the device as a XBOX 360 Controller. Compatible with the MAGIC-NS enabling
 *              console support.
 * 
 * Prerequisites:
 * * Arduino XInput USB Core for Arduino AVR (https://github.com/dmadison/ArduinoXInput_AVR)
 * * Arduino Xinput Library (Can be found in the Library Manager [Ctrl + Shift + I])
 * 
 * WARNING:
 * * Only upload this sketch if you know how to reset your Arduino, if you do not you will
 *   no longer be able to upload sketches to your Arduino. This can be done by jumping RST
 *   to ground.
 * 
 * Upload Intructions for Pro Micro:
 * * Under `Tools` -> `Board`,  Select `Xinput AVR Boards` -> `Arduino Leonardo w/ XInput`
 * * Under `Tools` -> `Port`, Select your Arduino (Skip this step, if the Arduino already has 
 *   an XInput sketch uploaded to it)
 * * Press `Upload` to upload the sketch to the Arduino.
 *   * If you are programming an Arduino that has an XInput sketch uploaded to it you will
 *     need to reset the Arduino while the IDE is uploading.
 */
#include <XInput.h>

// Uncomment the appropriate line to match your stick version
#define REV1
//#define REV2

/* Defines pin layout for REV1 */
#ifdef REV1
  #define PIN_UP        9
  #define PIN_DOWN      7
  #define PIN_LEFT      6
  #define PIN_RIGHT     8
  #define PIN_CROSS     10
  #define PIN_CIRCLE    14
  #define PIN_TRIANGLE  15
  #define PIN_SQUARE    16
  #define PIN_L1        21
  #define PIN_L2        20
  #define PIN_R1        19
  #define PIN_R2        18
  #define PIN_SELECT    2
  #define PIN_START     1
  #define PIN_L3        5
  #define PIN_R3        3
  #define PIN_PS        0

  /* Unused Pin */
  #define PIN_UNUSED    4
#endif

/* Defines pin layout for REV2 */
#ifdef REV2
  #define PIN_UP        5
  #define PIN_DOWN      9
  #define PIN_LEFT      8
  #define PIN_RIGHT     10
  #define PIN_CROSS     18
  #define PIN_CIRCLE    21
  #define PIN_TRIANGLE  14
  #define PIN_SQUARE    7
  #define PIN_L1        15
  #define PIN_L2        19
  #define PIN_R1        16
  #define PIN_R2        20
  #define PIN_SELECT    1
  #define PIN_START     4
  #define PIN_L3        3
  #define PIN_R3        0
  #define PIN_PS        6
#endif

/* Structure which stores state of all inputs as booleans */
typedef struct
{
  bool dpad_up;
  bool dpad_down;
  bool dpad_left;
  bool dpad_right;
  
  bool button_a;
  bool button_b;
  bool button_x;
  bool button_y;
  bool button_rb;
  bool button_rt;
  bool button_lb;
  bool button_lt;

  bool button_start;
  bool button_back;
  bool button_guide;

  bool button_l3;
  bool button_r3;
} input_states;

typedef enum
{
  mode_dpad = 0,
  mode_joystick
} dpad_modes;

/* Previous and Current Controller State */
input_states prev;
input_states curr;
int dpad_mode = mode_dpad;

void setup()
{
  /* Setup pins */
  pinMode(PIN_UP, INPUT_PULLUP);
  pinMode(PIN_DOWN, INPUT_PULLUP);
  pinMode(PIN_LEFT, INPUT_PULLUP);
  pinMode(PIN_RIGHT, INPUT_PULLUP);
  pinMode(PIN_CROSS, INPUT_PULLUP);
  pinMode(PIN_CIRCLE, INPUT_PULLUP);
  pinMode(PIN_TRIANGLE, INPUT_PULLUP);
  pinMode(PIN_SQUARE, INPUT_PULLUP);
  pinMode(PIN_L1, INPUT_PULLUP);
  pinMode(PIN_L2, INPUT_PULLUP);
  pinMode(PIN_R1, INPUT_PULLUP);
  pinMode(PIN_R2, INPUT_PULLUP);
  pinMode(PIN_SELECT, INPUT_PULLUP);
  pinMode(PIN_START, INPUT_PULLUP);
  pinMode(PIN_L3, INPUT_PULLUP);
  pinMode(PIN_R3, INPUT_PULLUP);
  pinMode(PIN_PS, INPUT_PULLUP);

  /* Initialise input states */
  memset(&prev, 0, sizeof(input_states));
  memset(&curr, 0, sizeof(input_states));

  /* 
   *  If left button is held when plugging in the controller,
   *  direction buttons will control joystick instead of dpad
   */
  if(digitalRead(PIN_LEFT) == LOW)
    dpad_mode = mode_joystick;

  /* Turn off autosend */
  XInput.setAutoSend(false);
  /* Start XInput */
  XInput.begin();
}

/*
 * Reads pins and sets up input state
 */
void ReadPins()
{
  curr.dpad_up       = (digitalRead(PIN_UP) == LOW);
  curr.dpad_down     = (digitalRead(PIN_DOWN) == LOW);
  curr.dpad_left     = (digitalRead(PIN_LEFT) == LOW);
  curr.dpad_right    = (digitalRead(PIN_RIGHT) == LOW);
  
  curr.button_a      = (digitalRead(PIN_CROSS) == LOW);
  curr.button_b      = (digitalRead(PIN_CIRCLE) == LOW);
  curr.button_y      = (digitalRead(PIN_TRIANGLE) == LOW);
  curr.button_x      = (digitalRead(PIN_SQUARE) == LOW);
  curr.button_lb     = (digitalRead(PIN_L1) == LOW);
  curr.button_lt     = (digitalRead(PIN_L2) == LOW);
  curr.button_rb     = (digitalRead(PIN_R1) == LOW);
  curr.button_rt     = (digitalRead(PIN_R2) == LOW);
  
  curr.button_back   = (digitalRead(PIN_SELECT) == LOW);
  curr.button_start  = (digitalRead(PIN_START) == LOW);
  curr.button_guide  = (digitalRead(PIN_PS) == LOW);
  
  curr.button_l3     = (digitalRead(PIN_L3) == LOW);
  curr.button_r3     = (digitalRead(PIN_R3) == LOW);
}

/*
 * Parses state and sends input to PC
 */
void ParseInput()
{
  /* Only do something if state has changed */
  if(memcmp(&prev, &curr, sizeof(input_states)))
  {
    /* Handle Directional Pad      */
    if (curr.dpad_up ^ prev.dpad_up ||
        curr.dpad_down ^ prev.dpad_down || 
        curr.dpad_left ^ prev.dpad_left ||
        curr.dpad_right ^ prev.dpad_right)
        dpad_mode ? XInput.setJoystick(JOY_LEFT, curr.dpad_up, 
                               curr.dpad_down, 
                               curr.dpad_left, 
                               curr.dpad_right, 
                               true)
                  : XInput.setDpad(curr.dpad_up, 
                                   curr.dpad_down, 
                                   curr.dpad_left, 
                                   curr.dpad_right, 
                                   true);

    /* Handle Buttons */
    if (curr.button_start ^ prev.button_start)
      XInput.setButton(BUTTON_START, curr.button_start);
    if (curr.button_back ^ prev.button_back)
      XInput.setButton(BUTTON_BACK, curr.button_back);
    if (curr.button_guide ^ prev.button_guide)
      XInput.setButton(BUTTON_LOGO, curr.button_guide); 
    if (curr.button_a ^ prev.button_a)
      XInput.setButton(BUTTON_A, curr.button_a);      
    if (curr.button_b ^ prev.button_b)
      XInput.setButton(BUTTON_B, curr.button_b);   
    if (curr.button_x ^ prev.button_x)
      XInput.setButton(BUTTON_X, curr.button_x);
    if (curr.button_y ^ prev.button_y)
      XInput.setButton(BUTTON_Y, curr.button_y);
    if (curr.button_lb ^ prev.button_lb)
      XInput.setButton(BUTTON_LB, curr.button_lb);
    if (curr.button_rb ^ prev.button_rb)
      XInput.setButton(BUTTON_RB, curr.button_rb);
    if (curr.button_l3 ^ prev.button_l3)
      XInput.setButton(BUTTON_L3, curr.button_l3);
    if (curr.button_r3 ^ prev.button_r3)
      XInput.setButton(BUTTON_R3, curr.button_r3);

    /* Handle Triggers */
    if (curr.button_lt ^ prev.button_lt)
      XInput.setButton(TRIGGER_LEFT, curr.button_lt);
    if (curr.button_rt ^ prev.button_rt)
      XInput.setButton(TRIGGER_RIGHT, curr.button_rt);

    /* Send inputs to PC */
    XInput.send();
    /* Set new previous state */
    memcpy(&prev, &curr, sizeof(input_states));
  }
}

void loop()
{
  ReadPins();
  ParseInput();
}
