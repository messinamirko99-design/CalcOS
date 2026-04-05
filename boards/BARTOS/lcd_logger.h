#pragma once
#ifdef USE_LCD_LOGGER
#include <tftLogger.h>

// LcdLogger: subclass of tft_logger with NO new data members.
// Overrides key display methods to mirror output to a 16x2 I2C LCD.
// Binary-layout compatible with tft_logger (same sizeof).
class LcdLogger : public tft_logger {
public:
    LcdLogger(int16_t w = TFT_WIDTH, int16_t h = TFT_HEIGHT)
        : tft_logger(w, h) {}

    // NO new member variables — sizeof(LcdLogger) == sizeof(tft_logger)

    // Un-hide all base-class print/println overloads (char, int, double, etc.)
    // Without these, overriding print(const String&) would hide them all (C++ name hiding).
    using tft_logger::print;
    using tft_logger::println;

    void     fillScreen(int32_t color) override;
    int16_t  drawString(const String &s, int32_t x, int32_t y, uint8_t font = 1) override;
    int16_t  drawCentreString(const String &s, int32_t x, int32_t y, uint8_t font = 1) override;
    int16_t  drawRightString(const String &s, int32_t x, int32_t y, uint8_t font = 1) override;
    size_t   print(const String &s) override;
    size_t   println(const String &s) override;
    size_t   println(void) override;
};

#endif // USE_LCD_LOGGER
