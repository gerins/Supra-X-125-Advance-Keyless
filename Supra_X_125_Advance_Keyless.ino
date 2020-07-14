#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_BMP280.h>
#include <DS3231.h>
// #include <Hash.h>
// #include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include "Costum_Fonts.h"
#include "Costum_Images.h"
#include "Web_Page.h"

DS3231 rtc;
Adafruit_BMP280 bmp;
Adafruit_SSD1306 display(128, 64, &Wire, -1);

const uint8_t buttonPin = 14;		//D5
const uint8_t buzzer = 12;			//D6
const uint8_t primaryRelay = 13; //D7

unsigned long millisPrintToSerial, millisOled;

void setup()
{
	// Serial.begin(9600);
	settingI2cDevices();
	settingPinAndState();
}

void loop()
{
	displayTimeAndDate(bmp.readAltitude(1013.25), 200);
	// printToSerial(1000);
}

void settingI2cDevices()
{
	bmp.begin(0x76);

	display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
	display.setTextColor(WHITE);
	display.clearDisplay();
	display.display();
}

void settingPinAndState()
{
	digitalWrite(primaryRelay, HIGH);
	digitalWrite(buzzer, HIGH);

	pinMode(buttonPin, INPUT_PULLUP);
	pinMode(buzzer, OUTPUT);
	pinMode(primaryRelay, OUTPUT);
}

void printToSerial(int duration)
{
	if (millis() - millisPrintToSerial >= duration)
	{
		millisPrintToSerial = millis();
		Serial.print(" batteryVoltage   : ");
		Serial.print(getBatteryVoltage());
		Serial.print("   millis() - timeStart   : ");
		Serial.print(millis() - timeStart);
		Serial.print("   counterOled   : ");
		Serial.print(counterOled);
		Serial.println("");
	}
}

void displayTimeAndDate(float getAltitude, int refreshInterval)
{
	if (millis() - millisOled >= refreshInterval)
	{
		millisOled = millis();

		bool h12 = false;
		bool PM = false;
		bool Century = false;
		byte nowHour = rtc.getHour(h12, PM);
		int8_t tinggiDisplay = -3;
		display.clearDisplay();

		display.setFont(&Cousine_Bold_11);
		display.setCursor(32, 16 + tinggiDisplay);

		display.print(rtc.getDate());
		display.print('-');
		display.print(stringOfMonth(rtc.getMonth(Century)));
		display.print('-');
		display.print(rtc.getYear());

		display.setFont(&DSEG7_Classic_Bold_21);

		display.setCursor(6, 43 + tinggiDisplay);
		if (nowHour < 10)
			display.print('0');
		display.print(nowHour);
		display.print(':');

		display.setCursor(47, 43 + tinggiDisplay);
		if (rtc.getMinute() < 10)
			display.print('0');
		display.print(rtc.getMinute());
		display.print(':');

		display.setCursor(88, 43 + tinggiDisplay);
		if (rtc.getSecond() < 10)
			display.print('0');
		display.print(rtc.getSecond());

		display.drawLine(3, 50 + tinggiDisplay, 125, 50 + tinggiDisplay, WHITE);

		display.setFont(&Meteocons_Regular_11);
		display.setCursor(80, 64 + tinggiDisplay);
		if (nowHour > 18 || nowHour < 6)
		{
			display.print('C');
		}
		else
		{
			display.print('B');
		}

		display.setCursor(93, 62 + tinggiDisplay);
		display.setFont();
		display.setTextSize(1);
		display.print(getAltitude, 0);
		display.print("C");

		display.setCursor(7, 56 + tinggiDisplay);
		display.print(DayOfWeek(rtc.getDoW()));

		display.display();
	}
}

String DayOfWeek(uint8_t dayOfWeek)
{
	switch (dayOfWeek)
	{
	case 1:
		return "Minggu";
	case 2:
		return "Senin";
	case 3:
		return "Selasa";
	case 4:
		return "Rabu";
	case 5:
		return "Kamis";
	case 6:
		return "Jumat";
	case 7:
		return "Sabtu";
	default:
		break;
	}
}

String stringOfMonth(uint8_t getMonth)
{
	switch (getMonth)
	{
	case 1:
		return "JAN";
	case 2:
		return "FEB";
	case 3:
		return "MAR";
	case 4:
		return "APR";
	case 5:
		return "MAY";
	case 6:
		return "JUN";
	case 7:
		return "JUL";
	case 8:
		return "AUG";
	case 9:
		return "SEP";
	case 10:
		return "OCT";
	case 11:
		return "NOV";
	case 12:
		return "DEC";
	default:
		break;
	}
}
