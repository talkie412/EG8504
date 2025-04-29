#include <Arduino.h> // Arduino-Pico でも pico-sdk でも可
#include <pico/stdlib.h>
#include "image.h" // 9600 byte = 320×240×1bit の生データ

#define LCD_HEIGHT 240
#define LCD_WIDTH 320

// ピン割り当て
#define PIN_XSCL 4 // 画素クロック
#define PIN_LP 5   // 行ラッチ
#define PIN_DIN 6  // フレーム先頭マーク

// データバス (GPIO0−3) 用マスク
#define BUS_MASK 0x0F

extern const uint8_t img[]; // image.h で定義

void setup()
{
  // D0-D3
  gpio_init_mask(BUS_MASK);
  gpio_set_dir_out_masked(BUS_MASK);

  // XSCL, LP, DIN
  gpio_init(PIN_XSCL);
  gpio_set_dir(PIN_XSCL, GPIO_OUT);
  gpio_init(PIN_LP);
  gpio_set_dir(PIN_LP, GPIO_OUT);
  gpio_init(PIN_DIN);
  gpio_set_dir(PIN_DIN, GPIO_OUT);
}

void loop()
{
  const uint32_t bytes_per_line = LCD_WIDTH / 8; // 40
  const uint32_t n_pulses_line = LCD_WIDTH / 4;  // 80

  for (uint32_t y = 0; y < LCD_HEIGHT; ++y)
  {
    const uint8_t *row = &img[y * bytes_per_line];

    // 40 バイト = 80 ニブル = 80 XSCL パルス
    for (uint32_t byte_ix = 0; byte_ix < bytes_per_line; ++byte_ix)
    {
      uint8_t b = row[byte_ix];

      // 上位・下位ニブルをそのまま D3-D0 に出力
      gpio_put_masked(BUS_MASK, (b >> 4) & BUS_MASK);
      gpio_put(PIN_XSCL, 1);
      gpio_put(PIN_XSCL, 0);

      gpio_put_masked(BUS_MASK, b & BUS_MASK);
      gpio_put(PIN_XSCL, 1);
      gpio_put(PIN_XSCL, 0);
    }

    // 行ラッチ & DIN パルス（フレーム先頭だけ DIN=High）
    if (y == 0)
      gpio_put(PIN_DIN, 1);
    gpio_put(PIN_LP, 1);
    gpio_put(PIN_LP, 0);
    if (y == 0)
      gpio_put(PIN_DIN, 0);

    // 必要ならウェイト（FPGA や LCD の最小 LP サイクルに合わせる）
    sleep_us(500);
  }
}
