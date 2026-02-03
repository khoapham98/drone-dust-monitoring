#ifndef BOARD_H
#define BOARD_H

#if defined(BOARD_ESP32_LOLIN32_LITE)

  #include "board_esp32_lolin32_lite.h"

#elif defined(BOARD_ESP32_DEV_MODULE)

  #include "board_esp32_dev_module.h"

#else
  /* Default board */
  #define BOARD_ESP32_DEV_MODULE
  #include "board_esp32_dev_module.h"

#endif


#endif