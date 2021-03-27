
#include <LiquidCrystal.h>
#include "Button.h"

// Coffee Stats
uint32_t
startTimeTop = 0,
startTimeBot = 0;

const uint8_t
NUM_FLAVOURS = 5,
NO_COFFEE = 255;
uint8_t
topFlavour = NO_COFFEE,
botFlavour = NO_COFFEE;

// Times in minutes
const uint32_t
AGE_STALE = 60,
AGE_CLEAR = 120;

// Buttons
Button
startButton(8),
flavourButton(9),
moveUpButton(10);

uint32_t nextUpdateTime = 0;
bool heartbeat = false;
uint16_t instructionCounter = 0xFFFE;

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
	lcd.print(F("Coffee Clock v1"));
	for (uint8_t g = 0; g <= 4; g++) {
		printCoffeeGlyph(9, 1, g);  // coffee pot
		delay(400);
	}
	lcd.clear();
}

void loop() {
	updateButtons();

	// If no coffee is on, show instructions
	if (topFlavour == NO_COFFEE && botFlavour == NO_COFFEE) {
		instructionCounter++;
	}
	if (instructionCounter > (2000 / 50)) {
		uint8_t instructionState = 0;

		// Wait for a customer
		do {
			switch (instructionState) {
			case 0:
				lcd.clear();
				lcd.setCursor(0, 0);
				lcd.print(F("1. Press to Start"));
				lcd.setCursor(1, 1);
				lcd.write((uint8_t)0);  // down arrow
				break;
			case 1:
				lcd.clear();
				lcd.setCursor(0, 0);
				lcd.print(F("2. Choose Flavour"));
				lcd.setCursor(10, 1);
				lcd.write((uint8_t)0);  // down arrow
				break;
			case 2:
				lcd.clear();
				lcd.setCursor(0, 0);
				lcd.print(F("3. Next Pot"));
				lcd.setCursor(18, 1);
				lcd.write((uint8_t)0);  // down arrow
				break;
			}
			if (++instructionState >= 3)instructionState = 0;

			for (uint16_t i = 0; i < (2000 / 50) && !startButton.isClicked(); i++) {
				delay(50);
				updateButtons();
			}
		} while (!startButton.isClicked());

		instructionCounter = 0;
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
		topTime = (millis() - startTimeTop) / 60000,
		botTime = (millis() - startTimeBot) / 60000;

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
		displayGlyphByAge(0, topTime);
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
		displayGlyphByAge(1, botTime);
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

void displayGlyphByAge(uint8_t line, uint32_t age) {
	uint8_t glyph = 0;
	if (age < AGE_STALE) {
		glyph = (AGE_STALE - 1 - age) / (AGE_STALE / 4) + 1;
	}
	printCoffeeGlyph(0, line, glyph);
}

void printCoffeeGlyph(uint8_t x, uint8_t line, uint8_t glyph) {
	// Make sure the correct glyph is loaded for the current line
	loadCoffeeGlyph(lcd, line, glyph);
	lcd.setCursor(x, line);
	lcd.write((uint8_t)(2 * line + 2));
	lcd.write((uint8_t)(2 * line + 3));
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
		lcd.print(F("Vanilla "));
	} else if (flav == 3) {
		lcd.print(F("Dark    "));
	} else if (flav == 4) {
		lcd.print(F("Decaf   "));
	} else {
		lcd.print(F("        "));
	}
}
