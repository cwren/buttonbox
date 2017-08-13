#include <G35.h>
#include <G35String.h>

#define LIGHT_COUNT (50)
#define GRID_COUNT (48)
#define G35_PIN (0)

#define DIM (1)
#define BRIGHT (255)
#define HOLD_MILLIS (5000)
#define FADE_MILLIS (1000)

#define MODE_OFF (0)
#define MODE_ON (1)
#define MODE_READ (2)
#define MODE_RAINBOW (3)
#define MODE_NIGHTLIGHT (4)
#define MODE_STORM (5)
#define MODE_BLUE (6)
 
#define SQUID (0)
#define BUUTT (1)
#define TRAIL (2)
#define MOUSE (3)

G35String lights(G35_PIN, LIGHT_COUNT);

static int32_t c;
static unsigned long press_time = 0;
static int sequence[] = { -1, -1, -1, -1 };
static int button[] = { A0, A1, A2, A3 };
static int lamp[] = { 3, 9, 10, 11 };
static int numButtons = 4;
static int mode = MODE_BLUE;
static int PATTERNS[][4] = { 
  { SQUID, BUUTT, TRAIL, MOUSE },  //  0  A B C D 
  { SQUID, BUUTT, MOUSE, TRAIL },  //  1
  { SQUID, TRAIL, BUUTT, MOUSE },  //  2
  { SQUID, TRAIL, MOUSE, BUUTT },  //  3
  { SQUID, MOUSE, BUUTT, TRAIL },  //  4
  { SQUID, MOUSE, TRAIL, BUUTT },  //  5
  { BUUTT, SQUID, TRAIL, MOUSE },  //  6
  { BUUTT, SQUID, MOUSE, TRAIL },  //  7
  { BUUTT, TRAIL, SQUID, MOUSE },  //  8
  { BUUTT, TRAIL, MOUSE, SQUID },  //  9
  { BUUTT, MOUSE, SQUID, TRAIL },  // 10
  { BUUTT, MOUSE, TRAIL, SQUID },  // 11
  { TRAIL, SQUID, BUUTT, MOUSE },  // 12
  { TRAIL, SQUID, MOUSE, BUUTT },  // 13
  { TRAIL, BUUTT, SQUID, MOUSE },  // 14
  { TRAIL, BUUTT, MOUSE, SQUID },  // 15
  { TRAIL, MOUSE, SQUID, BUUTT },  // 16
  { TRAIL, MOUSE, BUUTT, SQUID },  // 17
  { MOUSE, SQUID, BUUTT, TRAIL },  // 18
  { MOUSE, SQUID, TRAIL, BUUTT },  // 19
  { MOUSE, BUUTT, SQUID, TRAIL },  // 20
  { MOUSE, BUUTT, TRAIL, SQUID },  // 21
  { MOUSE, TRAIL, SQUID, BUUTT },  // 22
  { MOUSE, TRAIL, BUUTT, SQUID },  // 23  D C B A 
};
static int grid_index[][8] = {
  { 0,  1,  2,  3,  4,  5,  6,  7},
  {15, 14, 13, 12, 11, 10,  9,  8},
  {16, 17, 18, 19, 20, 21, 22, 23},
  {31, 30, 29, 28, 27, 26, 25, 24},
  {32, 33, 34, 35, 36, 37, 38, 39},
  {47, 46, 45, 44, 43, 42, 41, 40},
};

void setup() {
  c = 0;
  for (int i = 0; i < numButtons; i++) {
    pinMode(button[i], INPUT);
    digitalWrite(button[i], HIGH);
  }
  clear_sequence();
  lights.enumerate();  //Enumerate lights on string to enable individual bulb addressing
  alloff();
}

void loop() {
  unsigned long now = millis();
  for (int i = 0; i < numButtons; i++) {
    if (digitalRead(button[i]) == LOW) {
       if(add_button_to_sequence(i)) {
         press_time = millis();
       }
    }
  }
  
  set_lamps();
  
  if (sequence_full()) {
    int new_mode = interpret_sequence(mode);
    if (new_mode != mode) {
      alloff();
    }
    mode = new_mode;
    if ((now - press_time) > FADE_MILLIS) {
      clear_sequence();
    }
  }
  
  if ((now - press_time) > HOLD_MILLIS) {
    clear_sequence();
  }
  
  switch(mode) {
    case MODE_ON:
      allon();
      break;
      
    case MODE_READ:
      reading();
      break;
      
    case MODE_RAINBOW:
      rainbow_grid();
      break;
      
    case MODE_NIGHTLIGHT:
      nightlight();
      break;

    case MODE_STORM:
      storm();
      break;

    case MODE_BLUE:
      blue();
      break;

    case MODE_OFF:
    default: 
      alloff();
      break;
   }
  c += 1;  
}

