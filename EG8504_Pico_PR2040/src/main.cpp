#include <Arduino.h>
#include <pico/stdlib.h>
#include "image.h"

#define LCD_height 240
#define LCD_width 80

extern const uint8_t data[];

void setup()
{
  gpio_init_mask(0x0F); // Initialize GPIO pins 0-3
  gpio_set_dir_out_masked(0x0F);

  gpio_init_mask(0x07 << 4); // Initialize GPIO pins 4-6
  gpio_set_dir_out_masked(0x07 << 4);
}

void loop()
{
  for (int i = 0; i < LCD_height; i++)
  {
    for (int j = 0; j < LCD_width; j++)
    {
      uint8_t value = data[j + i * LCD_width]; // Read directly from data array
      gpio_put_masked(0x0F, value & 0x0F);     // Set PORTB equivalent

      gpio_put(4, 1); // XSCL HIGH
      gpio_put(4, 0); // XSCL LOW
    }
    if (i == 0)
    {
      gpio_put(6, 1); // DIN HIGH
    }
    gpio_put(5, 1); // LP HIGH
    gpio_put(5, 0); // LP LOW
    if (i == 0)
    {
      gpio_put(6, 0); // DIN LOW
    }
    sleep_us(10); // Delay in microseconds
  }
}