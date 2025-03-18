idf_component_register(SRCS "pwm.c" "main.c" "ble_provider.c"
                            "mpu6050.c"
                            "gesture.cc"
                    INCLUDE_DIRS ".")
#spiffs_create_partition_image(storage ../spiffs FLASH_IN_PROJECT)