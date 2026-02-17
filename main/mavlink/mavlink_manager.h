/**
 * @file    mavlink_manager.h
 * @brief   MAVLink message reception and dispatch interface
 */
#ifndef _MAVLINK_MANAGER_H_
#define _MAVLINK_MANAGER_H_
#include "common/mavlink.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#define MAVLINK_MAX_SUBSCRIBED_MSG      128
#define MAVLINK_QUEUE_SEND_TIMEOUT_MS   5

/**
 * @brief   Subscribe to a specific MAVLink message ID.
 * @param   queue Queue handle used to receive MAVLink messages.
 * @param   msgid MAVLink message ID to subscribe to.
 * @return  none.
 */
void mavlinkSubscribeMsg(QueueHandle_t queue, uint32_t msgid);

/**
 * @brief   Initialize the MAVLink manager.
 * @return  0 on success, -1 on failure.
 */
int setupMavlinkManager(void);

#endif