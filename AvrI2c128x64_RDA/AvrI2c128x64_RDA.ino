//Проверка совместимости ssd1306 128x64
// и RDA5807 
/* Добавляем управление с кнопок от AG
  GND   GND
  VCC   +3,3v
  D0    A5
  D1    A4
  RES   на пин 8 или через 10k к +3,3v и 1,0 mk на GND
  DC    на GND, адрес 0x3C; на +3,3v, адрес 0x3D
  CS    на GND 
*/
// кнопка подключена сюда (BTN_PIN --- КНОПКА --- GND)
#define BTN_PIN_UP 2   //  кнопка cледующ. станция
#define BTN_PIN_DOWN 3 //  кнопка предыд. станция
#define BTN_PIN_MODE 4 //  переключатель режима

#include <GyverButton.h> // Библиотека работы с кропками от Гайвера

//#include "SSD1306Ascii.h"
#include <SSD1306AsciiAvrI2c.h>
#include <radio.h>
#include <RDA5807M.h>
#include <RDSParser.h>

#include <avr/pgmspace.h> // константы хранятся в прогр. памяти

// 0X3C+SA0 - 0x3C or 0x3D
#define I2C_ADDRESS 0X3C //0x3C
#define rstPin 8 // пин RST дисплея
unsigned long nextFreqTime = 1000; // интервал вывода частоты
RADIO_FREQ lastf = 0;
RADIO_FREQ f = 0;
uint16_t EEMEM StartFrequency = 10470;  // начальное значение частоты станции, попадает в файл .eep
                    // если дальше в скрипте есть запись ЕЕПРОМ
uint16_t LastFrequency = 8570;

uint8_t r;    // новое значение уровня сигнала
uint8_t lastrssi;  // последнее значение уровня сигнала
uint8_t nrReg = 0; // 0 - ручная настройка, 1 - предустановленные частоты, 2 - изменение громкости

SSD1306AsciiAvrI2c oled; // класс дисплея
RDA5807M radio;    // Создаем класс для  RDA5807 chip radio
RDSParser rds;     // Класс RDS парсера

GButton knUp(BTN_PIN_UP);
GButton knDown(BTN_PIN_DOWN);
GButton knMode(BTN_PIN_MODE);

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
const String regName[] = {
  "T U N E", "P R E S E T", "V O L U M E",
};
RADIO_INFO StateInfo; // буфер для приема параметров станции
int    i_sidx = 11; // Стартуем со станции с indexa
int DlinaSpiska = (sizeof(preset) / sizeof(RADIO_FREQ)) - 1;// длина списка станций

// Вывод частоты настройки на дисплей.
void DisplayFrequency(RADIO_FREQ f)
{
  char s[12];
  radio.formatFrequency(s, sizeof(s));
  oled.set1X(); 
  oled.setCursor(0,0);
  oled.println("F R E Q U E N C Y");
  oled.set2X();
  oled.print(s); oled.clearToEOL();
} // DisplayFrequency()

void RDS_process(uint16_t block1, uint16_t block2, uint16_t block3, uint16_t block4) {
  rds.processData(block1, block2, block3, block4);
} // RDS_process

/// Update the ServiceName text on the display.
void DisplayServiceName(char *name)
{
//  Serial.print("RDS:");
//  Serial.println(name);
  oled.set1X(); 
  oled.setCursor(0,0);
  oled.print("R D S : ");
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
  oled.setCursor(0,6);
  oled.print(regName[r]);
  oled.clearToEOL();
}

//####################################################################################
void setup() {
  pinMode(rstPin, OUTPUT);
  Serial.begin(9600); 
  oled.reset(rstPin);
  oled.begin(&Adafruit128x64, I2C_ADDRESS);
  oled.setFont(Verdana12);  //Adafruit5x7
  radio.init();
  radio.debugEnable();

  oled.setContrast(0);  
  oled.clear();
  f = radio.getFrequency();
  Serial.println(f);
  DisplayFrequency(f);
  oled.set1X();
  
  delay(1000);
  radio.setBandFrequency(RADIO_BAND_FM, pgm_read_word_near(preset + i_sidx));// включение диапазона частот 
//  f = eeprom_read_word(StartFrequency);
  Serial.println(pgm_read_word_near(preset + i_sidx));
  radio.setFrequency(pgm_read_word_near(preset + i_sidx)); // запись частоты в радиочип
  radio.attachReceiveRDS(RDS_process);
  rds.attachServicenNameCallback(DisplayServiceName);  // объявление пп печати RDS 
  Serial.println("Request getFrequency");
  f = radio.getFrequency();
  DisplayFrequency(f);
  DisplayRegim(nrReg);
} // end setup
//####################################################################################
void loop() {
  unsigned long now = millis();

  // check for RDS data
   radio.checkRDS();

  knUp.tick();  // обязательная функция отработки. Должна постоянно опрашиваться
  knDown.tick();
  knMode.tick();
    if (knUp.isSingle()){ 
      radio.seekUp(true);
    }
    if (knDown.isSingle()) {
      radio.seekDown(true); 
    }
    if (knMode.isSingle()) {
      nrReg = nrReg + 1;
      Serial.println(nrReg);
      if(nrReg == 3) { nrReg = 0; }
      DisplayRegim(nrReg); 
    }

    if (now > nextFreqTime) {
      f = radio.getFrequency();
      if (f != lastf) {
        Serial.println(f);  // вывод новой частоты настройки
        DisplayFrequency(f);
        lastf = f;
        nextFreqTime = now + 1000;

//        eeprom_update_word(LastFrequency, StartFrequency);
      } // if
      radio.getRadioInfo(&StateInfo); // читает параметры приема станции
      r = StateInfo.rssi; // текущий уровень приёма
      if (r != lastrssi) {
        DisplayState(); // вывод уровня приёма если изменилсялся
        lastrssi = r;
      } // if
    } // if
} //end loop
