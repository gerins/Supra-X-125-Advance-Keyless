#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Ticker.h>
#include "Costum_Fonts.h"
#include "Costum_Images.h"

Ticker secondTicker;
volatile uint8_t watchDogCounter;

Adafruit_SSD1306 display(128, 64, &Wire, -1);
const uint8_t buttonPin = 14; // D5
unsigned long millisOled;
int counter;

void setup()
{
	secondTicker.attach_ms(100, ISRWatchDog);
	settingI2cDevices();
	settingPinAndState();
}

void loop()
{
	watchDogCounter = 0;
	displayCounter(200); // OLED merefresh layar setiap 0.3 detik sekali

	if (digitalRead(buttonPin) == false)
	{
		delay(700);
	}
}

void settingI2cDevices()
{
	display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
	display.setTextColor(WHITE);
	display.clearDisplay();
	display.display();
}

void settingPinAndState()
{
	pinMode(buttonPin, INPUT_PULLUP);
}

void displayCounter(int refreshInterval)
{
	if (millis() - millisOled >= refreshInterval)
	{
		millisOled = millis();
		counter++;

		display.clearDisplay();
		display.setFont(&DSEG7_Classic_Bold_21);

		display.setCursor(47, 43);
		display.print(counter);

		display.display(); // tampilkan ke OLED
	}
}

void ISRWatchDog()
{
	watchDogCounter++;
	if (watchDogCounter == 5)
	{
		ESP.reset();
	}
}