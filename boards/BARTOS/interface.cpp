/*
 * BARTOS interface.cpp
 * Display: I2C LCD 16x2 (auto-detected: 0x27, 0x3F, 0x20, 0x38)
 *          SDA = LCD_SDA_PIN (default 21), SCL = LCD_SCL_PIN (default 22)
 * Input:   BTN_SCROLL_PIN (default GPIO 0)  → NextPress on each press
 *          BTN_SELECT_PIN (default GPIO 26) → SelPress on short press
 *                                             EscPress on long press (>700ms)
 *
 * Features:
 *   - I2C auto-detect: scans 0x27, 0x3F, 0x20, 0x38 at boot
 *   - Backlight timeout: turns off LCD backlight after BACKLIGHT_TIMEOUT_MS inactivity
 *   - Marquee scroll: labels/strings longer than 16 chars scroll horizontally (non-blocking)
 *
 * Bruce interface hooks used: _setup_gpio(), _post_setup_gpio(), InputHandler()
 */
#include "lcd_logger.h"
#ifdef USE_LCD_LOGGER

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <globals.h>
#include <functional>

// ─── Pin defaults ──────────────────────────────────────────────────────────
#ifndef BTN_SCROLL_PIN
#define BTN_SCROLL_PIN 0
#endif
#ifndef BTN_SELECT_PIN
#define BTN_SELECT_PIN 26
#endif
#ifndef LCD_SDA_PIN
#define LCD_SDA_PIN 21
#endif
#ifndef LCD_SCL_PIN
#define LCD_SCL_PIN 22
#endif
#ifndef LCD_I2C_ADDR
#define LCD_I2C_ADDR 0x27
#endif

// Backlight turns off after this many ms of button inactivity (30 seconds)
#ifndef BACKLIGHT_TIMEOUT_MS
#define BACKLIGHT_TIMEOUT_MS (30 * 1000UL)
#endif

// ─── LCD object (pointer — address resolved at runtime via I2C scan) ────────
static LiquidCrystal_I2C* _lcd      = nullptr;
static char                _lcdBuf[2][17];
static bool                _lcdReady = false;

// ─── Backlight state ────────────────────────────────────────────────────────
static bool     _backlightOn    = true;
static uint32_t _lastActivityMs = 0;

// ─── Marquee state (non-blocking horizontal scroll for labels > 16 chars) ───
static String   _marqueeText    = "";
static uint8_t  _marqueeRow     = 0;
static int      _marqueePos     = 0;
static uint32_t _marqueeLastMs  = 0;
static bool     _marqueeActive  = false;
#define MARQUEE_STEP_MS   320    // ms between each shift
#define MARQUEE_PAUSE_MS  1400   // pause at end before restarting

// ─── Internal helpers ───────────────────────────────────────────────────────

// Truncate/pad string to 16 chars and write to LCD row
static void _lcdWriteRow(uint8_t row, const String &s) {
    if (!_lcdReady || !_lcd) return;
    memset(_lcdBuf[row], ' ', 16);
    _lcdBuf[row][16] = '\0';
    size_t len = s.length() > 16 ? 16 : s.length();
    memcpy(_lcdBuf[row], s.c_str(), len);
    _lcd->setCursor(0, row);
    _lcd->print(_lcdBuf[row]);
}

// Map TFT y-coordinate to LCD row (0 = top half, 1 = bottom half)
static uint8_t _yToRow(int32_t y) {
    return (y < TFT_HEIGHT / 2) ? 0 : 1;
}

// ─── Marquee public API (called from display.cpp via extern declaration) ────

void _lcdStartMarquee(uint8_t row, const String &text) {
    if (!_lcdReady || (int)text.length() <= 16) return;
    _marqueeText   = text;
    _marqueeRow    = row;
    _marqueePos    = 0;
    _marqueeActive = true;
    _marqueeLastMs = 0; // trigger on next InputHandler tick
}

void _lcdStopMarquee() {
    _marqueeActive = false;
}

// Called from InputHandler() — advances the marquee one step if due
static void _updateMarquee() {
    if (!_marqueeActive || !_lcdReady || !_lcd) return;
    uint32_t now = millis();
    if (now - _marqueeLastMs < MARQUEE_STEP_MS) return;

    int len = (int)_marqueeText.length();
    if (len <= 16) { _marqueeActive = false; return; }

    // Build 16-char window starting at _marqueePos
    String view = _marqueeText.substring(_marqueePos, _marqueePos + 16);
    while ((int)view.length() < 16) view += ' ';

    // Write directly to LCD (bypasses _lcdBuf to keep buffer as the "base" state)
    _lcd->setCursor(0, _marqueeRow);
    _lcd->print(view.c_str());

    _marqueePos++;
    if (_marqueePos > len - 16) {
        // Reached end: restart after pause
        _marqueePos    = 0;
        _marqueeLastMs = now + MARQUEE_PAUSE_MS;
    } else {
        _marqueeLastMs = now;
    }
}

// ─── LcdLogger method implementations ──────────────────────────────────────

void LcdLogger::fillScreen(int32_t color) {
    if (_lcdReady && _lcd) {
        _lcdStopMarquee();
        _lcd->clear();
        memset(_lcdBuf[0], ' ', 16); _lcdBuf[0][16] = '\0';
        memset(_lcdBuf[1], ' ', 16); _lcdBuf[1][16] = '\0';
    }
    tft_logger::fillScreen(color);
}

int16_t LcdLogger::drawString(const String &s, int32_t x, int32_t y, uint8_t font) {
    _lcdStopMarquee();
    _lcdWriteRow(_yToRow(y), s);
    return tft_logger::drawString(s, x, y, font);
}

