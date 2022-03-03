// stlink.h - stlink bridge api

#ifndef _STLINK_H
#define _STLINK_H


#include <inttypes.h>


// same as Brg_StatusT (BRG_xxx)
typedef enum _stlink_res_t
{
	STLINK_NO_ERR = 0,             // OK (no error)
	STLINK_CONNECT_ERR,            // USB Connection error
	STLINK_DLL_ERR,                // USB DLL error
	STLINK_USB_COMM_ERR,           // USB Communication error
	STLINK_NO_DEVICE,              // No Bridge device target found error
	STLINK_OLD_FIRMWARE_WARNING,   // Warning: current bridge firmware is not the last one available
	STLINK_TARGET_CMD_ERR,         // Target communication or command error
	STLINK_PARAM_ERR,              // Wrong parameters error
	STLINK_CMD_NOT_SUPPORTED,      // Firmware command not supported by the current firmware version
	STLINK_GET_INFO_ERR,           // Error getting STLink Bridge device information
	STLINK_STLINK_SN_NOT_FOUND,    // Required STLink serial number not found error
	STLINK_NO_STLINK,              // STLink Bridge device not opened error
	STLINK_NOT_SUPPORTED,          // Parameter error
	STLINK_PERMISSION_ERR,         // STLink Bridge device already in use by another program error
	STLINK_ENUM_ERR,               // USB enumeration error
	STLINK_COM_FREQ_MODIFIED,      // Warning: required frequency is not exactely the one applied
	STLINK_COM_FREQ_NOT_SUPPORTED, // Required frequency cannot be applied error
	STLINK_SPI_ERR,                // SPI communication error
	STLINK_I2C_ERR,                // I2C communication error
	STLINK_CAN_ERR,                // CAN communication error
	STLINK_TARGET_CMD_TIMEOUT,     // Timeout error during Bridge communication
	STLINK_COM_INIT_NOT_DONE,      // Bridge Init function not called error
	STLINK_COM_CMD_ORDER_ERR,      // Sequencial Bridge function order error
	STLINK_BL_NACK_ERR,            // Bootloader NACK error
	STLINK_VERIF_ERR,              // Data verification error
	STLINK_MEM_ALLOC_ERR,          // Memory allocation error
	STLINK_GPIO_ERR,               // GPIO communication error
	STLINK_OVERRUN_ERR,            // Overrun error during bridge communication
	STLINK_CMD_BUSY,               // Command busy: only Brg::GetLastReadWriteStatus() command allowed in that case
	STLINK_CLOSE_ERR,              // Error during device Close
	STLINK_INTERFACE_ERR,          // Unknown default error returned by STLinkInterface
} stlink_res_t;


#ifdef __cplusplus
extern "C" {
#endif //__cplusplus


extern stlink_res_t stlink_init(char* serial_number);

extern void stlink_done(void);


#ifdef __cplusplus
}
#endif //__cplusplus


#endif // _STLINK_H
