// Скетч для проверки работы RDA5807, дисплея на чипе SSD1306 и ИК пульта от видеокамеры.
// По нажатию кнопок ищет станции вверх или вниз по диапазону
//  Начато   29.11.2017
//  Изменено 02.08.2019
//  Автор - Сахно А.Д.
// Radio RD5807М
//Oled Module SSD1306 128х32 пикселя 
//     SDA —– pin SDA  (pin A4 для Arduino nano V3)
//     SCL —–  pin SCL (pin A5 для Arduino nano V3)
// 2-е кнопки pin D2 - поиск вверх, D3 - поиск вниз
// ИК приемник TSOP4xxx pin D9

//#include <newchip.h>
#include <radio.h>
#include <RDA5807M.h>
#include <RDSParser.h>

#include <avr/pgmspace.h> // константы хранятся в прогр. памяти
#include <avr/EEPROM.h> // для хранения параметров

#include <IRremote.h> // библиотека ИК приёмника
#include "Keys.h"     // Коды кнопок пульта от видеокамеры

#include <SPI.h>
#include <Wire.h>
//#include "SSD1306Ascii.h"  // библиотека для дисплея SSD1306Ascii (только символьный вывод).
#include "SSD1306AsciiWire.h"
#define I2C_ADDRESS 0x3C //0x3C // адрес OLED дисплея

SSD1306AsciiWire oled;

int ledPin = 13; // СД на плате Arduino
int knUp = 2;    // кнопка cледующ. станция
int knDown = 3;    // кнопка предыд. станция
int knBassBoost = 4; // вкл/выкл усиление басов
int knMode = 5;    // переключатель режима
int knVolPlus = 7; //  громкость больше
int knVolMinus = 6; // громкость меньше
int RECV_PIN = 9; // контакт подключения ИК-приемника
int VolumeLast = 5; // текущая громкость

IRrecv irrecv(RECV_PIN);
decode_results results;

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
unsigned long nextFreqTime = 400; // интервал изменения частоты и ИК пульта
unsigned long nextVolTime = 1000; // интервал опроса кнопок громкости
RADIO_FREQ presetf EEMEM = 10470; // частота последней станции перед выключением
RADIO_FREQ lastf = 0; 
RADIO_FREQ f = 0;
uint8_t r;		// новое значение уровня сигнала
uint8_t lastrssi;  // последнее значение уровня сигнала

RDA5807M radio;    // Создаем класс для  RDA5807 chip radio
RDSParser rds;     // Класс RDS парсера


//*** Мигнуть СД
void BlinkCD() {
  digitalWrite(ledPin, HIGH);
  delay(200);
  digitalWrite(ledPin, LOW);
} // BlinkCD

// Вывод частоты настройки на Serial и дисплей.
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

void DisplayState()
{
  char RDS_en = '-'; // R есть RDS, - нет RDS
  char St_en = '-';  // S есть стерео, - нет стерео
  radio.getRadioInfo(&StateInfo); // читает параметры приема станции
  Serial.print("RSSI: "); Serial.println(StateInfo.rssi);
  if (StateInfo.rds) {RDS_en = 'R';}
  if (StateInfo.stereo){St_en = 'S';}
  oled.setCursor(70,0);
  oled.print(RDS_en); oled.print(St_en);
  Serial.print(RDS_en); Serial.println(St_en);
  oled.setCursor(100,0);
  oled.print("  ");
  oled.setCursor(100,0);
  oled.print(StateInfo.rssi); // выводит уровень сигнала на дисплей

} // DisplayState()

