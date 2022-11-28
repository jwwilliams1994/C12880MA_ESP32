#pragma once
#include <Arduino.h>

#define FALSE 0
#define TRUE 1
#define PRINT_TCP FALSE

#if PRINT_TCP == TRUE
#include <Ethernet.h>
EthernetServer tcp_server{1337};
EthernetClient tcp_client;

template <class A>
void tcp_printf(A _inp, int _format) {
    tcp_client = tcp_server.available();
    if (tcp_client.connected()) {
        tcp_client.print(_inp, _format);
    }
}
template <class A>
void tcp_print(A _inp) {
    tcp_client = tcp_server.available();
    if (tcp_client.connected()) {
        tcp_client.print(String(_inp));
    }
}
void tcp_println() {
    tcp_client = tcp_server.available();
    if (tcp_client.connected()) {
        tcp_client.println();
    }
}

#endif
#if PRINT_TCP == FALSE
template <class A>
void tcp_printf(A _inp, int _format) {}
template <class A>
void tcp_print(A _inp) {}
void tcp_println() {}

#endif

static byte get_hours() {
    return millis() / 3600000 % 100;
}

static byte get_minutes() {
    return millis() / 60000 % 60;
}

static byte get_seconds() {
    return millis() / 1000 % 60;
}

template <class A>
void printf(A _inp, int _format) {
    Serial.print(_inp, _format);
    tcp_printf(_inp, _format);
}

template <class A>
void printlnf(A _inp, int _format) {
    Serial.print(_inp, _format);
    Serial.println();
    tcp_printf(_inp, _format);
    tcp_println();
}

template <class A>
void print(A _inp) {
    Serial.print(_inp);
    tcp_print(_inp);
}

template <class A, class... B>
void print(A _inp, B... _inp2) {
    print(_inp);
    print(_inp2...);
}

template <class A>
void println(A _inp) {
    print(_inp);
    Serial.println();
    tcp_println();
}

template <class A, class... B>
void println(A _inp, B... _inp2) {
    print(_inp);
    print(_inp2...);
    Serial.println();
    tcp_println();
}

void println() {
    Serial.println();
    tcp_println();
}

void print_time() {
    byte hours = get_hours();
    byte minutes = get_minutes();
    byte seconds = get_seconds();
    if (hours < 10) print(0);
    print(hours, F(":"));
    if (minutes < 10) print(0);
    print(minutes, F(":"));
    if (seconds < 10) print(0);
    print(seconds, F(" > "));
}

String get_time() {
    byte hours = get_hours();
    byte minutes = get_minutes();
    byte seconds = get_seconds();
    return String(hours) + ":" + String(minutes) + ":" + String(seconds);
}