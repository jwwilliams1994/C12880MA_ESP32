#pragma once
#include <Arduino.h>
#include <SPI.h>
#include <FS.h>
#include <SPIFFS.h>
#include <TFT_eSPI.h>
#include "print_helper.h"
#include "my_font.h"
#include "color_helper.h"


#define WIDTH 135
#define HEIGHT 240

class frameBufferSpr {
    byte cursorPos[2]{0, 0};
    TFT_eSprite buffer;
public:
    int width, height, rotation;
    frameBufferSpr(int _width, int _height, int _rotate, TFT_eSPI& _disp) : buffer{&_disp}, width{_width}, height{_height}, rotation{_rotate % 4} {
        if (rotation % 2 == 1) {
            width = _height;
            height = _width;
        }
        buffer.createSprite(_width, _height);
    }
    void scroll(int _inp) {
        switch (rotation) {
            case 0:
                buffer.scroll(0, _inp);
                break;
            case 1:
                buffer.scroll(-_inp, 0);
                break;
            case 2:
                buffer.scroll(0, -_inp);
                break;
            case 3:
                buffer.scroll(_inp, 0);
                break;
        }
    }
    void scroll(char _inp, myFont& _font, byte _textScale=1) {
        fontData myData = _font.fontDict[(byte)_inp];
        this->scroll((myData.height * _textScale) + 1);
    }
    void drawPixel(int x, int y, rgb _inp) {
        if (x >= width || x < 0) return;
        if (y >= height || y < 0) return;
        byte x2{0}, y2{0};
        switch (rotation) {
            case 0:
                x2 = x;
                y2 = y;
                break;
            case 1:
                x2 = (height - 1) - y;
                y2 = x;
                break;
            case 2:
                x2 = (width - 1) - x;
                y2 = (height - 1) - y;
                break;
            case 3:
                x2 = y;
                y2 = (width - 1) - x;
                break;
        }
        buffer.drawPixel(x2, y2, rgb_to_565_dithered(x2, y2, _inp));
    }
    void drawText(int start_x, int start_y, String _inp, myFont &_font, rgb _textRGB, byte _textScale=1) {
        byte num_chars = _inp.length();
        int x_offset{0};
        for (byte i = 0; i < num_chars; i++) {
            char my_char = _inp[i];
            fontData font_data = _font.fontDict[(byte)my_char];
            uint64_t char_data = font_data.data;
            byte charSize[2]{font_data.width, font_data.height};
            x_offset += charSize[0] + 1;
            for (int y = 0; y < charSize[1]; y++) {
                for (int x = 0; x < charSize[0]; x++) {
                    byte index = (y * charSize[0]) + x;
                    if ((char_data & (1 << index)) == 0) continue;

                    int _x = (x * _textScale) + start_x + (x_offset * _textScale);
                    int _y = (y * _textScale) + start_y;
                    for (byte x2 = 0; x2 < _textScale; x2++) {
                        for (byte y2 = 0; y2 < _textScale; y2++) {
                            drawPixel(_x + x2, _y + y2, _textRGB);
                        }
                    }
                }
            }
        }
    }
    void setCursorPos(byte _x, byte _y) {
        cursorPos[0] = _x;
        cursorPos[1] = _y;
    }
    byte getCursorX() { return cursorPos[0]; }
    byte getCursorY() { return cursorPos[1]; }
    void print(char _inp, myFont &_font, rgb _textRGB, byte _textScale=1) {
        fontData myData = _font.fontDict[(byte)_inp];
        byte index = 0;
        int y_offset = constrain(myData.yoffset, 0, 10);
        for (int y = 0; y < myData.height; y++) {
            for (int x = 0; x < myData.width; x++) {
                if ((myData.data & (uint64_t(1) << index)) == 0) {
                    index++;
                    continue;
                } else {
                    index++;
                }

                int _x = (x * _textScale) + cursorPos[0];
                int _y = ((y + y_offset) * _textScale) + cursorPos[1];
                for (byte x2 = 0; x2 < _textScale; x2++) {
                    for (byte y2 = 0; y2 < _textScale; y2++) {
                        drawPixel(_x + x2, _y + y2, _textRGB);
                    }
                }
            }
        }
        cursorPos[0] += (myData.width * _textScale) + 1;
    }
    void print(String _inp, myFont &_font, rgb _textRGB, byte _textScale=1) {
        byte num_chars = _inp.length();
        fontData myData = _font.fontDict['0'];
        for (byte i = 0; i < num_chars; i++) {
            char my_char = _inp[i];
            this->print(my_char, _font, _textRGB, _textScale);
        }
    }
    void println(String _inp, myFont &_font, rgb _textRGB, byte _textScale=1) {
        byte num_chars = _inp.length();
        fontData myData = _font.fontDict['0'];
        scroll((myData.height * _textScale) + _textScale + 1);
        for (byte i = 0; i < num_chars; i++) {
            char my_char = _inp[i];
            this->print(my_char, _font, _textRGB, _textScale);
        }
        cursorPos[0] = 0;
    }
    void drawVerticalGradient(hsv _hsv, uint16_t _gradient) {
        float mod = float(height) / float(_gradient);
        float xmod = 1.0f / float(width);
        rgb _buff{};
        hsv hsv_buff{0, 1.0f, 0.5f};
        float hue{0};
        for (byte y = 0; y < height; y++) {
            hue = float(y) * mod;
            hsv_buff.h = hue + _hsv.h;
            for (byte x = 0; x < width; x++) {
                hsv_buff.v = float(x) * xmod;
                hsv_buff.s = float(x) * xmod;
                _buff = hsv_to_rgb(hsv_buff);
                drawPixel(x, y, _buff);
            }
        }
    }
    void drawLine(int x_start, int y_start, int x_end, int y_end, rgb lineColor) {
        int x_range{x_end - x_start};
        int y_range{y_end - y_start};
        if (x_range != 0.0f) {
            float slope{float(y_range) / float(x_range)};
            float xslope = 1.0f;
            if (y_range != 0.0f) xslope = abs(float(x_range) / float(y_range));
            if (xslope > 1.0f) xslope = 1.0f;
            for (float x{0}; x <= x_range; x += xslope) {
                int y = int(x * slope);
                drawPixel(int(x) + x_start, y + y_start, lineColor);
            }
        } else {
            float yLower = y_start < y_end ? y_start : y_end;
            for (float y{0}; y <= y_range; y += 1.0f) {
                drawPixel(x_start, y + yLower, lineColor);
            }
        }
    }
    void drawPoint(byte _x, byte _y, byte _radius=1, rgb color=rgb{255, 255, 255}) {
        for (int x{-_radius}; x <= _radius; x++) {
            for (int y{-_radius}; y <= _radius; y++) {
                drawPixel(_x + x, _y + y, color);
                drawPixel(_x + x, _y + y, color);
            }
        }
    }
    void graphData(uint16_t* inp, uint16_t _len, float _min, float _max) {
        rgb _lineColor{160, 160, 160};
        bool drawDebugPoints{false};
        int numPoints{_len};
        float y_highest{_max};
        float y_lowest{_min};
        float x_highest{pixelToNmMap[287]};
        float x_lowest{pixelToNmMap[0]};
        rgb lineColor{255, 128, 128};
        for (int16_t i{0}; i < numPoints; i++) {
            if (float(inp[i]) > y_highest) y_highest = float(inp[i]);
            if (float(inp[i]) < y_lowest) y_lowest = float(inp[i]);
        }
        if (x_highest == 0.0f) return;
        float y_range = (y_highest - y_lowest);
        float x_range = (x_highest - x_lowest);
        if (x_range == 0.0f) return;
        float x_scale{239.0f / x_range};
        float y_scale{134.0f / y_range};
        float nm{0.0f};
        for (float i{0.0f}; i < x_highest; i += 100.0f) {
            int x_start = (i - x_lowest) * x_scale;
            if (x_start < 0) continue;
            drawLine(x_start, 0, x_start, 135, rgb{64, 64, 64});
        }
        drawLine(round((445.0f - x_lowest) * x_scale), 0, round((445.0f - x_lowest) * x_scale), 135, rgb{0, 0, 128});
        drawLine(round((537.0f - x_lowest) * x_scale), 0, round((537.0f - x_lowest) * x_scale), 135, rgb{0, 128, 0});
        drawLine(round((630.0f - x_lowest) * x_scale), 0, round((630.0f - x_lowest) * x_scale), 135, rgb{128, 0, 0});
        for (int16_t i{0}; i < numPoints - 1; i++) {

            nm = pixelToNmMap[constrain(i, 0, 288)];
            _lineColor = nmToRGB(nm);

            float x_1 = nm;
            float x_2 = pixelToNmMap[i + 1];
            int x_start = round((x_1 - x_lowest) * x_scale);
            int y_start = round((float(inp[i]) - y_lowest) * y_scale);
            int x_end = round((x_2 - x_lowest) * x_scale);
            int y_end = round((float(inp[i + 1]) - y_lowest) * y_scale);
            y_start = 134 - y_start;
            y_end = 134 - y_end;
            drawLine(x_start, y_start, x_end, y_end, _lineColor);
            if (drawDebugPoints) drawPoint(x_start, y_start);
        }
        setCursorPos(0, getCursorY() + 80);
    }
    void pushToDisplay() {
        buffer.pushSprite(0, 0);
    }
    void resetBuffer() {
        buffer.fillScreen(0);
    }
};


class myDisplay {
    TFT_eSPI tft;
    uint16_t width, height;
    hsv textColor{0, 1.0f, 1.0f};
    rgb textRGB;
    uint16_t framesDue{0};
    uint16_t framesAvail{0};
public:
    frameBufferSpr my_buffer;
    myDisplay(int _width, int _height) : tft{_width, _height}, width{_width}, height{_height}, my_buffer{_width, _height, 3, tft} {}
    void reset() {
        tft.fillScreen(TFT_BLACK);
    }
    void reset_buffer() {
        my_buffer.resetBuffer();
        my_buffer.setCursorPos(0, 0);
    }
    void initialize() {
        tft.begin();
        reset();
    }
    void update_display() {
        my_buffer.pushToDisplay();
    }
};

myDisplay my_display(WIDTH, HEIGHT);