/// Update the ServiceName text on the display.
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
	if (results.value == W) 
	{ Serial.println("Нажато W");
	  if(radio.getVolume() > 1){    
	  radio.setVolume(radio.getVolume()-1);
	  }
	}
	if (results.value == T) 
	{ Serial.println("Нажато T");
	  if(radio.getVolume() < 15){
	  radio.setVolume(radio.getVolume()+1);	
	  }
	}
	if (results.value == SearchDown) 
	{ Serial.println("Нажата кнопка поиск вниз");
//      здесь вызов процедуры поиска вниз
  }
	if (results.value == SearchUp) 
	{ Serial.println("Нажата кнопка поиск вверх");
 //     здесь вызов процедуры поиска вверх   
  }
	if (results.value == SkipDown) 
	{ Serial.println("Нажато к предыдущей станции");
        if (i_sidx > 0) {             // проверка на нижнюю границу списка станций
      i_sidx--;
    }
    else i_sidx = DlinaSpiska; // перейти в конец списка
    radio.setFrequency(pgm_read_word_near(preset + i_sidx)); // запись частоты в радиочип
    }
	if (results.value == SkipUp) 
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
  pinMode(knMode, INPUT);
  pinMode(knVolPlus, INPUT);
  pinMode(knVolMinus, INPUT);
  pinMode(knBassBoost, INPUT);
  pinMode(ledPin, OUTPUT);

  // open the Serial port
  Serial.begin(9600);
  Serial.println("Radio...");
  delay(500);

  //irrecv.enableIRIn(); // включить приемник

 // initialize with the I2C addr 0x3C (for the 128x32)
  oled.begin(&Adafruit128x64, I2C_ADDRESS); 
  oled.setFont(SystemFont5x7);

  delay(100);
  // 
  oled.clear();
  oled.set2X();
  oled.setCursor(0,0);
  oled.println("Scan station");
  oled.print  (" RDA5807!");
  delay(2000);
  oled.clear();

  // Initialize the Radio 
  radio.init();
  f = radio.getFrequency();
  Serial.println(f);
  // Enable information to the Serial port
  radio.debugEnable();

  radio.setBandFrequency(RADIO_BAND_FM, pgm_read_word_near(preset + i_sidx)); // 5. preset.
  radio.setFrequency(pgm_read_word_near(preset + i_sidx)); // запись частоты в радиочип

  // delay(100);

  radio.setMono(false);
  radio.setMute(false);
  // radio.debugRegisters();
  radio.setVolume(VolumeLast); // начальная громкость
  // setup the information chain for RDS data.
  radio.attachReceiveRDS(RDS_process);
  rds.attachServiceNameCallback(DisplayServiceName);
} // End Setup

void BtnRead(){
	unsigned long now = millis();
	// Проверка нажатия кнопок
if ( now > nextVolTime) {
	if (digitalRead(knVolPlus) == LOW){ // громкость больше
  	  if (VolumeLast < 15) {
  	  	VolumeLast = VolumeLast + 1;
  	    radio.setVolume(VolumeLast);
  	    Serial.print("Громкость: ");
  	    Serial.println(VolumeLast);
  	   }
    }
    else if (digitalRead(knVolMinus) == LOW){ // громкость меньше
  	    if (VolumeLast > 0) {
  	      VolumeLast = VolumeLast - 1;
  	      radio.setVolume(VolumeLast);
  	      Serial.print("Громкость: ");
  	      Serial.println(VolumeLast);
  	    }
  	}
  	else if (digitalRead(knBassBoost) == LOW){
  		radio.setBassBoost(!radio.getBassBoost());
  	}
    
 nextVolTime = now + 500;
 }
} // end BtnRead

void loop() {
  unsigned long now = millis();
 
// Проверка и выполнение команд с ИК пульта
//	ReadIR();

// Проверка нажатия кнопок
  if (digitalRead(knUp) == LOW){  // поиск станции вверх
  	Serial.println("Up"); 
  	radio.seekUp(true);
  }
  if (digitalRead(knDown) == LOW){ // поиск станции вниз
  	Serial.println("Down"); 
  	radio.seekDown(true);
  }
BtnRead();

  // check for RDS data
   radio.checkRDS();

  // обновление дисплея при изменении частоты
  if (now > nextFreqTime) {
    f = radio.getFrequency();
    if (f != lastf) {
      DisplayFrequency(f); // вывод новой частоты настройки
      lastf = f;
    } // if
	radio.getRadioInfo(&StateInfo); // читает параметры приема станции
    r = StateInfo.rssi; // текущий уровень приёма
    if (r != lastrssi) {
      DisplayState(); // вывод уровня приёма
      lastrssi = r;
    }
    nextFreqTime = now + 1000;
  } // if  

} // End loop

// End.
