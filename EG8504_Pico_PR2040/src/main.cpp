#include <Arduino.h>     // Arduinoコアライブラリ
#include <pico/stdlib.h> // Raspberry Pi Pico SDK (Arduino-Pico環境で利用可能)
#include "image.h"       // 9600 byte = 320×240×1bit の生データ

// LCDの解像度
static constexpr uint32_t LCD_HEIGHT = 240;
static constexpr uint32_t LCD_WIDTH  = 320;

// ピン割り当て (constexpr を使用)
static constexpr int PIN_XSCL = 8;  // 画素クロック
static constexpr int PIN_LP   = 7;  // 行ラッチ
static constexpr int PIN_DIN  = 6;  // フレーム先頭マーク

static constexpr int DATA0 = 2;
static constexpr int DATA1 = 3;
static constexpr int DATA2 = 4;
static constexpr int DATA3 = 5;

// image.h で定義されている画像データ配列
extern const uint8_t img_ba[];

// XSCL パルス幅調整用の NOP 命令の数
static constexpr int NUM_NOPS_FOR_XSCL_PULSE = 15;

/**
 * @brief 4bitデータを指定されたピンに出力します。
 * 元のコードのロジックを維持しており、val の特定のビットが
 * 特定のデータピンに対応します。
 * (例: DATA0 には val のビット2 が出力されます)
 * @param val 出力するデータを含む値
 */
inline void set_data_pins(uint8_t val) {
    gpio_put(DATA0, ((val) >> 2) & 0x1);
    gpio_put(DATA1, ((val) >> 3) & 0x1);
    gpio_put(DATA2, ((val) >> 4) & 0x1);
    gpio_put(DATA3, ((val) >> 5) & 0x1);
}

/**
 * @brief Arduinoのsetup関数。ハードウェアの初期化を行います。
 */
void setup() {
    // データピン (DATA0-DATA3 / GPIO2-GPIO5) の初期化
    gpio_init(DATA0);
    gpio_set_dir(DATA0, GPIO_OUT);
    gpio_init(DATA1);
    gpio_set_dir(DATA1, GPIO_OUT);
    gpio_init(DATA2);
    gpio_set_dir(DATA2, GPIO_OUT);
    gpio_init(DATA3);
    gpio_set_dir(DATA3, GPIO_OUT);

    // 制御ピン (XSCL, LP, DIN) の初期化
    gpio_init(PIN_XSCL);
    gpio_set_dir(PIN_XSCL, GPIO_OUT);
    gpio_init(PIN_LP);
    gpio_set_dir(PIN_LP, GPIO_OUT);
    gpio_init(PIN_DIN);
    gpio_set_dir(PIN_DIN, GPIO_OUT);
}

/**
 * @brief Arduinoのloop関数。画像データを繰り返しLCDに送信します。
 */
void loop() {
    const uint32_t bytes_per_line = LCD_WIDTH / 8; // 1行あたりのバイト数 (40)

    for (uint32_t y = 0; y < LCD_HEIGHT; ++y) {
        const uint8_t *row_data = &img_ba[y * bytes_per_line]; // 現在の行の画像データ

        // 1行分のデータを送信 (40 バイト = 80 ニブル = 80 XSCL パルス)
        for (uint32_t byte_index = 0; byte_index < bytes_per_line; ++byte_index) {
            uint8_t current_byte = row_data[byte_index];
            int nop_count; // NOPループ用のカウンタ

            // 上位ニブルをD2-D5に出力
            set_data_pins(current_byte >> 4); // 元のロジックに従い、バイトの上位4ビットをvalとして渡す
            gpio_put(PIN_XSCL, 1);
            nop_count = NUM_NOPS_FOR_XSCL_PULSE;
            while (nop_count--) {
                __asm__ volatile("nop"); // XSCL パルス幅のための NOP
            }
            gpio_put(PIN_XSCL, 0);

            // 下位ニブルをD2-D5に出力
            set_data_pins(current_byte);      // 元のロジックに従い、バイト全体をvalとして渡す
            gpio_put(PIN_XSCL, 1);
            nop_count = NUM_NOPS_FOR_XSCL_PULSE;
            while (nop_count--) {
                __asm__ volatile("nop"); // XSCL パルス幅のための NOP
            }
            gpio_put(PIN_XSCL, 0);
        }

        // 行ラッチ & DIN パルス（フレーム先頭だけ DIN=High）
        if (y == 0) {
            gpio_put(PIN_DIN, 1);
        }
        gpio_put(PIN_LP, 1);
        sleep_us(1); // LP パルス幅
        gpio_put(PIN_LP, 0);
        if (y == 0) {
            gpio_put(PIN_DIN, 0);
        }

        // 必要ならウェイト（FPGA や LCD の最小 LP サイクルに合わせる）
        sleep_us(25); // 25マイクロ秒のウェイト（元のコードの値。適宜調整）
    }
}
