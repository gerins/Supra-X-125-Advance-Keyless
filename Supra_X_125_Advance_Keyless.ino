#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_BMP280.h>
#include <DS3231.h>
#include <max6675.h>
#include "Costum_Fonts.h"
#include "Costum_Images.h"

DS3231 rtc;
Adafruit_BMP280 bmp;
#define SCK_PIN 14									  // Pin D5 SCK=Serial CLock (Kalo di arduino Pin 13)
#define SO_PIN 12										  // Pin D6 SO=Slave Out (Kalo di arduino Pin 12)
#define CS_PIN 15										  // Pin D8 CS=Chip Select (Kalo di arduino Pin 10)
MAX6675 thermocouple(SCK_PIN, CS_PIN, SO_PIN); // library baru dicoba di esp8266

Adafruit_SSD1306 display(128, 64, &Wire, -1);

const uint8_t buttonPin = 14;		//D5
const uint8_t buzzer = 12;			//D6
const uint8_t primaryRelay = 13; //D7

unsigned long millisOled;

void setup()
{
	settingI2cDevices();
	settingPinAndState();
}

void loop()
{
	displayTimeAndDate(300); // OLED merefresh layar setiap 0.3 detik sekali
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

void displayTimeAndDate(int refreshInterval)
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
		display.setCursor(3, 16 + tinggiDisplay); // atur posisi kalender

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

		display.drawLine(3, 50 + tinggiDisplay, 125, 50 + tinggiDisplay, WHITE); // print garis bawah

		if (bmp.readAltitude(1013.25) < 100)
		{
			display.setCursor(85, 62 + tinggiDisplay); // setting posisi tampilan MDPL
		}
		else
		{
			display.setCursor(73, 62 + tinggiDisplay); // setting posisi tampilan MDPL
		}

		display.setFont();
		display.setTextSize(1);
		display.print(bmp.readAltitude(1013.25), 0); // baca ketinggian
		display.print(" MDPL");								// print tulisan MDPL

		display.setCursor(89, 9 + tinggiDisplay); // setting posisi hari
		display.print(DayOfWeek(rtc.getDoW()));	// baca hari

		display.setCursor(7, 56 + tinggiDisplay);	 // setting posisi suhu mesin
		display.print(thermocouple.readCelsius()); // tampilkan suhu mesin ke OLED
		display.print("C");								 // print huruf C

		display.display(); // tampilkan ke OLED
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
