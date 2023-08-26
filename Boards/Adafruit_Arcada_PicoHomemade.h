#ifndef __ARCADA_PICO_HOMEMADE__
#define __ARCADA_PICO_HOMEMADE__
#if defined(ARDUINO_RASPBERRY_PI_PICO) || defined(ARDUINO_RASPBERRY_PI_PICO_W)

#include <Adafruit_ST7789.h>

#define ARCADA_TFT_SPI SPI
#define ARCADA_TFT_MISO 16 // Display Data in, not used
#define ARCADA_TFT_MOSI 19 // Display Data out, connect to DIN on screen
#define ARCADA_TFT_SCLK 18 // Display Clock out

#define ARCADA_TFT_CS 17 // Display CS Arduino pin number
#define ARCADA_TFT_DC 20 // Display D/C Arduino pin number
#define ARCADA_TFT_RST 21 // Display reset Arduino pin number
#define ARCADA_TFT_LITE 22 // Display backlight pin number

#define ARCADA_TFT_ROTATION 3
#define ARCADA_TFT_DEFAULTFILL 0x7BEF
#define ARCADA_TFT_WIDTH 320
#define ARCADA_TFT_HEIGHT 240

// #define ARCADA_SPEAKER_ENABLE 51
#define ARCADA_NEOPIXEL_PIN 7
#define ARCADA_NEOPIXEL_NUM 8
#define ARCADA_AUDIO_OUT A0

#define ARCADA_USE_JSON

// define keypad pins
#define ARCADA_KEYPAD_R1 12
#define ARCADA_KEYPAD_R2 13
#define ARCADA_KEYPAD_R3 14
#define ARCADA_KEYPAD_R4 15
#define ARCADA_KEYPAD_C1 11
#define ARCADA_KEYPAD_C2 10
#define ARCADA_KEYPAD_C3 9
#define ARCADA_KEYPAD_C4 8

#include "arcadatype.h"

class Adafruit_Arcada : public Adafruit_Arcada_SPITFT {
private:
    static const uint8_t ARCADA_KEYPAD_ROWS = 4;
    static const uint8_t ARCADA_KEYPAD_COLS = 4;

    // Define the buttons of the keypad. Keypad is 4x4 but we only need 8 buttons.
    // Hold keypad diagonally and use the 8 buttons in each 2x2 corner of the keypad.
    // Define 2 sets of buttons so keypad can be held in either of 2 diagonal orientations.
    //
    // "R" is staRt, "C" is seleCt, "+" are the pins, ASCII arrow points to top orientation:
    //
    //    . .  R A  *--
    // +  . .  C B  |\  
    // +            | \ 
    // +  U R  . .     \ 
    //    L D  . .
    //
    // or
    //
    //    A B  . .
    // +  R C  . .     /
    // +            | /
    // +  . .  R D  |/
    //    . .  U L  *--
    //
    static constexpr uint8_t arcadaKeypadKeys[ARCADA_KEYPAD_ROWS][ARCADA_KEYPAD_COLS] = {
        { ARCADA_BUTTONMASK_A, ARCADA_BUTTONMASK_B, ARCADA_BUTTONMASK_START, ARCADA_BUTTONMASK_A },
        { ARCADA_BUTTONMASK_START, ARCADA_BUTTONMASK_SELECT, ARCADA_BUTTONMASK_SELECT, ARCADA_BUTTONMASK_B },
        { ARCADA_BUTTONMASK_UP, ARCADA_BUTTONMASK_RIGHT, ARCADA_BUTTONMASK_RIGHT, ARCADA_BUTTONMASK_DOWN },
        { ARCADA_BUTTONMASK_LEFT, ARCADA_BUTTONMASK_DOWN, ARCADA_BUTTONMASK_UP, ARCADA_BUTTONMASK_LEFT }
    };
    static constexpr pin_size_t arcadaKeypadRowPins[ARCADA_KEYPAD_ROWS] = {
        ARCADA_KEYPAD_R1, ARCADA_KEYPAD_R2, ARCADA_KEYPAD_R3, ARCADA_KEYPAD_R4
    };
    static constexpr pin_size_t arcadaKeypadColPins[ARCADA_KEYPAD_COLS] = {
        ARCADA_KEYPAD_C1, ARCADA_KEYPAD_C2, ARCADA_KEYPAD_C3, ARCADA_KEYPAD_C4
    };

    static const uint8_t ARCADA_KEYPAD_SETTLING_DELAY = 20;

public:
    Adafruit_Arcada(void) {};

    bool variantBegin(void)
    {
        for (uint8_t i = 0; i < ARCADA_KEYPAD_COLS; i++) {
            pinMode(arcadaKeypadColPins[i], OUTPUT);
            digitalWrite(arcadaKeypadColPins[i], HIGH);
        }

        for (uint8_t i = 0; i < ARCADA_KEYPAD_ROWS; i++) {
            pinMode(arcadaKeypadRowPins[i], INPUT_PULLUP);
        }

        return true;
    }

    uint32_t variantReadButtons(void)
    {
        uint32_t buttons = 0;
        for (uint8_t i = 0; i < ARCADA_KEYPAD_COLS; i++) {
            digitalWrite(arcadaKeypadColPins[i], HIGH);
        }

        for (uint8_t c = 0; c < ARCADA_KEYPAD_COLS; c++) {
            digitalWrite(arcadaKeypadColPins[c], LOW);
            delayMicroseconds(ARCADA_KEYPAD_SETTLING_DELAY);
            for (uint8_t r = 0; r < ARCADA_KEYPAD_ROWS; r++) {
                if (!digitalRead(arcadaKeypadRowPins[r])) {
                    buttons = buttons | arcadaKeypadKeys[r][c];
                }
            }
            digitalWrite(arcadaKeypadColPins[c], HIGH);
        }

        return buttons;
    }

    void displayBegin(void)
    {
        ARCADA_TFT_SPI.setRX(ARCADA_TFT_MISO);
        ARCADA_TFT_SPI.setTX(ARCADA_TFT_MOSI);
        ARCADA_TFT_SPI.setSCK(ARCADA_TFT_SCLK);
        ARCADA_TFT_SPI.setCS(ARCADA_TFT_CS);

        Adafruit_ST7789* tft = new Adafruit_ST7789(&ARCADA_TFT_SPI, ARCADA_TFT_CS, ARCADA_TFT_DC, ARCADA_TFT_RST);
        tft->init(240, 320);
        tft->setRotation(ARCADA_TFT_ROTATION);
        tft->fillScreen(ARCADA_TFT_DEFAULTFILL);
        display = tft;
    }
};

#endif
#endif
