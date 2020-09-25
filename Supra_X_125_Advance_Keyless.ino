#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_BMP280.h>
#include <DS3231.h>
#include <max6675.h>
#include "Costum_Fonts.h"
#include "Costum_Images.h"

// WebServer Library
// #include <ESP8266WiFi.h>
// #include <ESPAsyncTCP.h>
// #include <ESP8266WebServer.h>
// #include <ESPAsyncWebServer.h>
// #include <Hash.h>
// #include <FS.h>
// #include "Web_Page.h"

#define SCK_PIN 14 // Pin D5 SCK=Serial CLock (Kalo di arduino Pin 13)
#define SO_PIN 12  // Pin D6 SO=Slave Out (Kalo di arduino Pin 12)
#define CS_PIN 15  // Pin D8 CS=Chip Select (Kalo di arduino Pin 10)

DS3231 rtc;
Adafruit_BMP280 bmp;
// AsyncWebServer server(80);
// WiFiEventHandler disconnectedEventHandler;
Adafruit_SSD1306 display(128, 64, &Wire, -1);
MAX6675 thermocouple(SCK_PIN, CS_PIN, SO_PIN);

char *wifiSSID = "Redmiqwery1";
char *wifiPassword = "kucing123";

const uint8_t buttonPin = 16; //D0
const uint8_t buzzer = 3;	  //RX

uint8_t counterOled = 1;
bool lastSwitchButton, checkingSwitchOled;
unsigned long timeStart, millisPrintToSerial, millisOled;

void setup()
{
	// Serial.begin(9600);
	settingI2cDevices();
	settingPinAndState();
	// startWiFiAndServer();
}

void loop()
{
	pressToStartTimer(buttonPin);
	switchAndDisplayOledScreen(200, 70);
	// printToSerial(30);
}

void startWiFiAndServer()
{
	// WiFi.begin(wifiSSID, wifiPassword);

	// disconnectedEventHandler = WiFi.onStationModeDisconnected([](const WiFiEventStationModeDisconnected &event) {
	// 	Serial.println("Station disconnected, trying to reconnect");
	// });

	// if (!SPIFFS.begin())
	// {
	// 	Serial.println("An Error has occurred while mounting SPIFFS");
	// }

	// File file = SPIFFS.open("/index.html", "r");
	// if (!file)
	// {
	// 	Serial.println("Failed to open file for reading");
	// }

	// file.close();

	// server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
	// 	request->send(SPIFFS, "/index.html");
	// });

	// server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request) {
	// 	request->send_P(200, "text/plain", String(bmp.readTemperature()).c_str());
	// });

	// server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request) {
	// 	request->send_P(200, "text/plain", String(bmp.readAltitude(1013.25)).c_str());
	// });

	// server.on("/pressure", HTTP_GET, [](AsyncWebServerRequest *request) {
	// 	request->send_P(200, "text/plain", String(getBatteryVoltage()).c_str());
	// });

	// server.begin();
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
	digitalWrite(buzzer, HIGH);

	pinMode(buttonPin, INPUT_PULLUP);
	pinMode(buzzer, OUTPUT);
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

void pressToStartTimer(byte inputButton)
{
	bool currentSwitchButton = digitalRead(inputButton);
	if (currentSwitchButton != lastSwitchButton)
	{
		if (currentSwitchButton == LOW)
		{
			timeStart = millis();
			checkingSwitchOled = true;
		}
		else
		{
			checkingSwitchOled = false;
		}
		lastSwitchButton = currentSwitchButton;
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

void switchAndDisplayOledScreen(int refreshDuration, uint8_t pressDuration)
{
	if (millis() <= 2000)
	{
		display.clearDisplay();
		display.drawBitmap(0, 0, supraX125Bitmap, 128, 64, WHITE);
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
		case 6:
			displayWifiConnectifity();
			break;
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

	switch (rtc.getMonth(Century))
	{
	case 1:
		display.print("JAN");
		break;
	case 2:
		display.print("FEB");
		break;
	case 3:
		display.print("MAR");
		break;
	case 4:
		display.print("APR");
		break;
	case 5:
		display.print("MAY");
		break;
	case 6:
		display.print("JUN");
		break;
	case 7:
		display.print("JUL");
		break;
	case 8:
		display.print("AUG");
		break;
	case 9:
		display.print("SEP");
		break;
	case 10:
		display.print("OCT");
		break;
	case 11:
		display.print("NOV");
		break;
	case 12:
		display.print("DEC");
		break;
	default:
		break;
	}

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

	switch (rtc.getDoW())
	{
	case 1:
		display.print("Minggu");
		break;
	case 2:
		display.print("Senin");
		break;
	case 3:
		display.print("Selasa");
		break;
	case 4:
		display.print("Rabu");
		break;
	case 5:
		display.print("Kamis");
		break;
	case 6:
		display.print("Jumat");
		break;
	case 7:
		display.print("Sabtu");
		break;
	default:
		break;
	}

	display.display();
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

void displayWifiConnectifity()
{
	display.clearDisplay();
	display.drawBitmap(1, 9, wifiBitmap, 40, 40, WHITE);

	display.setFont(&Cousine_Bold_11);
	display.setCursor(6, 57);
	display.print("WiFi");
	display.setCursor(58, 20);
	display.print("Status");

	display.setCursor(42, 38);
	display.setFont();
	display.setTextSize(1);
	// if (WiFi.status() == WL_CONNECTED)
	// {
	// 	display.print(WiFi.localIP());
	// }
	// else
	// {
	// 	display.setCursor(48, 34);
	// 	display.print("Reconnecting");
	// }

	display.display();
}