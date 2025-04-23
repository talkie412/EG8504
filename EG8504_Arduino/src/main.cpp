#include <Arduino.h>
#include <avr/pgmspace.h>
#include "image.h"

#define LCD_height 240
#define LCD_width 80

extern const uint8_t data[];

void setup()
{

  DDRB |= B00001111;
  DDRC |= B00000111;
}

void loop()
{

  for (int i = 0; i < LCD_height; i++)
  {
    for (int j = 0; j < LCD_width; j++)
    {
      uint8_t value = pgm_read_byte(&data[j + i * LCD_width]); // PROGMEM から読み込み
      PORTB = value;
      PORTC |= B00000001; // XSCL HIGH
      PORTC &= B11111110; // XSCL LOW
    }
    if (i == 0)
      PORTC |= B00000100; // DIN HIGH
    PORTC |= B00000010;   // LP HIGH
    PORTC &= B11111101;   // LP LOW
    if (i == 0)
      PORTC &= B11111011; // DIN LOW
    delayMicroseconds(10);
  }
}
