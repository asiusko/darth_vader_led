#include <Adafruit_NeoPixel.h>

#define PHOTO_POWER_PIN 0
#define PHOTO_READ_PIN 1
#define TOUCH_PIN 4
#define LED_PIN 3
#define NUM_LEDS 42
#define NIGHT_SENSOR_MAX 4095
#define NIGHT_SENSOR_MIN 500

Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

enum Modes { MAX_RED, MID_RED, LOW_RED, NIGHT_SENSOR, PULSE, PULSE_RUN, OFF };
Modes currentMode = OFF;

unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 200;
int lastTouchState = LOW;
int brightness = 0;
bool isPulseIncreasing = true;
uint8_t pulseBrightness = 0;
int pulseDirectionSign = 1;
int wavePosition = 0;  
unsigned long lastPulseMillis = 0;
unsigned long lastWaveMillis = 0;
const unsigned long pulseInterval = 20;
const unsigned long waveInterval = 100;

void setup() {  
  pinMode(PHOTO_POWER_PIN, OUTPUT);
  pinMode(PHOTO_READ_PIN, INPUT);
  digitalWrite(PHOTO_POWER_PIN, LOW);
  digitalWrite(PHOTO_POWER_PIN, HIGH);
  pinMode(TOUCH_PIN, INPUT);
}

void loop() {
  handleTouchInput();

  switch (currentMode) {
    case MAX_RED:
      setStripColor(255, 0, 0, 255);
      break;

    case MID_RED:
      setStripColor(255, 0, 0, 180);
      break;

    case LOW_RED:
      setStripColor(255, 0, 0, 130);
      break;

    case NIGHT_SENSOR:
      handleNightSensor();
      break;

    case PULSE:
      handlePulse();
      break;

    case PULSE_RUN:
      handlePulseRun();
      break;  

    case OFF:
      strip.clear();      
      break;
  }

  delay(50);
  strip.show();
}

void handleTouchInput() {
  int touchState = digitalRead(TOUCH_PIN);
  if (touchState != lastTouchState && touchState == HIGH) {
    unsigned long currentTime = millis();
    if (currentTime - lastDebounceTime > debounceDelay) {
      lastDebounceTime = currentTime;
      currentMode = static_cast<Modes>((currentMode + 1) % 7);
    }
  }
  lastTouchState = touchState;
}

void setStripColor(uint8_t red, uint8_t green, uint8_t blue, uint8_t brightness) {
  red = map(brightness, 0, 255, 0, red);
  green = map(brightness, 0, 255, 0, green);
  blue = map(brightness, 0, 255, 0, blue);

  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(red, green, blue));
  }
}

void handleNightSensor() {
  int sensorValue = analogRead(PHOTO_READ_PIN);

  if (sensorValue > NIGHT_SENSOR_MAX) {
    setStripColor(255, 0, 0, 255);
  } else if (sensorValue > NIGHT_SENSOR_MIN) {
    int dynamicBrightness = map(sensorValue, NIGHT_SENSOR_MIN, NIGHT_SENSOR_MAX, 0, 255);
    setStripColor(255, 0, 0, dynamicBrightness);
  } else {
    strip.clear();    
  }
}

void handlePulseRun() {
  unsigned long currentMillis = millis();

  if (currentMillis - lastPulseMillis > pulseInterval) {
    lastPulseMillis = currentMillis;
    pulseBrightness += pulseDirectionSign * 5;

    if (pulseBrightness <= 0) {
      pulseBrightness = 0;
      pulseDirectionSign = 1;
    } else if (pulseBrightness >= 255) {
      pulseBrightness = 255;
      pulseDirectionSign = -1;
    }
  }

  if (currentMillis - lastWaveMillis > waveInterval) {
    lastWaveMillis = currentMillis;
    wavePosition++;
    if (wavePosition >= NUM_LEDS) wavePosition = 0;
  }

  strip.clear();

  for (int i = 0; i < NUM_LEDS; i++) {
    if (i == wavePosition) {
      strip.setPixelColor(i, strip.Color(pulseBrightness, 0, 0));
    } else {      
      int diff = abs(i - wavePosition);
      if (diff == 1) {
        strip.setPixelColor(i, strip.Color(pulseBrightness / 4, 0, 0));
      }
    }
  }  
}

void handlePulse() {
  static unsigned long lastUpdate = 0;
  unsigned long currentTime = millis();

  if (currentTime - lastUpdate >= 50) {
    lastUpdate = currentTime;
    if (isPulseIncreasing) {
      brightness += 5;
      if (brightness >= 255) {
        brightness = 255;
        isPulseIncreasing = false;
      }
    } else {
      brightness -= 5;
      if (brightness <= 0) {
        brightness = 0;
        isPulseIncreasing = true;
      }
    }
    setStripColor(255, 0, 0, brightness);
  }
}