#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_BMP280.h>
#include <DS3231.h>
#include <max6675.h>

#include "Costum_Fonts.h"
#include "Costum_Images.h"

#define SCK_PIN 13 // Pin 14 / D5 SCK=Serial CLock (Kalo di arduino Pin 13)
#define SO_PIN 12	 // Pin 12 / D6 SO=Slave Out (Kalo di arduino Pin 12)
#define CS_PIN 10	 // Pin 15 / D8 CS=Chip Select (Kalo di arduino Pin 10)

DS3231 rtc;
Adafruit_BMP280 bmp;
Adafruit_SSD1306 display(128, 64, &Wire, -1);
MAX6675 thermocouple(SCK_PIN, CS_PIN, SO_PIN);

const uint8_t buttonPin = 2;
const uint8_t buzzer = 8;
const uint8_t primaryRelay = 9;

uint8_t counterOled;
bool stateRelay = true;
bool checkingSwitchButton, lastSwitchButton, checkingSwitchOled;
unsigned long timeStart, millisDeepSleep, millisAutoTurnOff, millisOled;

void setup()
{
	settingI2cDevices();
	settingPinAndState();
}

void loop()
{
	pressToStartTimer(buttonPin);
	remoteKeyless(primaryRelay, 350);
	autoTurnOffRelay(&stateRelay, 10000, 8, getBatteryVoltage());
	switchAndDisplayOledScreen(stateRelay, 200, 70);
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

void remoteKeyless(byte inputRelay, int pressDuration)
{
	if ((millis() - timeStart) >= pressDuration && checkingSwitchButton)
	{
		if (stateRelay == LOW)
		{
			stateRelay = HIGH; // Mematikan RELAY
			turnOnBuzzer(1, 200);
		}
		else
		{
			stateRelay = LOW; // Nyalakan RELAY
			turnOnBuzzer(2, 200);
		}
		checkingSwitchButton = false;
		digitalWrite(inputRelay, stateRelay);
	}
}

void pressToStartTimer(byte inputButton)
{
	bool currentSwitchButton = digitalRead(inputButton);
	if (currentSwitchButton != lastSwitchButton)
	{
		if (currentSwitchButton == LOW)
		{
			timeStart = millis();
			checkingSwitchButton = true;
			checkingSwitchOled = true;
		}
		else
		{
			checkingSwitchButton = false;
			checkingSwitchOled = false;
		}
		lastSwitchButton = currentSwitchButton;
	}
}

void autoTurnOffRelay(bool *relayCondition, int durBeforeTurnOff, byte voltThreshold, float batteryVoltage)
{
	if (*relayCondition == true)
	{
		millisAutoTurnOff = millis();
		return;
	}

	if (batteryVoltage >= voltThreshold)
	{
		millisAutoTurnOff = millis();
		return;
	}

	if (batteryVoltage <= voltThreshold && millis() - millisAutoTurnOff >= durBeforeTurnOff)
	{
		*relayCondition = true;
		turnOnBuzzer(1, 200);
		digitalWrite(primaryRelay, *relayCondition);
		return;
	}
}

void turnOnBuzzer(byte times, int duration)
{
	for (int i = 1; i <= times; i++)
	{
		digitalWrite(buzzer, false);
		delay(duration);
		digitalWrite(buzzer, true);
		delay(duration);
	}
}

float getBatteryVoltage()
{
	float vOUT, vIN;
	uint16_t analogValue;
	float R1 = 30000;
	float R2 = 7500;

	// @ts-ignore
	analogValue = analogRead(A0);
	vOUT = (analogValue * 3.3) / 1024;
	return vIN = vOUT / (R2 / (R1 + R2));
}

void switchAndDisplayOledScreen(bool relayCondition, int refreshDuration, uint8_t pressDuration)
{
	if (relayCondition == true)
	{
		Wire.endTransmission();
		display.clearDisplay();
		display.display();
		return;
	}

	if ((millis() - timeStart) >= pressDuration && checkingSwitchOled)
	{
		counterOled++;
		checkingSwitchOled = false;
	}

	if (millis() - millisOled >= refreshDuration)
	{
		Wire.endTransmission(false);
		millisOled = millis();
		switch (counterOled)
		{
		case 1:
			displayTimeAndDate(bmp.readTemperature());
			break;
		case 2:
			displayAltitude(bmp.readAltitude(1013.25));
			break;
		case 3:
			displayBatteryVoltage(getBatteryVoltage());
			break;
		case 4:
			displayEngineTemperature(thermocouple.readCelsius());
			break;
		case 5:
			displayTachometer(7500);
			break;
		// case 6:
		// 	displayWifiConnectifity();
		// 	break;
		default:
			counterOled = 1;
			break;
		}
	}
}

void displayTimeAndDate(float getTemperature)
{
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
	display.print(getTemperature, 1);
	display.print("C");

	display.setCursor(7, 56 + tinggiDisplay);
	display.print(DayOfWeek(rtc.getDoW()));

	display.display();
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

void displayAltitude(float getAltitude)
{
	display.clearDisplay();
	display.drawBitmap(0, 19, MountainBitmap2, 50, 50, WHITE);

	display.setFont(&Cousine_Bold_11);
	display.setCursor(50, 17);
	display.print("Altitude");

	display.setCursor(85, 55);
	display.print("MDPL");

	display.setFont(&DSEG7_Classic_Bold_21);
	display.setCursor(47, 44);
	if (getAltitude < 1000)
		display.print('0');
	if (getAltitude < 100)
		display.print('0');
	if (getAltitude < 10)
		display.print('0');
	display.print(getAltitude, 0);

	display.display();
}

void displayBatteryVoltage(float getVoltage)
{
	display.clearDisplay();
	display.drawBitmap(8, 18, BatteryBitmap, 40, 40, WHITE);

	display.setFont(&Cousine_Bold_11);
	display.setCursor(58, 20);
	display.print("Battery");

	display.setFont(&DSEG7_Classic_Bold_26);
	display.setCursor(44, 54);
	if (getVoltage < 10)
	{
		display.setCursor(56, 54);
	}
	display.print(getVoltage, 1);

	display.setFont(&Cousine_Bold_11);
	display.print("V");

	display.display();
}

void displayEngineTemperature(float getEngineTemp)
{
	display.clearDisplay();
	display.setTextSize(1);
	display.drawBitmap(8, 17, EngineTempBitmap, 40, 40, WHITE);

	display.setFont(&Cousine_Bold_11);
	display.setCursor(55, 20);
	display.print("Engine");

	display.setFont(&DSEG7_Classic_Bold_26);
	display.setCursor(53, 54);
	if (getEngineTemp > 100)
	{
		display.setCursor(41, 54);
	}
	display.print(getEngineTemp, 0);

	display.setCursor(99, 46);
	if (getEngineTemp > 100)
	{
		display.setCursor(109, 46);
	}
	display.setFont();
	display.setTextSize(2);
	display.print('C');
	display.setTextSize(1);

	display.display();
}

void displayTachometer(uint16_t getTachometer)
{
	display.clearDisplay();

	int tachometerLevel = map(getTachometer, 0, 8000, 64, 35);
	if (tachometerLevel < 35)
	{
		tachometerLevel = 35;
	}
	display.fillRect(119, tachometerLevel, 9, 64, WHITE);
	display.fillRect(0, tachometerLevel, 9, 64, WHITE);

	display.setFont(&Cousine_Bold_11);
	display.setCursor(26, 14);
	display.print("Tachometer");

	display.setFont(&DSEG7_Classic_Bold_26);
	display.setCursor(21, 48);

	if (getTachometer < 1000)
		display.print('0');
	if (getTachometer < 100)
		display.print('0');
	if (getTachometer < 10)
		display.print('0');
	display.print(getTachometer);

	display.setFont(&Cousine_Bold_11);
	display.setCursor(54, 60);
	display.print("RPM");

	display.drawLine(0, 35, 17, 35, WHITE);
	display.drawLine(112, 35, 128, 35, WHITE);

	display.display();
}
