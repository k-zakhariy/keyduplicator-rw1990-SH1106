#include <Wire.h>
#include "SH1106Wire.h"
#include <OneWire.h>
#include "fontsRus.h"
#define pin 2
#define PIN_BUTTON_SCAN 12 // D6 пин кнопки сканирования
#define PIN_BUTTON_COPY 14 // D5 пин кнопки копирования

SH1106Wire display(0x3c, 1, 2);
OneWire myWire (pin);

bool button_state_scan = true;
uint32_t ms_button_scan = 0;

bool button_state_copy = true;
uint32_t ms_button_copy = 0;

String keycode_info;
String keycode;

byte addrkeyCodeInPlatform[8]; // массив для ключа, просто вывод кода на экране
byte addrScanKey[8]; // массив для сканируемого ключа
byte addrCopyKey[8]; // массив для копиремого ключа

void setup() {
  Wire.begin();
  Serial.begin(115200);
  pinMode(PIN_BUTTON_SCAN, INPUT);
  pinMode(PIN_BUTTON_COPY, INPUT);
  display.init();
  display.flipScreenVertically();
  display.setFontTableLookupFunction(FontUtf8Rus);
  display.setFont(ArialRus_Plain_10);
  display.setContrast(255);
}

void loop() {
  
  uint32_t ms = millis();

  if ( button_state_scan && button_state_copy && myWire.search(addrkeyCodeInPlatform) ) {
    display.clear();
    display.drawString(35, 0, "КОД КЛЮЧА");
    keycode_info = String(addrkeyCodeInPlatform[0], HEX)+' '+String(addrkeyCodeInPlatform[1], HEX)+' '+
                String(addrkeyCodeInPlatform[2], HEX)+' '+String(addrkeyCodeInPlatform[3], HEX)+' '+
                String(addrkeyCodeInPlatform[4], HEX)+' '+String(addrkeyCodeInPlatform[5], HEX)+' '+
                String(addrkeyCodeInPlatform[6], HEX)+' '+String(addrkeyCodeInPlatform[7], HEX);          
    keycode_info.toUpperCase();
    display.drawString(15, 25, keycode_info); 
    display.display();
    myWire.reset_search();
    return;
  }

  display.clear();
  display.drawString(6, 0, "ВЫБЕРИТЕ ДЕЙСТВИЕ");
  display.drawString(4, 14, "scan - считывание ключа");
  display.drawHorizontalLine(0, 30, 128);
  display.drawString(0, 30, "copy - копирование ключа");
  display.drawRect (0, 48, 128, 16);
  display.drawString(4, 48, "key - ");
  if ( addrScanKey[0] == 0 ) {
    display.drawString(26, 48, "empty");
  } else {
    keycode = String(addrScanKey[0], HEX)+' '+String(addrScanKey[1], HEX)+' '+
              String(addrScanKey[2], HEX)+' '+String(addrScanKey[3], HEX)+' '+
              String(addrScanKey[4], HEX)+' '+String(addrScanKey[5], HEX)+' '+
              String(addrScanKey[6], HEX)+' '+String(addrScanKey[7], HEX);          
    keycode.toUpperCase();
    display.drawString(26, 48, keycode); 
  }
  display.display();

  // Фиксируем нажатие кнопки сканирования
  if ( digitalRead(PIN_BUTTON_SCAN) == LOW && !button_state_scan && ( ms - ms_button_scan ) > 150 ) {
    button_state_scan = true;
    ms_button_scan = ms;

    for (byte y = 0; y < 10; y++) {
      
      String timer;
      timer = String(10 - y);

      display.clear();
      display.drawString(0, 0, "РЕЖИМ СКАНИРОВАНИЕ");
      display.drawString(0, 15, "приложите ключ");
      display.drawString(0, 25, "к платформе");
      display.drawString(60, 45, timer);
      display.display();

      if (myWire.search (addrScanKey)) {
        display.clear();
        display.drawString(0, 20, "Сканирование завершено");
        display.display();
        delay(3000);
        break;
      }
      delay(1000);
     
    }
  }
  
  // Фиксируем отпускание кнопки сканирования
  if ( digitalRead(PIN_BUTTON_SCAN) == HIGH && button_state_scan && ( ms - ms_button_scan ) > 150  ) {
    button_state_scan = false;
    ms_button_scan = ms;
  }

  // Фиксируем нажатие кнопки копирования
  if ( digitalRead(PIN_BUTTON_COPY) == LOW && !button_state_copy && ( ms - ms_button_copy ) > 150 ) {

    //compute crc scan key//
    byte crcScanKeyFirst;
    crcScanKeyFirst = myWire.crc8(addrScanKey, 7);
    
    if ( crcScanKeyFirst == 0 ) {
      display.clear();
      display.drawString(2, 0, "ОШИБКА КОПИРОВАНИЯ");
      display.drawString(33, 22, "В памяти нет");
      display.drawString(2, 35, "отсканированного ключа");
      display.display();
      delay(3000);
      button_state_copy = true;
      ms_button_copy = ms;
      return;
    }

    button_state_copy = true;
    ms_button_copy = ms;

    for (byte z = 0; z < 10; z++) {

      String timer2;
      timer2 = String(10 - z);

      display.clear();
      display.drawString(0, 0, "РЕЖИМ КОПИРОВАНИЕ");
      display.drawString(0, 15, "приложите ключ");
      display.drawString(0, 25, "к платформе");
      display.drawString(60, 45, timer2);
      display.display();

      int u = 0;

      if ( !myWire.search(addrCopyKey) ) {
        
      } else {
        myWire.skip();myWire.reset();myWire.write(0x33);
        
        display.clear();
        display.drawRect ( 1, 22, 100, 12);
        display.display();
        
        myWire.skip();myWire.reset();myWire.write(0xD1);
        digitalWrite(2, LOW); pinMode(2, OUTPUT); delayMicroseconds(60);
        pinMode(2, INPUT); digitalWrite(2, HIGH); delay(10);

        myWire.skip();myWire.reset();myWire.write(0xD5);
        
        for (byte v = 0; v < 8; v++) {
          writeByte(addrScanKey[v]);
          u = u + 12;

          display.fillRect ( 3, 24, u, 8);
          display.drawString(0, 0, "ИДЁТ ЗАПИСЬ!");
          display.drawString(0, 40, "держите ключ");
          display.drawString(0, 50, "до окончания записи!");
          display.display();
          delay(100);
        }
        myWire.reset();
        myWire.write(0xD1);
        digitalWrite(2, LOW); pinMode(2, OUTPUT); delayMicroseconds(10);
        pinMode(2, INPUT); digitalWrite(2, HIGH); delay(10);

        myWire.reset_search();

        if ( myWire.search (addrCopyKey) ) {
          //compute crc scan key//
          byte crcScanKey;
          crcScanKey = myWire.crc8(addrScanKey, 7);
         
          //compute crc copy key//
          byte crcCopyKey;
          crcCopyKey = myWire.crc8(addrCopyKey, 7);
          
          if ( crcScanKey == crcCopyKey ) {
            display.clear();
            display.drawString(0, 20, "Копирование завершено");
            display.display();
            delay(3000);
            break;
          } else {
            display.clear();
            display.drawString(0, 20, "Копирование неудачно!");
            display.display();
            delay(3000);
            break; 
          }
        } else {
          display.clear();
          display.drawString(45, 0, "ОШИБКА");
          display.drawString(20, 30, "Рано убрали ключ!");
          display.display();
          delay(3000);
          break;  
        }
      }
      delay(1000);
    }
  }
  
  // Фиксируем отпускание кнопки копирования
  if ( digitalRead(PIN_BUTTON_COPY) == HIGH && button_state_copy && ( ms - ms_button_copy ) > 150  ) {
    button_state_copy = false;
    ms_button_copy = ms;
  }
  
}

