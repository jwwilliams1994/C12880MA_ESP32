#pragma once
#include <Arduino.h>
#include "print_helper.h"
#include "pin_wrapping.h"


#define SPEC_TRG         13
#define SPEC_ST          15
#define SPEC_CLK         2
#define SPEC_VIDEO       32
#define WHITE_LED        22
#define SPEC_EOS         12
#define SPEC_TRG         13


class c12880ma {
    outputPin clock{SPEC_CLK};
    outputPin start{SPEC_ST};
    outputPin led{WHITE_LED};

    inputPin video{SPEC_VIDEO};
    inputPin trg{SPEC_TRG};
    inputPin eos{SPEC_EOS};
    static const int numChannels{288};
    uint16_t dataIndex{0};
    uint16_t highestSeen{0};
    int16_t integrationValue{0};
    int16_t adjAmt{0};
    int16_t adjAcc{0};
    uint16_t dataBuffer[numChannels]{};
    void appendBuffer(uint16_t _inp) {
        dataBuffer[dataIndex] = _inp;
        dataIndex++;
        if (dataIndex >= numChannels) dataIndex = 0;
    }
    uint16_t data[numChannels]{};
    void pushBuffer() {
        highestSeen = 0;
        for (uint16_t i{0}; i < numChannels; i++) {
            data[i] = dataBuffer[dataIndex];
            if (dataBuffer[dataIndex] > highestSeen) highestSeen = dataBuffer[dataIndex];
            dataIndex++;
            if (dataIndex >= numChannels) dataIndex = 0;
        }
    }
    void trigger(uint16_t integration=0) {
        start.on();
        clock.toggleTimes(6 + integration);
        start.off();
    }

    void autoInt() {
        static uint16_t intCeil{40000};
        if (highestSeen < 3700) {
            if (adjAcc < 200) adjAcc++;
            if (adjAcc < 0) adjAcc = 0;
            if (adjAmt < 0) adjAmt = 0;
        } else if (highestSeen > 4000) {
            if (adjAcc > -200) adjAcc--;
            if (adjAcc > 0) adjAcc = 0;
            if (adjAmt > 0) adjAmt = 0;
        } else {
            adjAcc = 0;
            adjAmt = 0;
        }

        adjAmt += adjAcc;

        if (highestSeen > 4000) {
            if (integrationValue > 0 + abs(adjAmt)) integrationValue += adjAmt;
            if (integrationValue > 0 && integrationValue <= abs(adjAmt)) integrationValue = 0;
        }
        if (highestSeen < 3700) {
            if (integrationValue < intCeil - adjAmt) integrationValue += adjAmt;
            if (integrationValue < intCeil && integrationValue >= intCeil - adjAmt) integrationValue = intCeil;
        }
    }
public:
    uint16_t getNumChannels() { return numChannels; }
    uint16_t* getData() { return data; }
    uint16_t getHighestSeen() { return highestSeen; }
    uint16_t getIntegration() { return integrationValue; }
    int16_t getAdjAmt() { return adjAmt; }
    int16_t getAdjAcc() { return adjAcc; }

    void read() {
        clock.toggleTimes(2);
        trigger(integrationValue);
        clock.toggleTimes(84);
        bool readYet{false};
        while (!eos.digital_read()) {
            if (trg.digital_read()) {
                if (!readYet) {
                    appendBuffer(video.analog_read());
                    readYet = true;
                }
            } else {
                readYet = false;
            }
            clock.toggle();
            delayMicroseconds(1);
        }
        clock.off();
        clock.toggleTimes(4);
        pushBuffer();
        autoInt();
    }
};