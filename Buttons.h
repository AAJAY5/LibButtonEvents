#ifndef _BUTTONS_H_
#define _BUTTONS_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>


/// Button events
typedef enum {
    BTN_EVT_MULTI_CLICK = 1,    ///< Multiple click event
    BTN_EVT_PRESS       = 2,    ///< Short press event
    BTN_EVT_LONG_PRESS  = 4,    ///< Long press event
    BTN_EVT_ALL         = (BTN_EVT_MULTI_CLICK | BTN_EVT_PRESS | BTN_EVT_LONG_PRESS),   ///< All event
} btn_evt_t;

/// @brief typedef for btn
typedef struct btn_t btn_t;

/// @brief get tick count in ms callback
typedef uint32_t (*tick_ms_cb_t)(void);

/// @brief get gpio pin status callback
typedef int (*gpio_read_cb_t)(btn_t *btn);

/// @brief gpio event callback
typedef void (*btn_evt_cb_t)(btn_evt_t evt, btn_t *btn);

/// @brief Button active state
typedef enum {
    BTN_ACTIVE_LOW,     ///< Active low (PULL_UP)
    BTN_ACTIVE_HIGH,    ///< Active High (PULL_DOWN)
} btn_active_state_t;

/// @brief hold button runtime data
struct btn_t {
    uint32_t click_ms;
    uint32_t press_ms;
    uint32_t debounce_ms;
    uint32_t long_press_ms;
    uint32_t click_count;
    uint32_t events;
    uint32_t tick;
    uint32_t duration;
    int old_state;
    int cur_state;
    int new_state;

