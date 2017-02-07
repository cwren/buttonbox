#include <G35.h>
#include <G35String.h>

#define LIGHT_COUNT (50)
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
 
#define SQUID (0)
#define BUUTT (1)
#define TRAIL (2)
#define MOUSE (3)

G35String lights(G35_PIN, LIGHT_COUNT);

static uint16_t c;
static unsigned long press_time = 0;
static int sequence[] = { -1, -1, -1, -1 };
static int button[] = { A0, A1, A2, A3 };
static int lamp[] = { 3, 9, 10, 11 };
static int numButtons = 4;
static int mode = MODE_RAINBOW;
static int PATTERNS[][4] = { 
  { SQUID, BUUTT, TRAIL, MOUSE },  //  0
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
  { MOUSE, TRAIL, BUUTT, SQUID },  // 23
};

void setup() {
  c = 0;
  for (int i = 0; i < numButtons; i++) {
    pinMode(button[i], INPUT);
    digitalWrite(button[i], HIGH);
  }
  clear_sequence();
  lights.enumerate();  //Enumerate lights on string to enable individual bulb addressing
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
    mode = interpret_sequence(mode);
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
      rainbow();
      break;
      
    case MODE_NIGHTLIGHT:
      nightlight();
      break;

    case MODE_OFF:
    default: 
      alloff();
      break;
   }
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
  for(int i = 0; i < LIGHT_COUNT; i++) {
    color_t color = lights.color_hue(((i + c) * step) % (HUE_MAX + 1));
    lights.set_color(i, G35::MAX_INTENSITY, color);
  }
  c+=1;
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
    if (i == 14 || i == 15 || i == 18 || i == 19) {
      color = lights.color(255, 255, 255);
    }
    lights.set_color(i, G35::MAX_INTENSITY, color);
  }
}
void allon() { 
  for(int i = 0; i < LIGHT_COUNT; i++) {
    color_t color = lights.color(255, 255, 255);
    lights.set_color(i, G35::MAX_INTENSITY, color);
  }
}
void alloff() { 
  for(int i = 0; i < LIGHT_COUNT; i++) {
    color_t color = lights.color(0, 0, 0);
    lights.set_color(i, G35::MAX_INTENSITY, color);
  }
}

