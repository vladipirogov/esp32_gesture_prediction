#ifndef PWM_H
#define PWM_H

#include "driver/ledc.h"
#include "esp_err.h"

// Function prototypes
esp_err_t pwm_init();
esp_err_t pwm_set_duty(int duty);
esp_err_t pwm_start();
esp_err_t pwm_stop();

#endif // PWM_H