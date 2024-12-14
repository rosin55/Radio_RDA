#include <GyverButton.h>

#include <SSD1306Ascii.h>
#include <SSD1306AsciiAvrI2c.h>
#include <SSD1306AsciiSoftSpi.h>
#include <SSD1306AsciiSpi.h>
#include <SSD1306AsciiWire.h>
#include <SSD1306init.h>

//  Второй экземпляр приёмника.
//  Версия с 3-мя кнопками, 3-мя режимами
//  очистка дисплея через 3-и мин.
//  ИК упрравления нет
//  Второй экземпляр приёмника.
#define ver   "2025.01.01" 

/* Добавляем управление с кнопок от AG
	Подключение дисплея SSD1306 I2C:
	GND   GND
	VCC   +3,3v
	D0    A5
	D1    A4
	RES   на пин 8 или через 10k к +3,3v и 1,0 mk на GND
	DC    на GND - адрес 0x3C; на +3,3v - адрес 0x3D
	CS    на GND 
*/
// кнопка подключена сюда (BTN_PIN --- КНОПКА --- GND)
#define BTN_PIN_UP 2   //  кнопка cледующ. станция
#define BTN_PIN_DOWN 3 //  кнопка предыд. станция
#define BTN_PIN_MODE 4 //  переключатель режима

#include <GyverButton.h> // Библиотека работы с кнопками от Гайвера

#include "SSD1306Ascii.h"
#include <SSD1306AsciiAvrI2c.h>
#include <radio.h>
#include <RDA5807M.h>
#include <RDSParser.h>

#include <avr/pgmspace.h> // константы хранятся в прогр. памяти
#include <avr/eeprom.h>   // параметры хранятся в эн.незав. памяти

#include "Constants.h"

#define I2C_ADDRESS 0X3C //0x3C адрес дисплея на шине I2C
#define RST_PIN 8 // пин RST дисплея
#define RECV_PIN 9     //  пин ИК приёмника
#define BLINKLED 7		// индикация приёма команд от пульта 

unsigned long now = 0; // текущее время 
unsigned long nextFreqTime = 1000; // интервал вывода частоты
unsigned long sleepTime = 0; //  время без нажатия кнопок 
const unsigned long timeOut = 60000; //180000 ; // 10000//   интервал перехода сон в мс
RADIO_FREQ lastf = 0;
RADIO_FREQ f = 0;
RADIO_INFO StateInfo; // буфер для приема параметров станции
uint16_t EEMEM StartFrequency = 10470;  // стартовая частота станции, попадает в файл .eep
uint8_t EEMEM StartVolume = 8;         // стартовая громкость
uint8_t EEMEM StartnrReg = 0;		// стартовый режим
uint8_t EEMEM Starti_sidx =0;		// стартовое знач. указателя преднастройки частоты

uint8_t volume = 7;     // текущий уровень громкости

uint8_t r;    // новое значение уровня сигнала
uint8_t lastrssi;  // последнее значение уровня сигнала
uint8_t nrReg = 0; 	// 0 - ручная настройка, 1 - предустановленные частоты
										// 2 - изменение громкости, 3 - сон
uint8_t napravlenie = 1; //1 - вверх, -1 - вниз  
bool flSleep = false; // состояние сна
bool flBassBoost = false; // подъём басов
bool flMute = false; 			// выключение звука

SSD1306AsciiAvrI2c oled; // класс дисплея
RDA5807M radio;    // Создаем класс для  RDA5807 chip radio
RDSParser rds;     // Класс RDS парсера

GButton knUp(BTN_PIN_UP);
GButton knDown(BTN_PIN_DOWN);
GButton knMode(BTN_PIN_MODE);

int  i_sidx = 11; // Стартуем со станции с indexa
int DlinaSpiska = (sizeof(preset) / sizeof(RADIO_FREQ)) - 1;// длина списка станций
// Вывод частоты настройки на дисплей.
void DisplayFrequency(RADIO_FREQ f)
{
	char s[12];
	radio.formatFrequency(s, sizeof(s));
	oled.set1X(); 
	oled.setCursor(0,0);
	oled.print("F R E Q U E N C Y"); oled.clearToEOL(); oled.println();
	oled.set2X();
	oled.print(s); oled.clearToEOL();
} // DisplayFrequency()

