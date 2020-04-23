#define BLYNK_PRINT Serial


#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

char authA[] = "vVjNXIv1zQ_V-BLvk9QnHgAIcu-nxGG0";
char authB[] = "jJD3LrHpmD5VEvwqggWRPJwYzAnaA2Ho";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "Narnia";
char pass[] = "fluffyNest24/7";

struct RGB {
  int red;
  int blue;
  int green;
  unsigned long press_time; 
  bool is_on = true;
};

bool button_pressed = false;

struct RGB color_local;
struct RGB color_remote;

int green_pin = 0;
int red_pin = 14;
int blue_pin = 2;
int button_pin = 5;
int led_on_time = 700; // how many ms an led is on for

#define BRIDGE_PIN V10

BlynkTimer timer1;
WidgetLED button_led(V2);
WidgetBridge bridge1(BRIDGE_PIN);

void setup() {
  // Debug console
  Serial.begin(115200);

  displayColor(255,0,0);
  delay(250);
  displayColor(0,255,0);
  delay(250);
  displayColor(0,0,255);
  delay(250);

  Blynk.begin(authB, ssid, pass);

  //timer1.setInterval(500L, sendButton);

  pinMode(red_pin, OUTPUT);
  pinMode(blue_pin, OUTPUT);
  pinMode(green_pin, OUTPUT);
  digitalWrite(red_pin,LOW);
  digitalWrite(green_pin,LOW);
  digitalWrite(blue_pin, LOW);
  pinMode(button_pin, INPUT_PULLUP);

  color_local.blue = 0;
  color_local.red = 255;
  color_local.green = 0;

  displayColor(255,0,0);
  delay(500);
  displayColor(0,255,0);
  delay(500);
  displayColor(0,0,255);
  delay(500);
}

void loop() {
  //Serial.println("loop");
  Blynk.run();
  timer1.run();
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
  }
}


BLYNK_CONNECTED() {
  bridge1.setAuthToken(authA); // Place the AuthToken of the second hardware here
}

// Zebra
BLYNK_WRITE(V5) {
  color_remote.red = param[0].asInt();
  color_remote.green = param[1].asInt();
  color_remote.blue = param[2].asInt();
  displayColor(&color_remote);
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
  if (color_local.is_on) {
    //Serial.println("local check");
    if (millis() - color_local.press_time > led_on_time) { // over time limit
      rgbOff(&color_local); // set values to off
      Serial.println("Local Off");
    }
  }

  if (color_remote.is_on) {
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
  //Serial.print(r);
  //Serial.print(g);
  //Serial.print(b);
  //Serial.println("displaying color");
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
