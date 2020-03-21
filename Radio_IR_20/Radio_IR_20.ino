//  Версия с 3-мя кнопками, 3-мя режимами
//  очитска дисплея через 1-у мин.
#define ver   "2020.03.20" 

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

#include <GyverButton.h> // Библиотека работы с кропками от Гайвера

//#include "SSD1306Ascii.h"
#include <SSD1306AsciiAvrI2c.h>
#include <radio.h>
#include <RDA5807M.h>
#include <RDSParser.h>

#include <avr/pgmspace.h> // константы хранятся в прогр. памяти

#define I2C_ADDRESS 0X3C //0x3C
#define rstPin 8 // пин RST дисплея
unsigned long now = 0; // текущее время 
unsigned long nextFreqTime = 1000; // интервал вывода частоты
unsigned long sleepTime = 0; //  время без нажатия кнопок 
const unsigned long timeOut =180000; //   интервал перехода сон в мс
RADIO_FREQ lastf = 0;
RADIO_FREQ f = 0;
uint16_t EEMEM StartFrequency = 10470;  // начальное значение частоты станции, попадает в файл .eep
                    // если дальше в скрипте есть запись ЕЕПРОМ
uint16_t LastFrequency = 8570;

uint8_t r;    // новое значение уровня сигнала
uint8_t lastrssi;  // последнее значение уровня сигнала
uint8_t nrReg = 0; // 0 - ручная настройка, 1 - предустановленные частоты, 2 - изменение громкости
uint8_t napravlenie = 1; //1 - вверх, -1 - вниз  
uint16_t volume = 5;     // текущий уровень громкости

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
  oled.println("F R E Q U E N C Y   ");
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
        if (comm == 1) radio.seekUp(true);
        else radio.seekDown(true); 
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
      case 2: {        // рагулировка громкости
        if (comm == 1) {
          if (volume < 16) volume++;
          else volume = 16;
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
}
//####################################################################################
void setup() {
  pinMode(rstPin, OUTPUT);
  Serial.begin(9600); 
  oled.reset(rstPin);
  oled.begin(&Adafruit128x64, I2C_ADDRESS);
  oled.setFont(Verdana12);  //  Verdana12
  oled.setContrast(0);  
  oled.clear();
  DisplayIntro();

  radio.init();
  radio.debugEnable();
  radio.setBandFrequency(RADIO_BAND_FM, pgm_read_word_near(preset + i_sidx));// включение диапазона частот 
//  f = eeprom_read_word(StartFrequency);
  Serial.println(pgm_read_word_near(preset + i_sidx));
  radio.setFrequency(pgm_read_word_near(preset + i_sidx)); // запись частоты в радиочип
  radio.attachReceiveRDS(RDS_process);
  rds.attachServicenNameCallback(DisplayServiceName);  // объявление пп печати RDS 
//  rds.attachTimeCallback(DisplayServiceTime); // объявление пп печати времени
  f = radio.getFrequency();
  DisplayFrequency(f);
  DisplayRegim(nrReg);
} // end setup
//####################################################################################
void loop() {
  now = millis();

  // check for RDS data
  if (nrReg != 2) radio.checkRDS(); // для всех кроме громкости

  knUp.tick();  // обязательная функция отработки. Должна постоянно опрашиваться
  knDown.tick();
  knMode.tick();
  if ((now - sleepTime) > timeOut) { oled.clear(); } // гашение дисплея
    if (knMode.isSingle()) {
      nrReg = nrReg + 1;
      if(nrReg == 3) { nrReg = 0; }
      DisplayRegim(nrReg);
      sleepTime = now; 
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
        Serial.println(f);  // вывод новой частоты настройки
        DisplayFrequency(f);
        lastf = f;
        nextFreqTime = now + 1000;

//        eeprom_update_word(LastFrequency, StartFrequency);
      } // if
      radio.getRadioInfo(&StateInfo); // читает параметры приема станции
      r = StateInfo.rssi; // текущий уровень приёма
      if ((r != lastrssi) and (nrReg != 2)) {
        DisplayState(); // вывод уровня приёма если изменилсялся
        lastrssi = r;
      } // if
    } // if
} //end loop
