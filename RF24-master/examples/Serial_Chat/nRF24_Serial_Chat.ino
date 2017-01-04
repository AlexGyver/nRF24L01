/*
nRF Serial Chat

Date : 29 Sep 2015
Author : Stanley Seow
e-mail : stanleyseow@gmail.com
Version : 1.00
Desc : 
I worte this simple interactive serial chat over nRF that can be used for both sender 
and receiver as I swapped the TX & RX addr during read/write operation.

It read input from Serial Monitor and display the output to the other side
Serial Monitor or 16x2 LCD (if available)... like a simple chat program.

Max payload is 32 bytes for radio but the serialEvent will chopped the entire buffer
for next payload to be sent out sequentially.

*/

#define I2C_LCD

#ifdef I2C_LCD
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x3F,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display
//LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display
#endif

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"


RF24 radio(8,9);

const uint64_t pipes[2] = { 0xDEDEDEDEE7LL, 0xDEDEDEDEE9LL };

boolean stringComplete = false;  // whether the string is complete
static int dataBufferIndex = 0;
boolean stringOverflow = false;
char charOverflow = 0;

char SendPayload[31] = "";
char RecvPayload[31] = "";
char serialBuffer[31] = "";

void setup(void) {
 
  Serial.begin(9600);
  
#ifdef I2C_LCD
  lcd.init(); 
  lcd.backlight();
  lcd.begin(16,2);
  lcd.print("RF Chat V1.0");
  delay(500);
  //lcd.clear();
#endif

  printf_begin();
  radio.begin();
  
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_MAX);
  radio.setChannel(70);
  
  radio.enableDynamicPayloads();
  radio.setRetries(15,15);
  radio.setCRCLength(RF24_CRC_16);

  radio.openWritingPipe(pipes[0]);
  radio.openReadingPipe(1,pipes[1]);  
  
  radio.startListening();
  radio.printDetails();

  Serial.println();
  Serial.println("RF Chat V01.0");    
  delay(500);
  lcd.clear();
}

void loop(void) {
  
  nRF_receive();
  serial_receive();
  
} // end loop()

void serialEvent() {
  while (Serial.available() > 0 ) {
      char incomingByte = Serial.read();
      
      if (stringOverflow) {
         serialBuffer[dataBufferIndex++] = charOverflow;  // Place saved overflow byte into buffer
         serialBuffer[dataBufferIndex++] = incomingByte;  // saved next byte into next buffer
         stringOverflow = false;                          // turn overflow flag off
      } else if (dataBufferIndex > 31) {
         stringComplete = true;        // Send this buffer out to radio
         stringOverflow = true;        // trigger the overflow flag
         charOverflow = incomingByte;  // Saved the overflow byte for next loop
         dataBufferIndex = 0;          // reset the bufferindex
         break; 
      } 
      else if(incomingByte=='\n'){
          serialBuffer[dataBufferIndex] = 0; 
          stringComplete = true;
      } else {
          serialBuffer[dataBufferIndex++] = incomingByte;
          serialBuffer[dataBufferIndex] = 0; 
      }          
  } // end while()
} // end serialEvent()

void nRF_receive(void) {
  int len = 0;
  if ( radio.available() ) {
        len = radio.getDynamicPayloadSize();
        radio.read(&RecvPayload,len);
        delay(5);
  
    RecvPayload[len] = 0; // null terminate string
    
#ifdef I2C_LCD    
    lcd.setCursor(0,0);
    lcd.print("R:              ");
    lcd.setCursor(2,0);
    lcd.print(RecvPayload);
#endif    
    Serial.print("R:");
    Serial.print(RecvPayload);
    Serial.println();
    RecvPayload[0] = 0;  // Clear the buffers
  }  

} // end nRF_receive()

void serial_receive(void){
  
  if (stringComplete) { 
        strcat(SendPayload,serialBuffer);      
        // swap TX & Rx addr for writing
        radio.openWritingPipe(pipes[1]);
        radio.openReadingPipe(0,pipes[0]);  
        radio.stopListening();
        radio.write(&SendPayload,strlen(SendPayload));
        
#ifdef I2C_LCD            
        lcd.setCursor(0,1);
        lcd.print("S:              ");
        lcd.setCursor(2,1);
        lcd.print(SendPayload);
#endif        

        Serial.print("S:");  
        Serial.print(SendPayload);          
        Serial.println();
        stringComplete = false;

        // restore TX & Rx addr for reading       
        radio.openWritingPipe(pipes[0]);
        radio.openReadingPipe(1,pipes[1]); 
        radio.startListening();  
        SendPayload[0] = 0;
        dataBufferIndex = 0;
  } // endif
} // end serial_receive()    



