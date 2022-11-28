#pragma once
#include <Arduino.h>
#include "calibration_map.h"

struct rgb {
    byte r;
    byte g;
    byte b;
    rgb() : r{0}, g{0}, b{0} {}
    rgb(byte _r, byte _g, byte _b) : r{_r}, g{_g}, b{_b} {}
    void reset() {
        r = 0;
        g = 0;
        b = 0;
    }
};

uint32_t rgb_to_565(uint16_t red, uint16_t green, uint16_t blue) {
    return (((red * 31) / 255) << 11) + (((green * 63) / 255) << 5) + (((blue * 31) / 255) << 0);
}

uint32_t rgb_to_565(rgb _rgb) {
    return (((_rgb.r * 31) / 255) << 11) + (((_rgb.g * 63) / 255) << 5) + (((_rgb.b * 31) / 255) << 0);
}

struct rgb565 {
    uint16_t val;
    rgb565() : val{0} {}
    rgb565(byte _r, byte _g, byte _b) : val{rgb_to_565(rgb{_r, _g, _b})} {}
    rgb565(rgb _col) : val{rgb_to_565(_col)} {}
    void reset() {
        val = 0;
    }
};

struct hsv {
    float h; // 0 to 360
    float s; // 0 to 1
    float v; // 0 to 1
};

void print(hsv _inp) {
    Serial.print('[');
    Serial.print(_inp.h);
    Serial.print(", ");
    Serial.print(_inp.s);
    Serial.print(", ");
    Serial.print(_inp.v);
    Serial.print(']');
}

void print(rgb _inp) {
    Serial.print('[');
    Serial.print(_inp.r);
    Serial.print(", ");
    Serial.print(_inp.g);
    Serial.print(", ");
    Serial.print(_inp.b);
    Serial.print(']');
}

rgb hsv_to_rgb(hsv _hsv) {
    float c = _hsv.v * _hsv.s;
    float x = c * (1.0f - abs((float(uint16_t(_hsv.h * 10.0f) % 1200) / 600.0f) - 1.0f));
    float m = _hsv.v - c;
    while (_hsv.h > 360) _hsv.h -= 360.0f;
    if (_hsv.h < 0) _hsv.h += 360.0f;
    float _buff[3]{0};
    if (_hsv.h >= 0 && _hsv.h < 60) {
        _buff[0] = c; _buff[1] = x;
    } else if (_hsv.h >= 60 && _hsv.h < 120) {
        _buff[0] = x; _buff[1] = c;
    } else if (_hsv.h >= 120 && _hsv.h < 180) {
        _buff[1] = c; _buff[2] = x;
    } else if (_hsv.h >= 180 && _hsv.h < 240) {
        _buff[1] = x; _buff[2] = c;
    } else if (_hsv.h >= 240 && _hsv.h < 300) {
        _buff[0] = x; _buff[2] = c;
    } else { // else h is between 300 and 360
        _buff[0] = c; _buff[2] = x;
    }
    return rgb{byte((_buff[0] + m) * 255.0f), byte((_buff[1] + m) * 255.0f), byte((_buff[2] + m) * 255.0f)};
}

uint32_t hsv_to_565(hsv _hsv) {
    rgb _buff{hsv_to_rgb(_hsv)};
    return rgb_to_565(_buff.r, _buff.g, _buff.b);
}

int8_t threshold_map[4][4]{ // for ordered dithering
    {0, 8, 2, 10},
    {12, 4, 14, 6},
    {3, 11, 1, 9},
    {15, 7, 13, 5}
};

const float rgb_diff[3]{255.0f / 32.0f, 255.0f / 64.0f, 255.0f / 32.0f};
// const float rgb_diff2[3]{32, 64, 32};

uint32_t rgb_to_565_dithered(uint16_t _x, uint16_t _y, rgb _rgb) {
    // _x %= 4;
    byte xmod = _x % 4;
    byte ymod = _y % 4;
    // _y %= 4;
    float err{(float(threshold_map[xmod][ymod]) / 16.0f) - 0.5f};
    int16_t _red = int16_t(_rgb.r) + int16_t(err * rgb_diff[0]);
    _red = constrain(_red, 0, 255);
    int16_t _green = int16_t(_rgb.g) + int16_t(err * rgb_diff[1]);
    _green = constrain(_green, 0, 255);
    int16_t _blue = int16_t(_rgb.b) + int16_t(err * rgb_diff[2]);
    _blue = constrain(_blue, 0, 255);
    return rgb_to_565(_red, _green, _blue);
}

rgb nmToRGB(float _w) {
    float _r{0};
    float _g{0};
    float _b{0};
    if (_w >= 380.0f && _w < 440.0f) {
        _r = -(_w - 440.0f) / (440.0f - 380.0f);
        _g = 0.0f;
        _b = 1.0f;
    } else if (_w >= 440.0f && _w < 490.0f) {
        _r = 0.0f;
        _g = (_w - 440.0f) / (490.0f - 440.0f);
        _b = 1.0f;
    } else if (_w >= 490.0f && _w < 510.0f) {
        _r = 0.0f;
        _g = 1.0f;
        _b = -(_w - 510.0f) / (510.0f - 490.0f);
    } else if (_w >= 510.0f && _w < 580.0f) {
        _r = (_w - 510.0f) / (580.0f - 510.0f);
        _g = 1.0f;
        _b = 0.0f;
    } else if (_w >= 580.0f && _w < 645.0f) {
        _r = 1.0f;
        _g = -(_w - 645.0f) / (645.0f - 580.0f);
        _b = 0.0f;
    } else if (_w >= 645.0f && _w < 781.0f) {
        _r = 1.0f;
        _g = 0.0f;
        _b = 0.0f;
    } else {
        _r = 0.0f;
        _g = 0.0f;
        _b = 0.0f;
    }

    float factor{0.0f};
    if (_w >= 380.0f && _w < 420.0f) {
        factor = 0.3f + (0.7f * ((_w - 380.0f) / (420.0f - 380.0f)));
    } else if (_w >= 420.0f && _w < 701.0f) {
        factor = 1.0f;
    } else  if (_w >= 701.0f && _w < 781.0f) {
        factor = 0.3 + (0.7 * ((780.0f - _w) / (780.0f - 700.0f)));
    } else {
        factor = 0.3f;
    }
    _r = _r > 0.0f ? 255.0f * _r * factor : 0.0f;
    _g = _g > 0.0f ? 255.0f * _g * factor : 0.0f;
    _b = _b > 0.0f ? 255.0f * _b * factor : 0.0f;

    if (_r == 0 && _g == 0 && _b == 0) {
        _r = _b = _g = 50.0f;
    }


    return rgb{byte(_r), byte(_g), byte(_b)};
}
