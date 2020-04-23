#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

// What device you are programming
const char DEVICE = 'A';
// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "Rmadillow";
char pass[] = "Welc0me2the0nline";


char authA[] = "vVjNXIv1zQ_V-BLvk9QnHgAIcu-nxGG0";
char authB[] = "jJD3LrHpmD5VEvwqggWRPJwYzAnaA2Ho";


#if DEVICE == 'A'
  #define V_LED_PIN V2
#else
  #define V_LED_PIN V3
#endif

struct RGB {
  int red;
  int blue;
  int green;
  unsigned long press_time;
  bool is_on = true;
};

bool button_pressed = false;

struct RGB color_local; // store what color we have set on our device
struct RGB color_remote; // store what color the remote device is set to

// Pin assignments
const int green_pin = 0;
int red_pin = 4;
#if DEVICE == 'B'
  red_pin = 14;
#endif
const int blue_pin = 2;
const int button_pin = 5;
const unsigned long led_on_time = 700; // how many ms an led is on for

#define BRIDGE_PIN V10

WidgetLED button_led(V_LED_PIN);
WidgetBridge bridge1(V10);


void setup() {
  // Debug console
  const int pre_connect_delay = 250;
  const int post_connect_delay = 500;
  Serial.begin(115200);
  displayColor(255,0,0);
  delay(pre_connect_delay);
  displayColor(0,255,0);
  delay(pre_connect_delay);
  displayColor(0,0,255);
  delay(pre_connect_delay);

  if (DEVICE == 'A') {
    Blynk.begin(authA, ssid, pass); // connect to Blynk server
    color_local.red = 0;
    color_local.green = 0;
    color_local.blue = 255; // customized
  } else {
    Blynk.begin(authB, ssid, pass); // connect to Blynk server
    color_local.red = 0;
    color_local.green = 255;
    color_local.blue = 0; // customized
  }

  pinMode(red_pin, OUTPUT);
  pinMode(blue_pin, OUTPUT);
  pinMode(green_pin, OUTPUT);
  digitalWrite(red_pin,LOW);
  digitalWrite(green_pin,LOW);
  digitalWrite(blue_pin, LOW);
  pinMode(button_pin, INPUT_PULLUP);
  
  displayColor(255,0,0);
  delay(post_connect_delay);
  displayColor(0,255,0);
  delay(post_connect_delay);
  displayColor(0,0,255);
  delay(post_connect_delay);
}

void loop() {
  //Serial.println("loop");
  Blynk.run();
  button_press_local();
  checkLedTimes();
  showColors();
}



void button_press_local() {
  const int press_interval = 500;
  const int hold_time_min = 1000;
  static unsigned long press_time = millis();

  if (!digitalRead(button_pin) && millis() - press_time > press_interval) { // if it's time to read the button
    delay(50); // try and debounce
    press_time = millis();
    while (!digitalRead(button_pin)) { // wait while the button is being held
      if (millis() - press_time > hold_time_min) {  // if the button is held for long enough
        sweepColors();
        return;
      }
    }
    
    // if we get out of the loop, then it was just a press
    Serial.println("Pressed");
    press_time = millis();
    colorOn(&color_local);
    bridge1.virtualWrite(BRIDGE_PIN, color_local.red, color_local.green, color_local.blue);
    button_led.on();
  }
}

BLYNK_CONNECTED() {
  if (DEVICE == 'A') {
    bridge1.setAuthToken(authB); // Place the AuthToken of the second hardware here
  } else {
    bridge1.setAuthToken(authA); // Place the AuthToken of the second hardware here
  }
  
}

// Zebra
BLYNK_WRITE(V5) {
  color_remote.red = param[0].asInt();
  color_remote.green = param[1].asInt();
  color_remote.blue = param[2].asInt();
  displayColor(&color_remote);
  delay(100);
}

// BRIDGE Data in
BLYNK_WRITE(BRIDGE_PIN) {
  Serial.println("Bridge data received");
  color_remote.red = param[0].asInt();
  color_remote.green = param[1].asInt();
  color_remote.blue = param[2].asInt();
  colorOn(&color_remote);
}

void showColors() {
  int output_color[] = {0, 0, 0};
  if (color_local.is_on) { // add the local rgb values
    output_color[0] += color_local.red;
    output_color[1] += color_local.green;
    output_color[2] += color_local.blue;
  }
  if (color_remote.is_on) { // add remote rgb values
    output_color[0] += color_remote.red;
    output_color[1] += color_remote.green;
    output_color[2] += color_remote.blue;
  }

  // take average of rgb
  displayColor(output_color[0] / 2, output_color[1] / 2, output_color[2] / 2);
}


void checkLedTimes() {
  if (color_local.is_on) { // if the local led is on
    if (millis() - color_local.press_time > led_on_time) { // over time limit
      rgbOff(&color_local); // set values to off
      button_led.off();
      Serial.println("Local Off");
    }
  }

  if (color_remote.is_on) { // if the remote led is on
    if (millis() - color_remote.press_time > led_on_time) { // over time limit
      rgbOff(&color_remote); // set values to off
      Serial.println("Remote Off");
    }
  }
  
}

void rgbOff(struct RGB *rgb_s) {
  rgb_s->is_on = false;
}

void displayColor(int r, int g, int b) {
  analogWrite(red_pin, r);
  analogWrite(green_pin, g);
  analogWrite(blue_pin, b); 
  Serial.print(r);
  Serial.print(g);
  Serial.print(b);
  Serial.println("displaying color");
}
void displayColor(struct RGB *rgb_s) {
  displayColor(rgb_s->red, rgb_s->green, rgb_s->blue);
}


// turn "on" the led. This will be pixed up by the 
// show colors function
void colorOn(struct RGB *rgb_s) {
    rgb_s->is_on = true;
    rgb_s->press_time = millis();
}

void sweepColors() {
  byte r = 255;
  byte g = 0;
  byte b = 0;
  const int delay_time = 6;
  while (!digitalRead(button_pin)) {// while the button is pressed
    for (int i = 0; i < 255; i++) {
      if (digitalRead(button_pin)) { // check if button isn't pressed
        color_local.red = r;
        color_local.green = g;
        color_local.blue = b;
        return;
      }
      
      displayColor(r,g,b);
      delay(delay_time);
      r--;
      g++;
    }

    for (int i = 0; i < 255; i++) {
      
      if (digitalRead(button_pin)) {
        color_local.red = r;
        color_local.green = g;
        color_local.blue = b;
        return;
      }
      displayColor(r,g,b);
      delay(delay_time);
      b++;
      g--;
    }

    for (int i = 0; i < 255; i++) {
      
      if (digitalRead(button_pin)) {
        color_local.red = r;
        color_local.green = g;
        color_local.blue = b;
        return;
      }
      displayColor(r,g,b);
      delay(delay_time);
      b--;
      r++;
    }

    
  }

  // just in case we let go at end of loop
  color_local.red = r;
  color_local.green = g;
  color_local.blue = b;
  
}
