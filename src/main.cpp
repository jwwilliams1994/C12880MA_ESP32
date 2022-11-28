#include <Arduino.h>
#include "display.h"
#include "print_helper.h"
#include "c12880ma.h"


#define BAUDRATE 115200


c12880ma spectrometer;

void setup() {
    Serial.begin(BAUDRATE);
    println("Initializing...");
    my_display.initialize();
    my_display.update_display();
}

unsigned long m_threshold{300};
unsigned long m_frequency{8000};
unsigned long m_counter{0};

unsigned long d_threshold{1};
unsigned long d_frequency{6};

void loop() {
    if (millis() > m_threshold) {
        print_time();
        println(m_counter);
        m_counter = 0;
        m_threshold += m_frequency;
    }
    if (millis() >= d_threshold) {
        spectrometer.read();

        my_display.reset_buffer();
        my_display.my_buffer.graphData(spectrometer.getData(), spectrometer.getNumChannels(), 0.0f, 4400.0f);

        my_display.my_buffer.setCursorPos(0, 0);
        String my_string{""};
        my_string += String(spectrometer.getIntegration());

        my_display.my_buffer.print(my_string, my_font, rgb{0, 128, 255}, 2);

        // my_display.my_buffer.setCursorPos(0, my_font.fontDict['0'].height * 2 + 2);

        // my_string = "";
        // my_string += String(spectrometer.getCount());

        // my_display.my_buffer.print(my_string, my_font, rgb{0, 128, 255}, 2);

        // my_display.my_buffer.setCursorPos(0, my_display.my_buffer.getCursorY() + my_font.fontDict['0'].height * 2 + 2);

        // my_string = "";
        // my_string += String(spectrometer.getNots());

        // my_display.my_buffer.print(my_string, my_font, rgb{0, 128, 255}, 2);

        my_display.update_display();

        d_threshold = millis() + d_frequency;
    }
}