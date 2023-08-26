#include "audio.h"
#include <Adafruit_Arcada.h>
#include <Adafruit_SPIFlash.h>

Adafruit_Arcada arcada;
extern Adafruit_FlashTransport_QSPI flashTransport;
extern Adafruit_SPIFlash Arcada_QSPI_Flash;

uint8_t j = 0; // neopixel counter for rainbow

// Check the timer callback, this function is called every millisecond!
volatile uint16_t milliseconds = 0;
void timercallback()
{
    analogWrite(25, milliseconds); // pulse the LED
    if (milliseconds == 0) {
        milliseconds = 255;
    } else {
        milliseconds--;
    }
}

void setup()
{
    Serial.begin(9600);
    // while (!Serial);
    pinMode(25, OUTPUT);

    Serial.println("Hello! Arcada Pico test");
    if (!arcada.arcadaBegin()) {
        Serial.print("Failed to begin");
        while (1)
            ;
    }
    arcada.displayBegin();
    Serial.println("Arcada display begin");

    for (int i = 0; i < 250; i++) {
        // Serial.printf("lite = %d\n", i);
        arcada.setBacklight(i);
        delay(1);
    }

    // Serial.printf("color fills");
    arcada.display->fillScreen(ARCADA_RED);
    delay(100);
    arcada.display->fillScreen(ARCADA_GREEN);
    delay(100);
    arcada.display->fillScreen(ARCADA_BLUE);
    delay(100);
    arcada.display->fillScreen(ARCADA_BLACK);
    delay(100);

    arcada.display->setCursor(0, 0);
    arcada.display->setTextWrap(true);

    /********** Check QSPI manually */
    if (!Arcada_QSPI_Flash.begin()) {
        Serial.println("Could not find flash on QSPI bus!");
        arcada.display->setTextColor(ARCADA_RED);
        arcada.display->println("QSPI Flash FAIL");
    } else {
        uint32_t jedec;

        jedec = Arcada_QSPI_Flash.getJEDECID();
        Serial.print("JEDEC ID: 0x");
        Serial.println(jedec, HEX);
        arcada.display->setTextColor(ARCADA_GREEN);
        arcada.display->print("QSPI Flash OK\nJEDEC: 0x");
        arcada.display->println(jedec, HEX);
    }

    /********** Check filesystem next */
    if (!arcada.filesysBegin()) {
        Serial.println("Failed to load filesys");
        arcada.display->setTextColor(ARCADA_RED);
        arcada.display->println("Filesystem failure");
    } else {
        Serial.println("Filesys OK!");
        arcada.display->setTextColor(ARCADA_GREEN);
        arcada.display->print("Filesystem OK: ");
        arcada.display->print(arcada.filesysListFiles("/"));
        arcada.display->println(" files");
    }

    // TODO: add timer support to Arcada for Pico via Pico SDK.
    arcada.timerCallback(1000, timercallback);
}

void loop()
{
    delay(25); // add some delay so our screen doesnt flicker

    for (int32_t i = 0; i < arcada.pixels.numPixels(); i++) {
        arcada.pixels.setPixelColor(i, Wheel(((i * 256 / arcada.pixels.numPixels()) + j * 5) & 255));
    }
    arcada.pixels.show();
    j++;

    uint32_t pressed_buttons = arcada.readButtons();
    if (pressed_buttons) {
        Serial.print(pressed_buttons, 16);
        Serial.print(" ");
    }

    arcada.display->fillRect(0, 70, 160, 60, ARCADA_BLACK);

    if (pressed_buttons & ARCADA_BUTTONMASK_A) {
        Serial.print("A");
        arcada.display->drawCircle(145, 100, 10, ARCADA_WHITE);
    }
    if (pressed_buttons & ARCADA_BUTTONMASK_B) {
        Serial.print("B");
        arcada.display->drawCircle(120, 100, 10, ARCADA_WHITE);
    }
    if (pressed_buttons & ARCADA_BUTTONMASK_SELECT) {
        Serial.print("C");
        arcada.display->drawRoundRect(60, 100, 20, 10, 5, ARCADA_WHITE);
    }
    if (pressed_buttons & ARCADA_BUTTONMASK_START) {
        Serial.print("R");
        arcada.display->drawRoundRect(85, 100, 20, 10, 5, ARCADA_WHITE);
    }
    if (pressed_buttons & ARCADA_BUTTONMASK_UP) {
        Serial.print("^");
        arcada.display->drawTriangle(15, 86, 35, 86, 25, 76, ARCADA_WHITE);
    }
    if (pressed_buttons & ARCADA_BUTTONMASK_DOWN) {
        Serial.print("v");
        arcada.display->drawTriangle(15, 113, 35, 113, 25, 123, ARCADA_WHITE);
    }
    if (pressed_buttons & ARCADA_BUTTONMASK_LEFT) {
        Serial.print("<");
        arcada.display->drawTriangle(10, 90, 10, 110, 0, 100, ARCADA_WHITE);
    }
    if (pressed_buttons & ARCADA_BUTTONMASK_RIGHT) {
        arcada.display->drawTriangle(40, 90, 40, 110, 50, 100, ARCADA_WHITE);
        Serial.print(">");
    }

    bool playsound = false;
    // no accel, check both SEL/START pressed at once
    if ((pressed_buttons & (ARCADA_BUTTONMASK_START | ARCADA_BUTTONMASK_SELECT)) == (ARCADA_BUTTONMASK_START | ARCADA_BUTTONMASK_SELECT)) {
        playsound = true;
    }

    if (playsound) {
        Serial.print(" Sound ");
        arcada.enableSpeaker(true);
        play_tune(audio, sizeof(audio));
        arcada.enableSpeaker(false);
    }

    if (pressed_buttons) {
        Serial.println();
    }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos)
{
    if (WheelPos < 85) {
        return arcada.pixels.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
    } else if (WheelPos < 170) {
        WheelPos -= 85;
        return arcada.pixels.Color(255 - WheelPos * 3, 0, WheelPos * 3);
    } else {
        WheelPos -= 170;
        return arcada.pixels.Color(0, WheelPos * 3, 255 - WheelPos * 3);
    }
}

void play_tune(const uint8_t* audio, uint32_t audio_length)
{
    // TODO: use PWMAudio library in earlephilhower/arduino-pico to play audio.
    
    uint32_t t;
    uint32_t prior, usec = 1000000L / SAMPLE_RATE;
    analogWriteResolution(8);
    for (uint32_t i = 0; i < audio_length; i++) {
        while ((t = micros()) - prior < usec)
            ;
        analogWrite(ARCADA_AUDIO_OUT, (uint16_t)audio[i] / 8);
        prior = t;
    }
    analogWrite(ARCADA_AUDIO_OUT, 0);
}

/*****************************************************************/
