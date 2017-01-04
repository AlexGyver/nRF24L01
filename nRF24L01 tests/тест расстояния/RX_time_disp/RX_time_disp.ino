#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"

//-----------дисплей-----------
#include "TM1637.h"  //библиотека дисплея
#define CLK 2  //пин дисплея     
#define DIO 3   //пин дисплея 
TM1637 disp(CLK,DIO);   //обозвать дисплей disp
//--------дисплей-------

RF24 radio(9,10); // "создать" модуль на пинах 9 и 10 Для Уно
//RF24 radio(9,53); // для Меги

byte address[][6] = {"1Node","2Node","3Node","4Node","5Node","6Node"};  //возможные номера труб
byte pipeNo, gotByte; 

void setup(){
  Serial.begin(9600);
  pinMode(4,OUTPUT);
  digitalWrite(4,0);
  disp.init();  //инициализация дисплея
  disp.set(6);  //яркость (0-7) 

  radio.begin(); //активировать модуль
  radio.setAutoAck(0);         //режим подтверждения приёма, 1 вкл 0 выкл
  radio.setRetries(0,15);     //(время между попыткой достучаться, число попыток)

  radio.setPayloadSize(32);     //размер пакета, в байтах

  radio.openReadingPipe(1,address[0]);      //хотим слушать трубу 0
  radio.setChannel(0x60);  //выбираем канал (в котором нет шумов!)

  radio.setPALevel (RF24_PA_MAX); //уровень мощности передатчика. На выбор RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH, RF24_PA_MAX
  radio.setDataRate (RF24_250KBPS); //скорость обмена. На выбор RF24_2MBPS, RF24_1MBPS, RF24_250KBPS
  //должна быть одинакова на приёмнике и передатчике!
  //при самой низкой скорости имеем самую высокую чувствительность и дальность!!

  radio.powerUp(); //начать работу
  radio.startListening();  //начинаем слушать эфир, мы приёмный модуль
}

void black_print(String x) {
  disp.point(POINT_OFF);
  switch (x.length()) {         //кароч тут измеряется длина строки и соотвествено выводится всё на дисплей
  case 1:
    disp.display(0,18);
    disp.display(1,18);
    disp.display(2,18);
    disp.display(3,x[0]- '0');
    break;
  case 2:
    disp.display(0,18);
    disp.display(1,18);
    disp.display(2,x[0]- '0');
    disp.display(3,x[1]- '0');
    break;
  case 3:
    disp.display(0,18);
    disp.display(1,x[0]- '0');
    disp.display(2,x[1]- '0');
    disp.display(3,x[2]- '0');
    break;
  }
}

void loop() {
  int count = 0;
  volatile long timer = millis();
  while (millis() - timer < 2000) {
    while( radio.available(&pipeNo)){    // слушаем эфир со всех труб
      radio.read( &gotByte, 1 );         // чиатем входящий сигнал
      count++;
    }    
  }
  Serial.println(count);
  String nums = String(round(count));
  black_print(nums);
}