void RDS_process(uint16_t block1, uint16_t block2, uint16_t block3, uint16_t block4) {
	rds.processData(block1, block2, block3, block4);
} // RDS_process

/// Update the ServiceName text on the display.
void DisplayServiceName(char *name)
{
	oled.set1X(); 
	oled.setCursor(0,0);
	oled.print("R D S  T E X T: ");
	oled.clearToEOL();
	oled.set2X(); 
	oled.setCursor(0,2);
	oled.clearToEOL();
	oled.print(name);
 } // DisplayServiceName()

void DisplayState()
{
	radio.getRadioInfo(&StateInfo); // читает параметры приема станции
	oled.set1X(); 
	oled.setCursor(69,6);
	oled.print("R S S I :");
	oled.setCursor(110,6);
	oled.print(StateInfo.rssi); // выводит уровень сигнала на дисплей
} // DisplayState()

void DisplayRegim(uint8_t r)
{
	oled.set1X();
//  oled.setCursor(0,0); oled.clearToEOL(); 
	oled.setCursor(0,6);
	oled.print(regName[r]); oled.clearToEOL();
} // end DisplayRegim

void DisplayIntro() {
	oled.set1X();
	oled.setFont(Verdana12);
	oled.print("Ver : "); oled.print(ver);
	oled.setFont(Roosewood26);
	oled.setCursor(0,2);
	oled.print("RDA5807M");
	delay(2000);
	oled.setFont(Verdana12);
	oled.set1X();
} // end DisplayIntro

void DisplayVolume() {
	String bar  = "----------------";
	int i =0;
	oled.set1X();
	oled.setCursor(0,0); oled.clearToEOL();
	oled.setFont(X11fixed7x14);
	for (i; i<volume; i++){
		bar.setCharAt(i, char(0x7f));
	}
	oled.setCursor(0,0); oled.print(bar);
	oled.setCursor(112,0); oled.print(volume); // oled.clearToEOL();
	oled.setFont(Verdana12);
	DisplayRegim(nrReg);
} // end DisplayVolume

void ExecCommand(uint8_t comm)
{
	sleepTime = now; // обнуление счетчика сна
	switch ( nrReg ) {
			case 0: {        // плавная настройка
				if (comm == 1) {
					radio.seekUp(true);
					do { radio.getRadioInfo(&StateInfo); radio.debugRadioInfo(); } //Serial.println(StateInfo.tuned);}
    			while ((!StateInfo.tuned)); //&& (startSeek + 300 > millis()));
//    			radio.seekUp(false); radio.debugRadioInfo(); 
				}
				else radio.seekDown(true);{
				  do { radio.getRadioInfo(&StateInfo); }//Serial.println("Ищу вверх");}
    			while ((!StateInfo.tuned)); //&& (startSeek + 300 > millis()));
//    			radio.seekDown(false);
    		} 
			}      
				break;
			case 1:{        // переключение по преднастроенным станциям
				if (comm ==1){
					if (i_sidx < DlinaSpiska) i_sidx++; // проверка на верхнюю границу списка станций
					else i_sidx = 0;          // перейти в начало списка
					radio.setFrequency(pgm_read_word_near(preset + i_sidx)); // запись частоты в радиочип
				}
				else {
					if (i_sidx > 0) i_sidx--;  // проверка на нижнюю границу списка станций
					else i_sidx = DlinaSpiska; // перейти в конец списка
					radio.setFrequency(pgm_read_word_near(preset + i_sidx)); // запись частоты в радиочип
				}
			}
				break;
			case 2: {        // регулировка громкости
				if (comm == 1) {
					if (volume < 15) volume++;
					else volume = 15;
				}
				else {
					if (volume > 0) volume--;
					else volume = 0;
				}
				DisplayVolume();
				radio.setVolume(volume);
			}
				break;
			default:
				Serial.println("reg unknown");
	}
} // end ExecCommand

