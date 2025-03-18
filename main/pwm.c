#include "pwm.h"
#include "esp_log.h"

#define LEDC_OUTPUT_IO          (19) // Define the output GPIO
#define LEDC_FREQUENCY          (4000) // Frequency in Hertz. Set frequency at 4 kHz
#define LEDC_CHANNEL            LEDC_CHANNEL_0

static const char *TAG = "PWM";

esp_err_t pwm_init() {
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_13_BIT,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = LEDC_FREQUENCY,
        .clk_cfg = LEDC_AUTO_CLK
    };

    esp_err_t err = ledc_timer_config(&ledc_timer);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure timer: %s", esp_err_to_name(err));
        return err;
    }

    ledc_channel_config_t ledc_channel = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL,
        .timer_sel = LEDC_TIMER_0,
        .intr_type = LEDC_INTR_DISABLE,
        .gpio_num = LEDC_OUTPUT_IO,
        .duty = 0,
        .hpoint = 0
    };

    err = ledc_channel_config(&ledc_channel);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure channel: %s", esp_err_to_name(err));
        return err;
    }

    return ESP_OK;
}

esp_err_t pwm_set_duty(int duty) {
    esp_err_t err = ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL, duty);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set duty: %s", esp_err_to_name(err));
        return err;
    }

    err = ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to update duty: %s", esp_err_to_name(err));
        return err;
    }

    return ESP_OK;
}

esp_err_t pwm_start() {
    return pwm_set_duty(0);
}

esp_err_t pwm_stop() {
    esp_err_t err = ledc_stop(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL, 0);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to stop PWM: %s", esp_err_to_name(err));
        return err;
    }

    return ESP_OK;
}