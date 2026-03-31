#include <LiquidCrystal.h>
#include <FastLED.h>

#define NUM_LEDS 8
#define LED_PIN 10
#define POT_PIN A0

#define RPM_MIN 0
#define RPM_MAX 8000
#define SHIFT_RPM 7000

CRGB leds[NUM_LEDS];

const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

CRGB baseColors[NUM_LEDS] = {
  CRGB::Yellow,
  CRGB::Yellow,
  CRGB::Yellow,
  CRGB::Red,
  CRGB::Red,
  CRGB::Blue,
  CRGB::Blue,
  CRGB::Blue
};

void setup() {
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(50);

  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("RPMS:       ");
  lcd.setCursor(0, 1);
  lcd.print("CURRENT GEAR: #");
}

void strobeEffect() {
  for (int flash = 0; flash < 10; flash++) {
    for (int i = 0; i < NUM_LEDS; i++) leds[i] = CRGB::Red;
    FastLED.show();
    delay(25);
    for (int i = 0; i < NUM_LEDS; i++) leds[i] = CRGB::Black;
    FastLED.show();
    delay(25);
  }
}

int getRPM() {
  int potValue = analogRead(POT_PIN);
  return map(potValue, 0, 1023, RPM_MIN, RPM_MAX);
}

int getNumLedsToLight(int rpm) {
  if (rpm < 1000) return 0;
  return map(rpm, 1000, SHIFT_RPM, 1, NUM_LEDS);
}

void loop() {
  int rpm = getRPM();

  lcd.setCursor(6, 0);
  lcd.print("      ");
  lcd.setCursor(6, 0);
  lcd.print(rpm);

  if (rpm >= SHIFT_RPM) {
    lcd.setCursor(15, 1);
    lcd.print(" ");
    strobeEffect();
  } else {
    lcd.setCursor(15, 1);
    lcd.print(" ");

    int numLeds = getNumLedsToLight(rpm);
    FastLED.clear();
    for (int i = 0; i < numLeds; i++) {
      leds[i] = baseColors[i];
    }
    FastLED.show();
  }

  delay(50); 
}
