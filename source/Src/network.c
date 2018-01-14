
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


void reset_w5500(void);
void  wizchip_select(void);
void  wizchip_deselect(void);
uint8_t wizchip_read();
void  wizchip_write(uint8_t wb);
void init_w5500(void);
int8_t getLinkStatus(void); 
void my_ip_assign(void);
void my_ip_conflict(void);


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
	wizchip_setnetinfo(&gWizNetInfo);               // apply from DHCP
	
	nstatus = IPSET;
}

void my_ip_conflict(void)
{
	nstatus = OFF;
}

//MQTT

