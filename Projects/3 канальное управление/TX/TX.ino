/*   Данный скетч делает следующее: передатчик (TX) отправляет массив
     данных, который генерируется согласно показаниям с кнопки и с
     двух потенциомтеров. Приёмник (RX) получает массив, и записывает
     данные на реле, сервомашинку и генерирует ШИМ сигнал на транзистор.
    by AlexGyver 2016
*/

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"

RF24 radio(9, 10); // "создать" модуль на пинах 9 и 10 Для Уно
//RF24 radio(9,53); // для Меги

byte address[][6] = {"1Node", "2Node", "3Node", "4Node", "5Node", "6Node"}; //возможные номера труб

byte button = 3;  // кнопка на 3 цифровом
byte potent = 0; // потенциометр на 0 аналоговом
byte slider = 1; // движковый потенциометр на 1 аналоговом пине

byte transmit_data[3]; // массив, хранящий передаваемые данные
byte latest_data[3]; // массив, хранящий последние переданные данные
boolean flag; // флажок отправки данных

void setup() {
  Serial.begin(9600); //открываем порт для связи с ПК

  pinMode(button, INPUT_PULLUP); // настроить пин кнопки

  radio.begin(); //активировать модуль
  radio.setAutoAck(1);         //режим подтверждения приёма, 1 вкл 0 выкл
  radio.setRetries(0, 15);    //(время между попыткой достучаться, число попыток)
  radio.enableAckPayload();    //разрешить отсылку данных в ответ на входящий сигнал
  radio.setPayloadSize(32);     //размер пакета, в байтах

  radio.openWritingPipe(address[0]);   //мы - труба 0, открываем канал для передачи данных
  radio.setChannel(0x60);  //выбираем канал (в котором нет шумов!)

  radio.setPALevel (RF24_PA_MAX); //уровень мощности передатчика. На выбор RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH, RF24_PA_MAX
  radio.setDataRate (RF24_250KBPS); //скорость обмена. На выбор RF24_2MBPS, RF24_1MBPS, RF24_250KBPS
  //должна быть одинакова на приёмнике и передатчике!
  //при самой низкой скорости имеем самую высокую чувствительность и дальность!!

  radio.powerUp(); //начать работу
  radio.stopListening();  //не слушаем радиоэфир, мы передатчик
}

void loop() {

  transmit_data[0] = !digitalRead(button); // инвертированный (!) сигнал с кнопки
  transmit_data[1] = map(analogRead(potent), 0, 1023, 0, 180); // получить значение
  // в диапазоне 0..1023, перевести в 0..180, и записать на 1 место в массиве
  transmit_data[2] = map(analogRead(slider), 0, 1023, 0, 255);

  for (int i = 0; i < 3; i++) { // в цикле от 0 до числа каналов
    if (transmit_data[i] != latest_data[i]) { // если есть изменения в transmit_data
      flag = 1; // поднять флаг отправки по радио
      latest_data[i] = transmit_data[i]; // запомнить последнее изменение
    }
  }

  if (flag == 1) {
    radio.powerUp(); // включить передатчик
    radio.write(&transmit_data, sizeof(transmit_data)); // отправить по радио
    flag = 0; //опустить флаг
    radio.powerDown(); // выключить передатчик
  }

}
