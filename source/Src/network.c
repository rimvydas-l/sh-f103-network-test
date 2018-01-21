
#include "network.h"
//------------------------
#include "stm32f1xx_hal.h"
#include "socket.h"
#include "dhcp.h"
#include "MQTTClient.h"

networkStatus nstatus;
networkInitStack* initVars;

uint8_t memsize[2][8] = { { 2, 2, 2, 2, 2, 2, 2, 2 }, { 2, 2, 2, 2, 2, 2, 2, 2 } };
uint8_t gDhcpDataBuff[DATA_BUF_SIZE];
uint8_t gMQTTSendDataBuff[DATA_BUF_SIZE];
uint8_t gMQTTRecvDataBuff[DATA_BUF_SIZE];

wiz_NetInfo gWizNetInfo = {
	.mac = { 0x00, 0x01, 0xff, 0xab, 0xcd, 0xef },
	.ip = { 0, 0, 0, 0 },
	.sn = { 0, 0, 0, 0 },
	.gw = { 0, 0, 0, 0 },
	.dns = { 0, 0, 0, 0 },
	.dhcp = NETINFO_DHCP
};

uint8_t dhcpStatus;
uint8_t linkOffRetry;

MQTTClient client;
Network n;
MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
unsigned char mhost[4] = MQTT_CLIENT_HOST;        // mqtt server IP
unsigned int mhostPort = MQTT_CLIENT_PORT;        // mqtt server port

MQTTMessage m;

void reset_w5500(void);
void  wizchip_select(void);
void  wizchip_deselect(void);
uint8_t wizchip_read();
void  wizchip_write(uint8_t wb);
void init_w5500(void);
int8_t getLinkStatus(void); 
void my_ip_assign(void);
void my_ip_conflict(void);
uint8_t checkSockOpen(uint8_t sNr);
void initMQTTClient();

//public functions

void network_w5500_init(networkInitStack* nis)
{
	nstatus = OFF;
	initVars = nis;
}

networkStatus network_w5500_run(void) 
{
	switch (nstatus) {
	case OFF:
		linkOffRetry = LINK_OFF_RETRY;
		reset_w5500();
		nstatus = RESETED;
		break;
	case RESETED:
		init_w5500();
		nstatus = INITED;
		break;
	case INITED:
		if (getLinkStatus() == PHY_LINK_ON) {
			dhcpStatus = DHCP_run();
			if (dhcpStatus == DHCP_FAILED) 	{ nstatus = OFF; }
			if (dhcpStatus == DHCP_IP_LEASED) { nstatus = IPSET; }
		}
		else 
		{
			HAL_Delay(100);
			linkOffRetry--;
			if (linkOffRetry == 0) { nstatus = OFF; }
		}
		break;
	case IPSET:
		if (checkSockOpen(SOCK_MQTT) && client.isconnected) 
		{
			nstatus = MQTTRDY;
		}
		else 
		{
			initMQTTClient();
		}
		break;
	case MQTTRDY:
		MQTTYield(&client, 50);
		nstatus = OK;
		break;
	case OK:
		nstatus = INITED;
		break;
	}
	
	return nstatus;	
}

void network_1s_callback(void) 
{
	DHCP_time_handler();
}

//W5500 service functions

void reset_w5500(void)
{
	HAL_GPIO_WritePin(initVars->RESET_PORT, initVars->RESET_PIN, GPIO_PIN_RESET);
	HAL_Delay(200);
	HAL_GPIO_WritePin(initVars->RESET_PORT, initVars->RESET_PIN, GPIO_PIN_SET);
	HAL_Delay(1000);
}

void  wizchip_select(void)
{
	HAL_GPIO_WritePin(initVars->CS_PORT, initVars->CS_PIN, GPIO_PIN_RESET);	
}

void  wizchip_deselect(void)
{
	HAL_GPIO_WritePin(initVars->CS_PORT, initVars->CS_PIN, GPIO_PIN_SET);	
}

uint8_t wizchip_read()
{
	uint8_t rb;
	HAL_SPI_Receive(initVars->SPI, &rb, 1, HAL_MAX_DELAY);
	return rb;
}

void  wizchip_write(uint8_t wb)
{
	HAL_SPI_Transmit(initVars->SPI, &wb, 1, HAL_MAX_DELAY);
}

void init_w5500(void)
{
	reg_wizchip_cs_cbfunc(wizchip_select, wizchip_deselect);
	reg_wizchip_spi_cbfunc(wizchip_read, wizchip_write);

	/* wizchip initialize*/
	if (ctlwizchip(CW_INIT_WIZCHIP, (void*) memsize) == -1) {
		nstatus = OFF;
		return;
	}

	DHCP_init(SOCK_DHCP, gDhcpDataBuff);
	reg_dhcp_cbfunc(my_ip_assign, my_ip_assign, my_ip_conflict);
}

int8_t getLinkStatus() 
{
	return wizphy_getphylink();
}

void my_ip_assign(void)
{
	getIPfromDHCP(gWizNetInfo.ip);
	getGWfromDHCP(gWizNetInfo.gw);
	getSNfromDHCP(gWizNetInfo.sn);
	getDNSfromDHCP(gWizNetInfo.dns);
	gWizNetInfo.dhcp = NETINFO_DHCP;
	/* Network initialization */
	wizchip_setnetinfo(&gWizNetInfo);                      // apply from DHCP
	
	nstatus = IPSET;
}

void my_ip_conflict(void)
{
	nstatus = OFF;
}

//MQTT

void publishMessage(char* topic, char* payload, uint8_t len)
{
	m.payload = payload;
	m.payloadlen = len;
	MQTTPublish(&client, topic, &m);
}

void messageArrived1(MessageData* md)
{
	if (md->message->payloadlen == 1)
	{
		char* payload=md->message->payload;
		if (*payload == '0') 
		{
			HAL_GPIO_WritePin(initVars->LED_PORT, initVars->LED_PIN, GPIO_PIN_SET);
			publishMessage("aaa/out/1", "0", 1);
		}
		if (*payload == '1')
		{
			HAL_GPIO_WritePin(initVars->LED_PORT, initVars->LED_PIN, GPIO_PIN_RESET);
			publishMessage("aaa/out/1", "1", 1);
		}
	}
}

void initMessage() 
{
	m.retained = 0;
	m.qos = QOS1;
}

void initMQTTClient()
{
	initMessage();
	
	int rc = 0;
	NewNetwork(&n, SOCK_MQTT);
	ConnectNetwork(&n, mhost, mhostPort);
	MQTTClientInit(&client, &n, 1000, gMQTTSendDataBuff, DATA_BUF_SIZE, gMQTTRecvDataBuff, DATA_BUF_SIZE);
	
	data.willFlag = 0;
	data.MQTTVersion = 3;
	data.clientID.cstring = MQTT_CLIENT_ID;
	data.username.cstring = MQTT_CLIENT_USR;
	data.password.cstring = MQTT_CLIENT_PWD;
	data.keepAliveInterval = 60;
	data.cleansession = 1;
	rc = MQTTConnect(&client, &data);
	
	rc = MQTTSubscribe(&client, "aaa/in/1", QOS0, messageArrived1);
}

uint8_t checkSockOpen(uint8_t sNr) {
	return getSn_SR(sNr) == SOCK_ESTABLISHED;
}

void network_1ms_callback(void)
{
	MilliTimer_Handler();
}