void DisplayServiceTime(uint8_t hour, uint8_t minute)
{
	oled.set1X(); 
	oled.setCursor(0,0);
	oled.print("R D S  T I M E : ");
	oled.clearToEOL();
	oled.set2X(); 
	oled.setCursor(0,2);
	oled.clearToEOL();
	oled.print(hour); oled.print(" : "); oled.print(minute);
} // end DisplayServiceTime

void RestoreParam(){
	lastf = eeprom_read_word(&StartFrequency);
	volume = eeprom_read_byte(&StartVolume);
	nrReg = eeprom_read_byte(&StartnrReg);
	i_sidx = eeprom_read_byte(&Starti_sidx);
} // end RestoreParam

//####################################################################################

void setup() {
	Serial.begin(9600);
	pinMode(RST_PIN, OUTPUT);
	oled.reset(RST_PIN);
	oled.begin(&Adafruit128x64, I2C_ADDRESS);
	oled.setFont(Verdana12);  //  Verdana12
	oled.setContrast(0);  
	oled.clear();
	DisplayIntro();

	radio.init();
	radio.debugEnable();
	i_sidx = eeprom_read_byte(&Starti_sidx);
	nrReg = eeprom_read_byte(&StartnrReg);
	radio.setBand(RADIO_BAND_FM);						// задание диапазона частот
	radio.setFrequency(eeprom_read_word(&StartFrequency)); // чтение частоты из ЕЕПРОМ и запись в радиочип 
	volume = eeprom_read_byte(&StartVolume); // чтение громкости из ЕЕПРОМ
	radio.setVolume(volume);                //  и запись в радиочип
	radio.attachReceiveRDS(RDS_process);
	rds.attachServiceNameCallback(DisplayServiceName);  // объявление пп печати RDS 
//  rds.attachTimeCallback(DisplayServiceTime); // объявление пп печати времени
	f = radio.getFrequency();
	DisplayFrequency(f);
	DisplayRegim(nrReg);
} // end setup
//####################################################################################
void loop() {
	now = millis();

	knUp.tick();  // обязательная функция отработки. Должна постоянно опрашиваться
	knDown.tick();
	knMode.tick();
	if (((now - sleepTime) > timeOut) and ( not(flSleep) )){ // гашение дисплея и запоминание параметров
		oled.clear();
		eeprom_update_word(&StartFrequency, lastf ); // запоминаем посл. частоту
		eeprom_update_byte(&StartVolume, volume);    // запомнить громкость
		eeprom_update_byte(&StartnrReg, nrReg);			// запомнить режим
		eeprom_update_byte(&Starti_sidx, i_sidx);		// запомнить указатель преднастройки
		flSleep = true;     // состояние сна 
	} 
		if (knMode.isSingle()) {
			if (flSleep) {											// выход из сна
				RestoreParam();
				DisplayFrequency(f);
				DisplayRegim(nrReg);
				sleepTime = now;
				flSleep = false;
			}
			else {
				nrReg = nrReg + 1;
				if(nrReg == 3) { nrReg = 0; }
				DisplayRegim(nrReg);
				DisplayFrequency(f);
				sleepTime = now;
			} 
		}
		if (knUp.isSingle()){ 
			ExecCommand(1);
		}
		if (knDown.isSingle()) {
			ExecCommand(-1); 
		}
		
		if (now > nextFreqTime) {
			f = radio.getFrequency();
			if ((f != lastf) and (nrReg != 2)) {
				DisplayFrequency(f);
				lastf = f;
				nextFreqTime = now + 1000;
			} // if
			radio.getRadioInfo(&StateInfo); // читает параметры приема станции
			r = StateInfo.rssi; // текущий уровень приёма
			if ((r != lastrssi) and (not(flSleep))) {
				DisplayState(); // вывод уровня приёма если изменился и нет сна
				lastrssi = r;
			} // if
		} // if
} //end loop
