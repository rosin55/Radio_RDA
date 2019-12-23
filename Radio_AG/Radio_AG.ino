// Пример использования библиотеки GyverButton, 1- 2- 3- нажатие
// Управление радио RDA5807:
//    режим 1 - поиск вверх/вниз по диапазону
//    режим 2 - переключение станций по предустановленной таблице
//
// кнопка подключена сюда (BTN_PIN --- КНОПКА --- GND)
#define BTN_PIN_UP 2   //  кнопка cледующ. станция
#define BTN_PIN_DOWN 3 //  кнопка предыд. станция
#define BTN_PIN_MODE 5 //  переключатель режима

uint16_t EEMEM StartFrequency = 10470;  // начальное значение частоты станции, попадает в файл .eep
										// если дальше в скрипте есть запись ЕЕПРОМ
uint16_t LastFrequency = 8570;

#include <avr/eeprom.h> 
#include "GyverButton.h" // библиотека опроса кнопок от Гайвера
#include <radio.h>
#include <RDA5807M.h>
#include <RDSParser.h>
#include <SPI.h>
#include <Wire.h>
#include "SSD1306Ascii.h"    // библиотека для дисплея SSD1306Ascii (только символьный вывод).
#include "SSD1306AsciiWire.h"// https://github.com/greiman/SSD1306Ascii 

#define I2C_ADDRESS 0x3C
#define ledPin = 13; // СД на плате Arduino

GButton knUp(BTN_PIN_UP);
GButton knDown(BTN_PIN_DOWN);
GButton knMode(BTN_PIN_MODE);

// Определим несколько станций:
// 87.50 MHz как 8750

const RADIO_FREQ preset[] PROGMEM = {
  8750, // Бизнес FM
  8910, // Радио Джаз
  9030, // Авторадио
  9080, // Релакс FM
  9120, // Эхо Москвы
  9240, // Радио Дача
  9560, // Радио Звезда
  9600, // Дорожное Радио 
  9640, // Такси FM
  9680, // Детское Радио
 10300, // Радио Шансон
 10470, // Радио 7
 10740  // Хит FM
};
RADIO_INFO StateInfo; // буфер для приема параметров станции
int    i_sidx = 11; // Стартуем со станции с index=11
int DlinaSpiska = (sizeof(preset) / sizeof(RADIO_FREQ)) - 1;// длина списка станций
unsigned long nextFreqTime = 400; // интервал опроса кнопок и ИК пульта
RADIO_FREQ presetf EEMEM = 10470; // частота последней станции перед выключением
RADIO_FREQ lastf = 0; 
RADIO_FREQ f = 0;
uint8_t r;		// новое значение уровня сигнала
uint8_t lastrssi;  // последнее значение уровня сигнала

RDA5807M radio;    // Создаем класс для  RDA5807 chip radio
RDSParser rds;     // Класс RDS парсера


void setup() {
	LastFrequency = eeprom_read_word(&StartFrequency); // чтение сохраненной частоты из ЕЕПРОМ
	Serial.begin(9600);
	Serial.println(LastFrequency);
}
void loop() {
	knUp.tick();  // обязательная функция отработки. Должна постоянно опрашиваться
	if (knUp.isSingle()) Serial.println("Single");     // проверка на один клик
	if (knUp.isDouble()) Serial.println("Double");     // проверка на двойной клик
	if (knUp.isTriple()) Serial.println("Triple");     // проверка на тройной клик
	eeprom_write_word((uint16_t *) &StartFrequency, LastFrequency); // запись в ЕЕПРОМ последней частоты

}
