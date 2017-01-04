#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"

RF24 radio(9,10); // инициализировать модуль на пинах 9 и 10 Для Уно
//RF24 radio(9,53);// Для Меги

const uint8_t num_channels = 128;
uint8_t values[num_channels];
void setup(void)
{
  Serial.begin(9600);
  printf_begin();
  radio.begin();
  radio.setAutoAck(false);
  radio.startListening();

  radio.printDetails();  // Вот эта строка напечатает нам что-то, если все правильно соединили.
  delay(5000);              // И посмотрим на это пять секунд.

  radio.stopListening();
  int i = 0;    // А это напечатает нам заголовки всех 127 каналов
  while ( i < num_channels )  {
    printf("%x",i>>4);
    ++i;
  }
  printf("\n\r");
  i = 0;
  while ( i < num_channels ) {
    printf("%x",i&0xf);
    ++i;
  }
  printf("\n\r");
}
const int num_reps = 100;

void loop(void)
{
  memset(values,0,sizeof(values));
  int rep_counter = num_reps;
  while (rep_counter--) {
    int i = num_channels;
    while (i--) {
      radio.setChannel(i);
      radio.startListening();
      delayMicroseconds(128);
      radio.stopListening();
      if ( radio.testCarrier() )
        ++values[i];
    }
  }
  int i = 0;
  while ( i < num_channels ) {
    printf("%x",min(0xf,values[i]&0xf));
    ++i;
  }
  printf("\n\r");
}
int serial_putc( char c, FILE * ) {
  Serial.write( c );
  return c;
}

void printf_begin(void) {
  fdevopen( &serial_putc, 0 );
}
 