    btn_active_state_t active_state;
    gpio_read_cb_t gpio_read_cb;
    tick_ms_cb_t tick_ms_cb;
    btn_evt_cb_t btn_evt_cb;
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Check button clicked/pressed or not
 * 
 * @param btn pointer to btn
 * @return true if pressed
 * @return false not pressed
 */
static bool is_btn_pressed(btn_t *btn) {
    return (btn->cur_state == btn->active_state);
}

/**
 * @brief create button with active state.
 * 
 * @param s active state
 * @return btn_t* if success else NULL
 * @note default time ms: 
 *          - debounce_ms = 20 
 *          - click_ms = 200
 *          - press_ms = 500
 *          - long_press_ms = 2000
 */
btn_t *btn_add(btn_active_state_t s) {
    btn_t *btn = (btn_t *)calloc(1, sizeof(btn_t));
    if (btn == NULL)
        return NULL;
    btn->debounce_ms    = 20;
    btn->click_ms       = 200;
    btn->press_ms       = 500;
    btn->long_press_ms  = 2000;
    btn->active_state   = s;
    btn->old_state      = (btn->active_state == BTN_ACTIVE_LOW) ? BTN_ACTIVE_HIGH : BTN_ACTIVE_LOW;
    btn->cur_state      = btn->old_state;
    btn->new_state      = btn->old_state;
    return btn;
}

/**
 * @brief set multi click debounce ms (consider as multiclick if clicked within duration)
 * 
 * @param btn btn_t pointer 
 * @param ms time in ms 
 * @return true if success
 * @return false if not
 */
bool btn_set_click_ms(btn_t *btn, uint32_t ms){
    if(!btn) return false;
    btn->click_ms = ms;
    return true;
}

/**
 * @brief set multi press min ms
 * 
 * @param btn btn_t pointer 
 * @param ms time in ms 
 * @return true if success
 * @return false if not
 */
bool btn_set_press_ms(btn_t *btn, uint32_t ms){
    if(!btn) return false;
    btn->press_ms = ms;
    return true;
}

/**
 * @brief set long press min ms
 * 
 * @param btn btn_t pointer 
 * @param ms time in ms 
 * @return true if success
 * @return false if not
 */
bool btn_set_long_press_ms(btn_t *btn, uint32_t ms){
    if(!btn) return false;
    btn->long_press_ms = ms;
    return true;
}

/**
 * @brief Enable specific event on specific button
 * 
 * @param btn btn_t pointer 
 * @param evt events
 * @return true if success
 * @return false if not
 */
bool btn_enable_evt(btn_t *btn, uint32_t evt) {
    if (!btn)
        return false;
    btn->events |= evt;
    return true;
}

/**
 * @brief Disable specific event on specific button
 * 
 * @param btn btn_t pointer 
 * @param evt events
 * @return true if success
 * @return false if not
 */
bool btn_disable_evt(btn_t *btn, uint32_t evt) {
    if (!btn)
        return false;
    btn->events &= ~(evt);
    return true;
}

/**
 * @brief set gpio read callback function
 * 
 * @param btn btn_t pointer
 * @param func attach function to read gpio
 * @return true if success
 * @return false if failed
 */
bool btn_set_read_gpio_func(btn_t *btn, gpio_read_cb_t func) {
    if (!func)
        return false;
    btn->gpio_read_cb = func;
    return true;
}

/**
 * @brief set gpio event callback function
 * 
 * @param btn btn_t pointer
 * @param func attach function for event
 * @return true if success
 * @return false if failed
 */
bool btn_set_evt_func(btn_t *btn, btn_evt_cb_t func) {
    if (!func)
        return false;
    btn->btn_evt_cb = func;
    return true;
}


/**
 * @brief set tick ms callback function
 * 
 * @param btn  btn_t pointer
 * @param func attach function to get tick ms
 * @return true success
 * @return false failed
 */
bool btn_set_tick_ms_func(btn_t *btn, tick_ms_cb_t func) {
    if (!func)
        return false;
    btn->tick_ms_cb = func;
    return true;
}

/**
 * @brief Run forever
 * 
 * @param btn pointer
 */
void btn_loop(btn_t *btn) {
    if ((!btn) ||
        (!btn->gpio_read_cb) ||
        (!btn->tick_ms_cb) ||
        (!btn->btn_evt_cb))
        return;

    btn->cur_state = btn->gpio_read_cb(btn);

    if (btn->cur_state != btn->old_state) {
        if (is_btn_pressed(btn)) {
            btn->duration = 0;
        } else {
            btn->duration = btn->tick_ms_cb() - btn->tick;
        }
        btn->tick = btn->tick_ms_cb();
        btn->old_state = btn->cur_state;
    }

    if (btn->new_state != btn->cur_state) {
        if ((btn->tick_ms_cb() - btn->tick) >= btn->debounce_ms) {
            if (is_btn_pressed(btn)) {
                btn->click_count++;
            }
            btn->new_state = btn->cur_state;
        }
    }

    if (btn->click_count) {
        if (is_btn_pressed(btn)) {
            if ((btn->events & BTN_EVT_LONG_PRESS) == BTN_EVT_LONG_PRESS) {
                if ((btn->tick_ms_cb() - btn->tick) >= btn->long_press_ms) {
                    btn->btn_evt_cb(BTN_EVT_LONG_PRESS, btn);
                    btn->click_count = 0;
                }
            } else if ((btn->events & BTN_EVT_PRESS) == BTN_EVT_PRESS) {
                if ((btn->tick_ms_cb() - btn->tick) >= btn->press_ms) {
                    btn->btn_evt_cb(BTN_EVT_PRESS, btn);
                    btn->click_count = 0;
                }
            }
        } else {
            if ((btn->duration) >= btn->long_press_ms) {
                if ((btn->events & BTN_EVT_LONG_PRESS) == BTN_EVT_LONG_PRESS)
                    btn->btn_evt_cb(BTN_EVT_LONG_PRESS, btn);
                btn->click_count = 0;
            } else if ((btn->duration) >= btn->press_ms) {
                if ((btn->events & BTN_EVT_PRESS) == BTN_EVT_PRESS)
                    btn->btn_evt_cb(BTN_EVT_PRESS, btn);
                btn->click_count = 0;
            } else if ((btn->tick_ms_cb() - btn->tick) >= btn->click_ms) {
                if ((btn->events & BTN_EVT_MULTI_CLICK) == BTN_EVT_MULTI_CLICK)
                    btn->btn_evt_cb(BTN_EVT_MULTI_CLICK, btn);
                btn->click_count = 0;
            } else {
            }
        }
    }
}

/**
 * @brief delete resources used by button
 * 
 * @param btn btn_t pointer
 */
void btn_remove(btn_t **btn) {
    free(*btn);
    *btn = NULL;
}

#ifdef __cplusplus
}
#endif

#endif