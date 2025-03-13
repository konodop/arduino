#include "Adafruit_GFX.h" // библиотеки для работы с адресной лентой
#include "Adafruit_NeoMatrix.h"
#include "Adafruit_NeoPixel.h"
#include <Wire.h>
#include <iarduino_RTC.h>
iarduino_RTC watch(RTC_DS1302,6,7,8);  // для модуля DS1302 - RST, CLK, DAT
#include "DHT.h" // библиотека для датчика DHT11

#define DHT_PIN 2 // порт подключения датчика DHT11
#define LED_PIN 9  // порт подключения адресной ленты
#define WIDTH 30  // ширина матрицы в пикселях
#define ROWS 8      // количество строк в матрице
boolean humidity = true;          // выводить влажность воздуха - true/false
unsigned int watchr = 50;             // скорость бегущей строки
unsigned int watchrs = 80;             // скорость бегущей строки слов
unsigned int pausatb = 5000;   // время табло для волейбольных счётов
unsigned int pausa1 = 5000;   // на сколько мсек остановится время на табло
unsigned int pausa2 = 1000;     // на сколько мсек остановится температура и влажность
unsigned int brightness = 100;  // яркость матрицы с адресной лентой

// настройки матрицы (если текст выводится некорректно, следует их изменить)
Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(WIDTH, ROWS, 1, 1, LED_PIN,
                            NEO_MATRIX_RIGHT     + NEO_MATRIX_BOTTOM +
                            NEO_MATRIX_ROWS + NEO_MATRIX_ZIGZAG,
                            NEO_GRB            + NEO_KHZ800);

DHT dht(DHT_PIN, DHT11); // создаем объект для датчика температуры
byte w, t, h;
int lsignal=0;
int llastsignal=0;
int rsignal=0;
int rlastsignal=0;
byte mode = 0;
int x = WIDTH;
int left = 0;
int right = 0;

void setup() {
  dht.begin();              // запускаем датчик давления
  Serial.begin(9600); // запускаем монитор порта

  matrix.begin();
  matrix.fillScreen(0);
  // установка цветовой палитры адресной ленты (RGB)
  matrix.setTextColor(matrix.Color(0, 255, 0));
  matrix.setTextWrap(false);
  matrix.setBrightness(brightness);

  watch.begin(&Wire);
  // после установки времени и загрузки скетча в плату Ардуино
  // закомментируйте следующую строчку и снова загрузите скетч в плату
//  watch.settime(__TIMESTAMP__); // сек,  мин,  час, число, месяц, год, день недели
}
void loop() {
  Serial.println(watch.gettime("H:i"));
  variant(); // первый вариант с бегущей строкой
}

String utf8rus(String source)
{
  int i,k;
  String target;
  unsigned char n;
  char m[2] = { '0', '\0' };

  k = source.length(); i = 0;

  while (i < k) {
    n = source[i]; i++;

    if (n >= 0xBF){
      switch (n) {
        case 0xD0: {
          n = source[i]; i++;
          if (n == 0x81) { n = 0xA8; break; }
          if (n >= 0x90 && n <= 0xBF) n = n + 0x2F;
          break;
        }
        case 0xD1: {
          n = source[i]; i++;
          if (n == 0x91) { n = 0xB7; break; }
          if (n >= 0x80 && n <= 0x8F) n = n + 0x6F;
          break;
        }
      }
    }
    m[0] = n; target = target + String(m);
  }
return target;
}


void scores() {
  matrix.setBrightness(brightness);
  matrix.fillScreen(0);
  w = 1;
  byte e = 0;
  while (w == 1) {
    // левый
    lsignal=digitalRead(11);
     if (lsignal!=llastsignal)
     {
      if (lsignal==HIGH)
      {
        e = 1;
        left=left+1;
      }
      llastsignal=lsignal;
      if (left==16)
      {
        left=0;
        right=0;
      }
     }
    // правый
    rsignal=digitalRead(13);
     if (rsignal!=rlastsignal)
     {
      if (rsignal==HIGH)
      {
        e = 1;
        right=right+1;
      }
      rlastsignal=rsignal;
      if (right==16)
      {
        right=0;
        left=0;
      }
     }
    // отображение на матрицу
    matrix.fillScreen(0);
    matrix.setCursor(x, 0);
    if (left < 10) {
      matrix.print(" ");
    }
    matrix.print(left);
    matrix.print(":");
    matrix.print(right);
    matrix.show();
    delay(watchr);
    if (x > 0){
      x--;
    } else{
      if (digitalRead(12)) { x = WIDTH; w = 0;}
    }
  }
}
void variant() {
  matrix.setBrightness(brightness);
  matrix.fillScreen(0);
  w = 1;

  // == выводим время (часы и минуты) на светодиодное табло == //
  while (w == 1) {
    matrix.fillScreen(0);
    matrix.setCursor(x, 0);
    matrix.print(watch.gettime("H:i"));
    matrix.show();
    delay(watchr);
    int buttonState = digitalRead(12);
    if (buttonState) {
      mode = 1;
      scores();
    }
    x--;
    // когда текст оказывается посередине табло - делаем паузу
    if (x == -1) { delay(pausa1); }
    if (x < - WIDTH) { x = WIDTH; w = 2; }
  }

  // == выводим температуру воздуха на светодиодное табло == //
  while (w == 2) {
    t = dht.readTemperature();
    matrix.fillScreen(0);
    matrix.setCursor(x, 0);
    matrix.print(t);
    matrix.print("'C");
    matrix.show();
    delay(watchr);
    int buttonState = digitalRead(12);
    if (buttonState) {
      mode = 1;
      scores();
    }
    x--;
    // когда текст оказывается посередине табло - делаем паузу
    if (x == 3) { delay(pausa2); }
    if (x < - WIDTH && humidity == true) { x = WIDTH; w = 3; }
    if (x < - WIDTH && humidity == false) { x = WIDTH; w = 0; }
  }

  // == выводим влажность воздуха на светодиодное табло == //
  while (w == 3) {
    h = dht.readHumidity();
    matrix.fillScreen(0);
    matrix.setCursor(x, 0);
    matrix.print(h);
    matrix.print("%");
    matrix.show();
    delay(watchr);
    int buttonState = digitalRead(12);
    if (buttonState) {
      mode = 1;
      scores();
    }
    x--;
    // когда текст оказывается посередине табло - делаем паузу
    if (x == 6) { delay(pausa2); }
    if (x < - WIDTH) { x = WIDTH; w = 4;}
  }
  while (w == 4) {
    matrix.fillScreen(0);
    matrix.setCursor(x, 0);
//    matrix.setFont(&FreeSerif6pt8b); // выбор шрифта
    matrix.print(utf8rus("Тест"));
//    matrix.setFont(); // выбор шрифта по умолчанию
    matrix.print("Test");
    matrix.show();
    delay(watchrs);
    int buttonState = digitalRead(12);
    if (buttonState) {
      mode = 1;
      scores();
    }
    x--;
    if (x < -200) { x = WIDTH; w = 5;}
  }
}