int writeByte(byte data) {
  int data_bit;
  for (data_bit = 0; data_bit < 8; data_bit++) {
    if (data & 1) {
      digitalWrite(pin, LOW); pinMode(pin, OUTPUT);
      delayMicroseconds(60);
      pinMode(pin, INPUT); digitalWrite(pin, HIGH);
      delay(10);
    } else {
      digitalWrite(pin, LOW); pinMode(pin, OUTPUT);
      pinMode(pin, INPUT); digitalWrite(pin, HIGH);
      delay(10);
    }
    data = data >> 1;
  }
  return 0;
}

char FontUtf8Rus(const byte ch) {
  static uint8_t LASTCHAR;

  if ((LASTCHAR == 0) && (ch < 0xC0)) {
    return ch;
  }

  if (LASTCHAR == 0) {
    LASTCHAR = ch;
    return 0;
  }

  uint8_t last = LASTCHAR;
  LASTCHAR = 0;

  switch (last) {
    case 0xD0:
      if (ch == 0x81) return 0xA8;
      if (ch >= 0x90 && ch <= 0xBF) return ch + 0x30;
      break;
    case 0xD1:
      if (ch == 0x91) return 0xB8;
      if (ch >= 0x80 && ch <= 0x8F) return ch + 0x70;
      break;
  }

  return (uint8_t) 0;
}
