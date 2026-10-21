/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


 #include <stdio.h>
 #include "pico/stdlib.h"
 #include "hardware/gpio.h"
 #include "hardware/timer.h"
 #include "hardware/adc.h"
 
 const int PIN_LED_B = 4;
 
 const float conversion_factor = 3.3f / (1 << 12);
 
 volatile int g_timer_fired = 0;

 bool led_timer_callback(repeating_timer_t *rt) {
     g_timer_fired = 1;
     return true;
 }

 /**
  * 0..1.0V: Desligado
  * 1..2.0V: 300 ms
  * 2..3.3V: 500 ms
  */
 int main() {
     stdio_init_all();

     gpio_init(PIN_LED_B);
     gpio_set_dir(PIN_LED_B, GPIO_OUT);
     gpio_put(PIN_LED_B, 0);

     adc_init();
     adc_gpio_init(28);
     adc_select_input(2); // GP28 -> ADC2

     repeating_timer_t timer;
     int zona_atual = 0;
     int led = 0;

     while (1) {
         float voltage = adc_read() * conversion_factor;

         int zona;
         if (voltage < 1.0f) {
             zona = 0;
         } else if (voltage < 2.0f) {
             zona = 1;
         } else {
             zona = 2;
         }

         if (zona != zona_atual) {
             if (zona_atual != 0) {
                 cancel_repeating_timer(&timer);
             }
             led = 0;
             gpio_put(PIN_LED_B, 0);
             g_timer_fired = 0;

             if (zona == 1) {
                 add_repeating_timer_ms(300, led_timer_callback, NULL, &timer);
             } else if (zona == 2) {
                 add_repeating_timer_ms(500, led_timer_callback, NULL, &timer);
             }
             zona_atual = zona;
         }

         if (g_timer_fired) {
             g_timer_fired = 0;
             led = !led;
             gpio_put(PIN_LED_B, led);
         }
     }
 }
