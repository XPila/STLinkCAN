// can.h - common CAN low-level interface

#ifndef _CAN_H
#define _CAN_H


#include <inttypes.h>


typedef enum _can_res_t
{
	can_OK = 0,
	can_ERR_NOT_INIT = -1,
	can_ERR_HW = -2,
	can_ERR_RX_EMPTY = -3,
	can_ERR_RX_OVER = -4,
	can_ERR_TX_FULL = -5,
} can_res_t;

typedef enum _can_mode_t
{
	can_mode_NORMAL = 0,         // Normal mode
	can_mode_LOOPBACK = 1,       // Loopback mode
	can_mode_SILENT = 2,         // Silent mode
	can_mode_SILENT_LOOPBACK = 3 // Silent Loopback mode
} can_mode_t;

typedef struct _can_msg_t
{
	uint32_t id:32;
	uint8_t ext:1;
	uint8_t rtr:1;
	uint8_t dl:4;
	uint8_t :2;
	uint8_t data[8];
} can_msg_t;

typedef struct _can_filter_mask32_t
{
	uint8_t fifo;   // 0-1
	uint32_t id;    // 0x000-0x7ff
	uint32_t mask;  // 0x000-0x7ff
	uint8_t ext;    // 0-1
	uint8_t rtr;    // 0-1
	uint8_t enabled;// 0-1
} can_filter_mask32_t;


#ifdef __cplusplus
extern "C" {
#endif //__cplusplus


extern can_res_t can_init(can_mode_t mode, uint32_t baudrate);

extern can_res_t can_done(void);

extern can_res_t can_set_filter_mask32(uint8_t bank, can_filter_mask32_t* pf);

extern can_res_t can_start(void);

extern can_res_t can_stop(void);

extern can_res_t can_tx_msg(can_msg_t* pmsg);

extern can_res_t can_rx_msg(can_msg_t* pmsg);


#ifdef __cplusplus
}
#endif //__cplusplus


#endif // _CAN_H
