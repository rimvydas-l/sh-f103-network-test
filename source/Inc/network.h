
#ifndef _PROJECT_NETWORK_H
#define _PROJECT_NETWORK_H

#include "stm32f1xx_hal.h"
#include "dhcp.h"
#include "MQTTClient.h"


#define DATA_BUF_SIZE				2048
#define SOCK_MQTT					6
#define SOCK_DHCP					7
#define LINK_OFF_RETRY				50
#define MQTT_CLIENT_ID				"F103-ID"
#define MQTT_CLIENT_USR				NULL
#define MQTT_CLIENT_PWD				NULL
#define MQTT_CLIENT_HOST			{ 192, 168, 1, 108 }
#define MQTT_CLIENT_PORT			1883


//#define MAGIC_COOKIE             0x001  ///< Any number. You can be modifyed it any number
//#define DCHP_HOST_NAME           "SH 0001\0"

typedef enum {
	OFF     = 0,
	RESETED = 1,
	INITED  = 2,
	IPSET   = 3,
	MQTTRDY	= 4,
	OK      = 100 
} networkStatus;

typedef struct {
	GPIO_TypeDef* RESET_PORT;
	uint16_t RESET_PIN;
	GPIO_TypeDef* CS_PORT;
	uint16_t CS_PIN;
	SPI_HandleTypeDef* SPI;
} networkInitStack;

extern MQTTClient client;

void network_w5500_init(networkInitStack*);
networkStatus network_w5500_run(void);
void network_1s_callback(void);
void network_1ms_callback(void);

#endif
