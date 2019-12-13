/*Тестовый скетч для проверки радио-модуля и кнопок
после переноса на макетную печатную плату

18.08.2019

*/
#include <radio.h>
#include <RDA5807M.h>
#include <RDSParser.h>

//#include <SPI.h>
#include <Wire.h>

int ledPin = 13; // СД на плате Arduino
int knUp = 2;    // кнопка cледующ. станция
int knDown = 3;    // кнопка предыд. станция
int knMode = 5;    // переключатель режима

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

/// Вывод частоты настройки на Serial и дисплей.
void DisplayFrequency(RADIO_FREQ f)
{
  char s[12];
  radio.formatFrequency(s, sizeof(s));
  Serial.print("FREQ:"); Serial.println(s);
} // DisplayFrequency()

void DisplayState()
{
  char RDS_en = '-'; // R есть RDS, - нет RDS
  char St_en = '-';  // S есть стерео, - нет стерео
  radio.getRadioInfo(&StateInfo); // читает параметры приема станции
  Serial.print("RSSI: "); Serial.println(StateInfo.rssi);
  if (StateInfo.rds) {RDS_en = 'R';}
  if (StateInfo.stereo){St_en = 'S';}
  Serial.print(RDS_en); Serial.println(St_en);

} // DisplayState()

/// Update the ServiceName text on the display.
void DisplayServiceName(char *name)
{
  Serial.print("RDS:");
  Serial.println(name);
 } // DisplayServiceName()

void RDS_process(uint16_t block1, uint16_t block2, uint16_t block3, uint16_t block4) {
  rds.processData(block1, block2, block3, block4);
} // RDS_process

void setup()
{
 //*** инициализация пинов kn1, kn2, ledPin
  pinMode(knUp, INPUT);
  pinMode(knDown, INPUT);
  pinMode(knMode, INPUT);
  pinMode(ledPin, OUTPUT);

  // open the Serial port
  Serial.begin(9600);
  Serial.println("Radio...");
  delay(500);

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
  radio.setVolume(5);
  // setup the information chain for RDS data.
  radio.attachReceiveRDS(RDS_process);
  rds.attachServicenNameCallback(DisplayServiceName);
	
}

void loop()
{
  unsigned long now = millis();
  if (digitalRead(knUp) == LOW){
  	Serial.println("Up"); 
  	radio.seekUp(true);
  }
  if (digitalRead(knDown) == LOW){
  	Serial.println("Down"); 
  	radio.seekDown(true);
    }

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
} // loop

