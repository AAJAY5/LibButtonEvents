#include <Arduino.h>

#include "Buttons.h"

#define MAX_BUTTON 3

int pins[MAX_BUTTON] = {0, 22, 23};
btn_t *btn[MAX_BUTTON];

void btn_evt(btn_evt_t evt, btn_t *button) {
    int pin = -1;
    for (int i = 0; i < MAX_BUTTON; i++) {
        if (button == btn[i]) {
            pin = pins[i];
            break;
        }
    }
    switch (evt) {
        case BTN_EVT_MULTI_CLICK:
            Serial.printf("Click %d, %u\r\n", pin, button->click_count);
            break;
        case BTN_EVT_PRESS:
            Serial.printf("Press %d\r\n", pin);
            break;
        case BTN_EVT_LONG_PRESS:
            Serial.printf("LongPress %d\r\n", pin);
            break;
        default:
            break;
    }
}

int getState(btn_t *button) {
    for (int i = 0; i < MAX_BUTTON; i++) {
        if (button == btn[i]) {
            return digitalRead(pins[i]);
        }
    }
    return 1;
}

uint32_t mymillis() {
    return millis();
}

void setup() {
    Serial.begin(115200);
    pinMode(0, INPUT);
    pinMode(22, INPUT_PULLUP);
    pinMode(23, INPUT_PULLUP);

    for (int i = 0; i < MAX_BUTTON; i++) {
        btn[i] = btn_add(BTN_ACTIVE_LOW);
        btn_set_read_gpio_func(btn[i], getState);
        btn_set_tick_ms_func(btn[i], mymillis);
        btn_set_evt_func(btn[i], btn_evt);
    }

    btn_enable_evt(btn[0], BTN_EVT_MULTI_CLICK);
    btn_enable_evt(btn[1], BTN_EVT_PRESS);
    btn_enable_evt(btn[2], BTN_EVT_LONG_PRESS | BTN_EVT_MULTI_CLICK);
}

void loop() {
    for (int i = 0; i < MAX_BUTTON; i++) {
        btn_loop(btn[i]);
    }
}