int interpret_sequence(int mode) {
  // MOUSE, TRAIL, SQUID, BUUTT
  if (sequence_equals(PATTERNS[22])) {
    return MODE_OFF;
  }
  
  // SQUID, BUUTT, TRAIL, MOUSE
  if (sequence_equals(PATTERNS[0])) {
    return MODE_NIGHTLIGHT;
  }
  
  // SQUID, TRAIL, BUUTT, MOUSE
  if (sequence_equals(PATTERNS[2])) {
    return MODE_READ;
  }
  
  // SQUID, BUUTT, MOUSE, TRAIL
  if (sequence_equals(PATTERNS[1])) {
    return MODE_ON;
  }
  
  // MOUSE, TRAIL, BUUTT, SQUID
  if (sequence_equals(PATTERNS[23])) {
    return MODE_RAINBOW;
  }
  
  // MOUSE, BUUTT, TRAIL, SQUID
  if (sequence_equals(PATTERNS[21])) {
    return MODE_STORM;
  }
  
  return mode;
}


boolean sequence_equals(int* that) {
  for (int i = 0; i < numButtons; i++) {
    if (sequence[i] != that[i])
      return false;
  }
  return true;
}

void clear_sequence() {
  for (int i = 0; i < numButtons; i++) {
    sequence[i] = -1;
  }
}

boolean is_button_in_sequence(int button) {
  for (int i = 0; i < numButtons; i++) {
    if (sequence[i] == button)
      return true;
  }
  return false;
}

boolean sequence_full() {
  return sequence[numButtons - 1] != -1;
}

boolean add_button_to_sequence(int button) {
  for (int i = 0; i < numButtons; i++) {
    if (sequence[i] == button) {
      return false;
    }
    if (sequence[i] == -1) {
      sequence[i] = button;
      return true;
    }
  }
  return false;
}

void set_lamps() {
  for (int i = 0; i < numButtons; i++) {
    int fade = DIM;
    if (is_button_in_sequence(i)) {
      fade = BRIGHT;
    }
    analogWrite(lamp[i], fade);
  }
}

void rainbow() { 
  uint16_t step = 2 * HUE_MAX / LIGHT_COUNT;
  for(int i = 0; i < GRID_COUNT; i++) {
    color_t color = lights.color_hue(((i + c) * step) % (HUE_MAX + 1));
    lights.set_color(i, G35::MAX_INTENSITY, color);
  }
}

void rainbow_grid() {
  uint16_t step = HUE_MAX / 8;
  for(int col = 0; col < 8; col++) {
    uint8_t hue = ((col * step) + c) % (HUE_MAX + 1);
    hue = HUE_MAX - hue;
    color_t color = lights.color_hue(hue);
    for(int row = 0; row < 6; row++) {
      lights.set_color(grid_index[row][7-col], G35::MAX_INTENSITY, color);
    }
  }
}

void nightlight() { 
  for(int i = 0; i < LIGHT_COUNT; i++) {
    color_t color = lights.color(0, 0, 0);
    if (i == 49) {
      color = lights.color(255, 255, 255);
    }
    lights.set_color(i, 16, color);
  }
}

void reading() { 
  for(int i = 0; i < LIGHT_COUNT; i++) {
    color_t color = lights.color(0, 0, 0);
    if (i == 0  || i == 1  || i == 2  || i == 15 || i == 14 ||
        i == 13 || i == 16 || i == 17 || i == 18 || i == 31 ||
        i == 30 || i == 29 || i == 32 || i == 33 || i == 34 ||
        i == 47 || i == 46 || i == 45 || i == 44 || i == 43 ||
        i == 42 || i == 41 || i == 40 || i == 48 || i == 49 ) {
      color = lights.color(255, 255, 255);
    }
    lights.set_color(i, G35::MAX_INTENSITY, color);
  }
}

void allon() { 
  for(int i = 0; i < LIGHT_COUNT; i++) {
    color_t color = lights.color(255, 255, 255);
    lights.set_color(i, 16, color);
  }
}

void alloff() { 
  for(int i = 0; i < LIGHT_COUNT; i++) {
    color_t color = lights.color(0, 0, 0);
    lights.set_color(i, G35::MAX_INTENSITY, color);
  }
}

//void bluespectrum() { 
  //for(uint8_t i = 0; i < LIGHT_COUNT; i++) {
    //uint8_t r = 2;
    //uint8_t g = (i * 16) / 50;
    //uint8_t b = 15;
    //color_t color = lights.color(r, g, b);
    //lights.set_color(i, G35::MAX_INTENSITY, color);
  //}
//}
//void blue() {
  //int f = 4*c;
  //int d = f / 128;
  //uint16_t e = f % 128;
  //if (d % 2 != 0) {
    //e =  128 - e;
  //}
  //for(int i = 0; i < LIGHT_COUNT; i++) {
    //color_t color = lights.color(2, 0, 15);
    //lights.set_color(i, e, color);
  //}
//}

void blue() { 
  uint16_t f = c / 6;
  uint16_t d = f / 16;
  uint16_t e = f % 16;
  if (d % 2 != 0) {
    e =  15 - e;
  }
  for(int i = 0; i < LIGHT_COUNT; i++) {
    color_t color = lights.color(0, 0, e);
    lights.set_color(i, G35::MAX_INTENSITY, color);
  }
}

static int pulse[] = { 0, 0x66, 0xcc, 0x66, 0};
void storm() { 
  static boolean storm_state = true;
  color_t color = lights.color(255, 255, 255);
  if (c % 2 == 0) {
    storm_state = !storm_state;
  }
  for(int j = 0; j < 5; j++) {
    int i = (c + j) % LIGHT_COUNT;
    lights.set_color(i, pulse[j], color);
  }
  c+=1;
}