int16_t LcdLogger::drawCentreString(const String &s, int32_t x, int32_t y, uint8_t font) {
    _lcdStopMarquee();
    _lcdWriteRow(_yToRow(y), s);
    return tft_logger::drawCentreString(s, x, y, font);
}

int16_t LcdLogger::drawRightString(const String &s, int32_t x, int32_t y, uint8_t font) {
    _lcdStopMarquee();
    _lcdWriteRow(_yToRow(y), s);
    return tft_logger::drawRightString(s, x, y, font);
}

size_t LcdLogger::print(const String &s) {
    if (_lcdReady) {
        _lcdStopMarquee();
        // Update row 1 in-place (no scroll — partial line output)
        _lcdWriteRow(1, s);
    }
    return tft_logger::print(s);
}

size_t LcdLogger::println(const String &s) {
    if (_lcdReady) {
        _lcdStopMarquee();
        // Scroll: row1 → row0, new text → row1
        _lcdWriteRow(0, String(_lcdBuf[1]));
        _lcdWriteRow(1, s);
    }
    return tft_logger::println(s);
}

size_t LcdLogger::println(void) {
    if (_lcdReady) {
        _lcdStopMarquee();
        // Blank newline: scroll row1 → row0, clear row1
        _lcdWriteRow(0, String(_lcdBuf[1]));
        _lcdWriteRow(1, "");
    }
    return tft_logger::println();
}

// ─── Button state ───────────────────────────────────────────────────────────
static bool     _btnScrollPrev  = HIGH;
static bool     _btnSelectPrev  = HIGH;
static uint32_t _selectPressMs  = 0;
static bool     _selectHeld     = false;
#define DEBOUNCE_MS  50
#define LONGPRESS_MS 700

// ─── Bruce interface hooks ──────────────────────────────────────────────────

/***************************************************************************************
** Function name: _setup_gpio()
** Description:   Initial GPIO setup (buttons) — called early in Bruce main
***************************************************************************************/
void _setup_gpio() {
    pinMode(BTN_SCROLL_PIN, INPUT_PULLUP);
    pinMode(BTN_SELECT_PIN, INPUT_PULLUP);
}

/***************************************************************************************
** Function name: _post_setup_gpio()
** Description:   Post-setup: I2C auto-detect + LCD init — called after Bruce core starts
***************************************************************************************/
void _post_setup_gpio() {
    Wire.begin(LCD_SDA_PIN, LCD_SCL_PIN);

    // Auto-detect I2C LCD address (try common PCF8574 backpack addresses)
    uint8_t addr = 0;
    const uint8_t candidates[] = {0x27, 0x3F, 0x20, 0x38};
    for (uint8_t a : candidates) {
        Wire.beginTransmission(a);
        if (Wire.endTransmission() == 0) { addr = a; break; }
    }
    if (!addr) addr = LCD_I2C_ADDR; // fallback to compile-time default

    _lcd = new LiquidCrystal_I2C(addr, 16, 2);
    _lcd->init();
    _lcd->backlight();
    _backlightOn    = true;
    _lastActivityMs = millis();

    memset(_lcdBuf[0], ' ', 16); _lcdBuf[0][16] = '\0';
    memset(_lcdBuf[1], ' ', 16); _lcdBuf[1][16] = '\0';
    _lcdReady = true;

    // Splash screen
    _lcdWriteRow(0, "    BARTOS");
    _lcdWriteRow(1, "  Bruce fw");
    delay(1200);
    _lcd->clear();
}

/***************************************************************************************
** Function name: InputHandler()
** Description:   Called periodically from Bruce task — polls buttons, runs marquee/backlight
***************************************************************************************/
void InputHandler(void) {
    static uint32_t lastScroll = 0;
    static uint32_t lastSelect = 0;
    uint32_t now = millis();

    // ── Marquee tick (non-blocking) ───────────────────────────────────────
    _updateMarquee();

    // ── Backlight timeout ─────────────────────────────────────────────────
    if (_lcdReady && _lcd) {
        if (_backlightOn && (now - _lastActivityMs > BACKLIGHT_TIMEOUT_MS)) {
            _lcd->noBacklight();
            _backlightOn = false;
        }
    }

    // Wake backlight on any button activity
    auto _wakeBacklight = [&]() {
        _lastActivityMs = now;
        if (!_backlightOn && _lcdReady && _lcd) {
            _lcd->backlight();
            _backlightOn = true;
        }
    };

    // ── SCROLL button (next) ──────────────────────────────────────────────
    bool scrollCur = digitalRead(BTN_SCROLL_PIN);
    if (_btnScrollPrev == HIGH && scrollCur == LOW && (now - lastScroll > DEBOUNCE_MS)) {
        lastScroll = now;
        _wakeBacklight();
        NextPress = true;
    }
    _btnScrollPrev = scrollCur;

    // ── SELECT button (select / back) ─────────────────────────────────────
    bool selectCur = digitalRead(BTN_SELECT_PIN);

    if (_btnSelectPrev == HIGH && selectCur == LOW) {
        // Falling edge: start timing press
        if (now - lastSelect > DEBOUNCE_MS) {
            _selectPressMs = now;
            _selectHeld    = false;
            lastSelect     = now;
            _wakeBacklight();
        }
    }

    if (selectCur == LOW && !_selectHeld) {
        // Still held down: check for long press
        if ((now - _selectPressMs) > LONGPRESS_MS) {
            _selectHeld = true;
            EscPress    = true;
        }
    }

    if (_btnSelectPrev == LOW && selectCur == HIGH) {
        // Rising edge: short press if not already handled as long press
        if (!_selectHeld) {
            SelPress = true;
        }
        _selectHeld = false;
    }

    _btnSelectPrev = selectCur;
}

#endif // USE_LCD_LOGGER
