// Скетч для проверки работы RDA5806, дисплея на чипе SSD1306 и ИК пульта от видеокамеры.
//  Начато   29.11.2017
//  Изменено 17.07.2019
//  Автор - Сахно А.Д.
// Radio RD5807М
//Oled Module SSD1306 128х32 пикселя 
//     SDA —– pin SDA  (pin A4 для Arduino nano V3)
//     SCL —–  pin SCL (pin A5 для Arduino nano V3)
// 2-е кнопки pin D2, D3
// ИК приемник TSOP4xxx pin D9

#include <newchip.h>
#include <radio.h>
#include <RDA5807M.h>  // от Matthias Hertel по адресу https://github.com/mathertel/Radio
#include <RDSParser.h> // https://github.com/greiman/SSD1306Ascii

#include <avr/pgmspace.h> // константы хранятся в прогр. памяти

#include <IRremote.h> // библиотека ИК приёмника
#include <Key_CarMp3.h>     // Коды кнопок пульта от видеокамеры

#include <SPI.h>
#include <Wire.h>
#include "SSD1306Ascii.h"  // библиотека для дисплея SSD1306Ascii (только символьный вывод).
#include "SSD1306AsciiWire.h"
#define I2C_ADDRESS 0x3C

SSD1306AsciiWire oled;

#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2

// ***
int ledPin = 13; // СД на плате Arduino
int knUp = 2;    // кнопка cледующ. станция
int knDown = 3;    // кнопка предыд. станция

int RECV_PIN = 9; // контакт подключения ИК-приемника
IRrecv irrecv(RECV_PIN);
decode_results results;

// Определим несколько станций:
// 89.40 MHz как 8940

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
int    i_sidx = 0; // Стартуем со станции с index=0
int DlinaSpiska = (sizeof(preset) / sizeof(RADIO_FREQ)) - 1;// длина списка станций
unsigned long nextFreqTime = 400; // интервал опроса кнопок и ИК пульта
RADIO_FREQ lastf = 0; 
RADIO_FREQ f = 0;

RDA5807M radio;    // Создаем класс для  RDA5807 chip radio
RDSParser rds;     // Класс RDS парсера

//*** Мигнуть СД
void BlinkCD() {
  digitalWrite(ledPin, HIGH);
  delay(200);
  digitalWrite(ledPin, LOW);
} // BlinkCD

/// Вывод частоты настройки на Serial и дисплей.
void DisplayFrequency(RADIO_FREQ f)
{
  char s[12];
  radio.formatFrequency(s, sizeof(s));
  Serial.print("FREQ:"); Serial.println(s);
      oled.setCursor(0,0);
      oled.println("FREQ:");
      oled.setCursor(0,2);
      oled.println(s);
 
} // DisplayFrequency()

/// Update the ServiceName text on the Serial monitor.
void DisplayServiceName(char *name)
{
  Serial.print("RDS:");
  Serial.println(name);
      oled.setCursor(0,0);
      oled.print("RDS: ");
      oled.setCursor(0,2);
      oled.clearToEOL();
      oled.print(name);
 } // DisplayServiceName()

void RDS_process(uint16_t block1, uint16_t block2, uint16_t block3, uint16_t block4) {
  rds.processData(block1, block2, block3, block4);
} // RDS_process

void ReadIR(){
	if (irrecv.decode(&results))
  {
	if (results.value == knPlus) 
	{ Serial.println("Нажато W");
	  if(radio.getVolume() > 1){    
	  radio.setVolume(radio.getVolume()-1);
	  }
	}
	if (results.value == knMinus) 
	{ Serial.println("Нажато T");
	  if(radio.getVolume() < 15){
	  radio.setVolume(radio.getVolume()+1);	
	  }
	}
	if (results.value == knEQ ) 
	{ Serial.println("Нажата кнопка поиск вниз");
//      здесь вызов процедуры поиска вниз
  }
	if (results.value == knEQ ) 
	{ Serial.println("Нажата кнопка поиск вверх");
 //     здесь вызов процедуры поиска вверх   
  }
	if (results.value == knEQ ) 
	{ Serial.println("Нажато к предыдущей станции");
        if (i_sidx > 0) {             // проверка на нижнюю границу списка станций
      i_sidx--;
    }
    else i_sidx = DlinaSpiska; // перейти в конец списка
    radio.setFrequency(pgm_read_word_near(preset + i_sidx)); // запись частоты в радиочип
    }
	if (results.value == knEQ ) 
	{ Serial.println("Нажато к следующей станции");
       if (i_sidx < DlinaSpiska) { // проверка на верхнюю границу списка станций
      i_sidx++;
      }
    else i_sidx = 0;          // перейти в начало списка
    radio.setFrequency(pgm_read_word_near(preset + i_sidx)); // запись частоты в радиочип
    }
  delay(1000);
	irrecv.resume(); // получить следующее значение

  }
} // end ReadIR

void setup() {
  //*** инициализация пинов kn1, kn2, ledPin
  pinMode(knUp, INPUT);
  pinMode(knDown, INPUT);
  pinMode(ledPin, OUTPUT);

  // open the Serial port
  Serial.begin(9600);
  Serial.print("Radio...");
  delay(500);

  irrecv.enableIRIn(); // включить приемник

 // initialize with the I2C addr 0x3C (for the 128x32)
  oled.begin(&Adafruit128x32, I2C_ADDRESS); 
  oled.setFont(SystemFont5x7);

  delay(100);
  // 
  oled.clear();
  oled.set2X();
  oled.setCursor(0,0);
  oled.println("  Hello");
  oled.print  (" RDA5807!");
  delay(2000);
  oled.clear();


  // Clear the buffer.
//  oled.init();

  // Initialize the Radio 
  radio.init();

  // Enable information to the Serial port
  //radio.debugEnable();

  radio.setBandFrequency(RADIO_BAND_FM, pgm_read_word_near(preset + i_sidx)); // 5. preset.

  // delay(100);

  radio.setMono(false);
  radio.setMute(false);
  // radio.debugRegisters();
  radio.setVolume(5);
  // setup the information chain for RDS data.
  radio.attachReceiveRDS(RDS_process);
  rds.attachServicenNameCallback(DisplayServiceName);
} // End Setup

void loop() {
  unsigned long now = millis();
  // static unsigned long nextFreqTime = 0;
  // static unsigned long nextRadioInfoTime = 0;
  // static RADIO_FREQ lastf = 0;
  // RADIO_FREQ f = 0;

 
// Проверка и выполнение команд с ИК пульта
	ReadIR();

// Проверка нажатия кнопок
  if (digitalRead(knUp) == 0){ 
    if (i_sidx < DlinaSpiska) { // проверка на верхнюю границу списка станций
      i_sidx++;
    }
    else i_sidx = 0;          // перейти в начало списка
    BlinkCD();
    radio.setFrequency(pgm_read_word_near(preset + i_sidx)); // запись частоты в радиочип
  }
  if (digitalRead(knDown) == 0){
    if (i_sidx > 0) {             // проверка на нижнюю границу списка станций
      i_sidx--;
    }
    else i_sidx = DlinaSpiska; // перейти в конец списка
    BlinkCD();
    radio.setFrequency(pgm_read_word_near(preset + i_sidx)); // запись частоты в радиочип
    }

  // check for RDS data
   radio.checkRDS();

  // update the display from time to time
  if (now > nextFreqTime) {
    f = radio.getFrequency();
    if (f != lastf) {
      // print current tuned frequency
      DisplayFrequency(f);
//      Serial.println(f);
      lastf = f;
    } // if
    nextFreqTime = now + 400;
  } // if  

} // End loop

// End.
