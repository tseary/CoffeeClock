
#include <LiquidCrystal.h>
#include "Button.h"

// Coffee Stats
uint32_t
  startTimeTop = 0,
  startTimeBot = 0;

const uint8_t
  NUM_FLAVOURS = 4,
  NO_COFFEE = 255;
uint8_t
  topFlavour = NO_COFFEE,
  botFlavour = NO_COFFEE;

// Times in minutes
const uint8_t
AGE_FRESH = 15,
AGE_STALE = 30,
AGE_CLEAR = 60;

// Buttons
Button
  startButton(8),
  flavourButton(9),
  moveUpButton(10);

uint32_t nextUpdateTime = 0;
bool heartbeat = false;

// Display
const uint8_t
  RS = 2, EN = 3,
  D4 = 4, D5 = 5, D6 = 6, D7 = 7;
LiquidCrystal lcd(RS, EN, D4, D5, D6, D7);

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  initDisplay();

  // Show splash
  lcd.setCursor(3, 0);
  lcd.print(F("Coffee Clock V1"));
  lcd.setCursor(9, 1);
  displayGlyph(0);  // coffee pot
  delay(2000);
}

void loop() {
  updateButtons();
  
  // If no coffee is on, show instructions
  if (topFlavour == NO_COFFEE && botFlavour == NO_COFFEE) {
    lcd.clear();
    lcd.setCursor(3, 0);
    lcd.print(F("Press to start"));
    lcd.setCursor(1, 1);
    lcd.write((uint8_t)0);  // down arrow
    
    // Wait for a customer
    do {
      delay(50);
      updateButtons();
    } while (!startButton.isClicked());
    
    lcd.clear();
  }

  // Start coffee (bottom burner)
  if (startButton.isClicked() && botFlavour == NO_COFFEE) {
    startTimeBot = millis();
    botFlavour = 0;
  }
  
  // Flavour button
  if (flavourButton.isClicked() && botFlavour != NO_COFFEE) {
    if (++botFlavour >= NUM_FLAVOURS) botFlavour = 0;
  }
  
  // Move bottom pot up, clear bottom burner
  if (moveUpButton.isClicked()) {
    startTimeTop = startTimeBot;
    topFlavour = botFlavour;

    botFlavour = NO_COFFEE;
  }

  if (anyButtonClicked() || millis() > nextUpdateTime) {
    updateDisplay();
  }

  delay(50);
}

void initDisplay() {
  lcd.begin(20, 2);

  // Create arrow glyphs
  lcd.createChar(0, new uint8_t[8]
  {0x04, 0x04, 0x04, 0x04, 0x1F, 0x0E, 0x04, 0x00});  // down arrow

  lcd.createChar(1, new uint8_t[8]
  {0x01, 0x01, 0x05, 0x0D, 0x1F, 0x0C, 0x04, 0x00});  // return arrow

  // Create coffee pot glyphs
  lcd.createChar(2, new uint8_t[8]
  {0x0F, 0x03, 0x07, 0x0F, 0x1F, 0x1F, 0x0F, 0x00});
  lcd.createChar(3, new uint8_t[8]
  {0x1E, 0x13, 0x19, 0x1D, 0x1E, 0x1E, 0x1C, 0x00});

  lcd.createChar(4, new uint8_t[8]
  {0x0F, 0x03, 0x04, 0x08, 0x1F, 0x1F, 0x0F, 0x00});
  lcd.createChar(5, new uint8_t[8]
  {0x1E, 0x13, 0x09, 0x05, 0x1E, 0x1E, 0x1C, 0x00});

  lcd.createChar(6, new uint8_t[8]
  {0x0F, 0x03, 0x04, 0x08, 0x10, 0x10, 0x0F, 0x00});
  lcd.createChar(7, new uint8_t[8]
  {0x1E, 0x13, 0x09, 0x05, 0x02, 0x02, 0x1C, 0x00});
}

inline void updateButtons() {
  startButton.update();
  flavourButton.update();
  moveUpButton.update();
}

bool anyButtonClicked() {
  return startButton.isClicked() ||
         flavourButton.isClicked() ||
         moveUpButton.isClicked();
}

void swapTopAndBottom() {
  uint32_t tempTime = startTimeTop;
  startTimeTop = startTimeBot;
  startTimeBot = tempTime;
  uint8_t tempFlav = topFlavour;
  topFlavour = botFlavour;
  botFlavour = tempFlav;
}

void updateDisplay() {
  // Calculate age of coffee in minutes
  uint32_t
    topTime = (millis() - startTimeTop) / 1000,
    botTime = (millis() - startTimeBot) / 1000;

  // Clear really old coffee
  // TODO move this
  if (topTime >= AGE_CLEAR) topFlavour = NO_COFFEE;
  if (botTime >= AGE_CLEAR) botFlavour = NO_COFFEE;

  uint8_t topHour = topTime / 60,
          topMinute = topTime % 60;
  uint8_t botHour = botTime / 60,
          botMinute = botTime % 60;

  // LCD
  // Top line
  if (topFlavour != NO_COFFEE) {
    lcd.setCursor(0, 0);
    displayGlyph(topTime);
    lcd.setCursor(4, 0);
    displayTimeString(topHour, topMinute);
  } else {
    lcd.setCursor(0, 0);
    lcd.print(F("    -:--"));
  }
  lcd.setCursor(10, 0);
  displayFlavourString(topFlavour);

  // Bottom line
  if (botFlavour != NO_COFFEE) {
    lcd.setCursor(0, 1);
    displayGlyph(botTime);
    lcd.setCursor(4, 1);
    displayTimeString(botHour, botMinute);
  } else {
    lcd.setCursor(0, 1);
    lcd.print(F("    -:--"));
  }
  lcd.setCursor(10, 1);
  displayFlavourString(botFlavour);

  lcd.setCursor(19, 1);
  lcd.print(heartbeat ? '.' : ' ');

  // Schedule the next update
  nextUpdateTime = millis() + 1000;
  heartbeat = !heartbeat;
}

void displayGlyph(uint32_t age) {
  if (age < AGE_FRESH) {
    // Full pot
    lcd.write((uint8_t)2);
    lcd.write((uint8_t)3);
  } else if (age < AGE_STALE) {
    // Half pot
    lcd.write((uint8_t)4);
    lcd.write((uint8_t)5);
  } else {
    // Empty pot
    lcd.write((uint8_t)6);
    lcd.write((uint8_t)7);
  }
}

void displayTimeString(uint8_t hour, uint8_t minute) {
  lcd.print(hour);
  lcd.print(':');
  if (minute <= 9) lcd.print('0');
  lcd.print(minute);
}

void displayFlavourString(uint8_t flav) {
  // Note: all strings must be same length
  if (flav == 0) {
    lcd.print(F("Regular "));
  } else if (flav == 1) {
    lcd.print(F("Hazelnut"));
  } else if (flav == 2) {
    lcd.print(F("Dark    "));
  } else if (flav == 3) {
    lcd.print(F("Decaf   "));
  } else {
    lcd.print(F("        "));
  }
}
