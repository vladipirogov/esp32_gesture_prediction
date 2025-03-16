#ifndef INCLUDE_GESTURE_H_
#define INCLUDE_GESTURE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "mpu6050.h"

typedef struct {
    mpu6050_handle_t sensor;
    QueueHandle_t sensorQueue;
} MPUparams;

void gesture_predict(void *param);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_GESTURE_H_ */
