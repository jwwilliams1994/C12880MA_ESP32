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
    static const int buffLen{289};
    uint16_t dataIndex{0};
    uint16_t highestSeen{0};
    int32_t integrationValue{0};
    int32_t adjAmt{0};
    int32_t adjAcc{0};
    uint16_t dataBuffer[buffLen]{};
    void appendBuffer(uint16_t _inp) {
        dataBuffer[dataIndex] = _inp;
        dataIndex++;
        if (dataIndex >= buffLen) dataIndex = 0;
    }
    uint16_t data[numChannels]{};
    void pushBuffer() {
        highestSeen = 0;
        for (uint16_t i{0}; i < numChannels; i++) {
            data[i] = dataBuffer[dataIndex];
            if (dataBuffer[dataIndex] > highestSeen) highestSeen = dataBuffer[dataIndex];
            dataIndex++;
            if (dataIndex >= buffLen) dataIndex = 0;
        }
    }
    void trigger(int32_t integration=0) {
        start.on();
        clock.toggleTimes(6 + integration);
        start.off();
    }

    void autoInt() {
        static int32_t intCeil{300000};
        int32_t adjBy = integrationValue / 500;
        if (adjBy < 1) adjBy = 1;
        if (highestSeen < 3700) {
            if (adjAcc < 200) adjAcc += adjBy;
            if (adjAcc < 0) adjAcc = 0;
            if (adjAmt < 0) adjAmt = 0;
        } else if (highestSeen > 4000) {
            if (adjAcc > -200) adjAcc -= adjBy;
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
    uint16_t countReads{0};
    uint16_t countNots{0};
public:
    uint16_t getNumChannels() { return numChannels; }
    uint16_t* getData() { return data; }
    uint16_t getHighestSeen() { return highestSeen; }
    int32_t getIntegration() { return integrationValue; }
    int32_t getAdjAmt() { return adjAmt; }
    int32_t getAdjAcc() { return adjAcc; }
    uint16_t getCount() { return countReads; }
    uint16_t getNots() { return countNots; }

    void read() {
        clock.toggleTimes(2);
        trigger(integrationValue);
        clock.toggleTimes(87);
        bool readYet{false};
        // uint16_t breakAfter{288};
        countReads = 0;
        countNots = 0;
        // uint16_t aRead{0};
        uint16_t bRead{0};
        while (!eos.digital_read()) {
            if (!trg.digital_read()) {
                if (!readYet) {
                    countReads++;
                    // aRead = video.analog_read();
                    // delayMicroseconds(1);
                    bRead = video.analog_read();
                    appendBuffer(bRead);
                    readYet = true;
                    // breakAfter--;
                    // if (breakAfter == 0) break;
                }
            } else {
                readYet = false;
                countNots++;
            }
            clock.toggle();
            delayMicroseconds(1);
        }
        clock.off();
        clock.toggleTimes(8);
        pushBuffer();
        autoInt();
    }
};