#include <SPI.h>
#include <mcp_can.h>
#include <LiquidCrystal.h>
#include <FastLED.h>

// ── CAN ──────────────────────────────────────────
#define CAN_CS 53
#define CAN_INT 7
MCP_CAN CAN(CAN_CS);

// ── LEDs ─────────────────────────────────────────
#define NUM_LEDS 8
#define LED_PIN 10
CRGB leds[NUM_LEDS];

// ── LCD ──────────────────────────────────────────
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// ── RPM settings ─────────────────────────────────
#define RPM_MIN   0
#define RPM_MAX   8500
#define SHIFT_RPM 3000

// ── Timing ───────────────────────────────────────
unsigned long lastLCDUpdate  = 0;
unsigned long lastStrobeTime = 0;
bool strobeOn = false;
const int LCD_INTERVAL    = 100;
const int STROBE_INTERVAL = 40;

// ── State ─────────────────────────────────────────
int currentRPM = 0;

CRGB baseColors[NUM_LEDS] = {
  CRGB::Yellow, CRGB::Yellow, CRGB::Yellow,
  CRGB::Red,    CRGB::Red,
  CRGB::Blue,   CRGB::Blue,  CRGB::Blue
};

// ─────────────────────────────────────────────────
void readCAN() {
  while (CAN_MSGAVAIL == CAN.checkReceive()) {
    long unsigned int id;
    unsigned char len = 0;
    unsigned char buf[8];
    CAN.readMsgBuf(&id, &len, buf);

    if (id == 0x17C) {
      currentRPM = (buf[2] * 256) + buf[3];
      currentRPM = constrain(currentRPM, RPM_MIN, RPM_MAX);
    }
  }
}

// ─────────────────────────────────────────────────
int getNumLedsToLight(int rpm) {
  if (rpm < 1000) return 0;
  return constrain(map(rpm, 1000, SHIFT_RPM, 1, NUM_LEDS), 0, NUM_LEDS);
}

// ─────────────────────────────────────────────────
void updateLEDs() {
  if (currentRPM >= SHIFT_RPM) {
    if (millis() - lastStrobeTime >= STROBE_INTERVAL) {
      lastStrobeTime = millis();
      strobeOn = !strobeOn;
      if (strobeOn) {
        for (int i = 0; i < NUM_LEDS; i++) leds[i] = CRGB::Red;
      } else {
        FastLED.clear();
      }
      FastLED.show();
    }
  } else {
    strobeOn = false;
    int numLeds = getNumLedsToLight(currentRPM);
    FastLED.clear();
    for (int i = 0; i < numLeds; i++) leds[i] = baseColors[i];
    FastLED.show();
  }
}

// ─────────────────────────────────────────────────
void updateLCD() {
  if (millis() - lastLCDUpdate < LCD_INTERVAL) return;
  lastLCDUpdate = millis();

  lcd.setCursor(0, 0);
  lcd.print("RPM: ");
  lcd.print(currentRPM);
  lcd.print("    ");
}

// ─────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);

  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(50);
  FastLED.clear();
  FastLED.show();

  lcd.begin(16, 2);
  lcd.print("Initializing...");
  pinMode(CAN_INT, INPUT);

  if (CAN.begin(MCP_ANY, CAN_500KBPS, MCP_8MHZ) == CAN_OK) {
    Serial.println("CAN Init OK");
    lcd.setCursor(0, 0);
    lcd.print("CAN OK!         ");
  } else {
    Serial.println("CAN Init FAIL");
    lcd.setCursor(0, 0);
    lcd.print("CAN FAIL!       ");
    while (1);
  }

  CAN.setMode(MCP_NORMAL);
  delay(1000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("RPM: 0");
}

// ─────────────────────────────────────────────────
void loop() {
  readCAN();
  updateLEDs();
  updateLCD();
}
