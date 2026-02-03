#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "device_setup.h"

void app_main(void)
{
    deviceSetup();

    vTaskDelete(NULL);
}
