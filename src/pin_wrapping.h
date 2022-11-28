#pragma once
#include <Arduino.h>

class pinWrap {
public:
    int pin;
    explicit pinWrap(int _pin) {
        pin = _pin;
    }
};

class inputInterface {
public:
    bool invert_output{false};
    int analog{0};
    bool digital{false};
    virtual bool digital_read() { return false; }
    virtual void update() { return; }
    virtual int analog_read() { return 0; };
};

class inputPin : public pinWrap, public inputInterface {
public:

    inputPin(int _pin, bool pullup=false) : pinWrap(_pin) {
        // pinMode(_pin, pullup ? INPUT_PULLUP : INPUT);
        pinMode(_pin, INPUT);
        // if (pullup) {
        //     digitalWrite(_pin, HIGH);
        // }
    }

    inputPin(int _pin, bool pullup, bool _invert_output) : pinWrap(_pin) {
        pinMode(_pin, INPUT);
        if (pullup) {
            digitalWrite(_pin, HIGH);
        }
        invert_output = _invert_output;
    }

    virtual bool digital_read() override {
        digital = digitalRead(pin);
        if (invert_output) { // inverting the output is great for making pullup reads make more intuitive sense as closed = true
            digital = !digital;
        }
        return digital;
    }

    virtual int analog_read() override {
        analog = analogRead(pin);
        return analog;
    }
};

class outputPin : public pinWrap {
public:
    bool digital = false;
    byte analog = 0;
    outputPin(int _pin) : pinWrap(_pin) {
        pinMode(this->pin, OUTPUT);
    }

    virtual void digital_write(bool _pin_state) {
        this->digital = _pin_state;
        digitalWrite(this->pin, _pin_state ? HIGH : LOW);
    }

    virtual void update() {}

    virtual void trigger() {
        digital_write(true);
    }

    virtual void reset() {
        digital_write(false);
    }

    virtual void on() {
        digital_write(true);
    }

    virtual void off() {
        digital_write(false);
    }

    void toggle() {
        digital_write(!digital);
    }

    void toggleTimes(uint16_t _num, uint16_t _delay=1) {
        for (uint16_t i = 0; i < (_num * 2); i++) {
            delayMicroseconds(_delay);
            toggle();
        }
    }
};