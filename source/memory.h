#ifndef _PROJECT_MEMORY_H
#define _PROJECT_MEMORY_H

#include <stdint.h>

#define CMD_READ_STATUS_1			0x05
#define CMD_READ_STATUS_2			0x35
#define CMD_READ_STATUS_3			0x33
#define CMD_WRITE_ENABLE			0x06
#define CMD_WRITE_ENABLE_VSR		0x50
#define CMD_WRITE_DISABLE			0x06
#define CMD_WRITE_STATUS_REG		0x01
#define CMD_SET_BURST_WITH_WRAP		0x77
#define CMD_SET_BLOCK_PROTECTION	0x39
#define CMD_PAGE_PROGRAM			0x02
#define CMD_SECTOR_ERASE			0x20 //4k
#define CMD_BLOCK_ERASE				0xD8 //64k
#define CMD_CHIP_ERASE				0xC7
#define CMD_ERASE_SUSPEND			0x75
#define CMD_ERASE_RESUME			0x7A
#define CMD_READ_DATA				0x03
#define CMD_FAST_READ				0x0B
#define CMD_DEVICE_ID				0x90
#define CMD_RESET_EN				0x66
#define CMD_RESET					0x99
#define CMD_READ_SFDP				0x5A


typedef struct __MEMCHIP
{
	struct _CS
	{
		void(*_select)(void);       
		void(*_deselect)(void);      
	}CS;  
	
	struct
	{
		void(*_read_burst)(uint8_t* pBuf, uint16_t len);
		void(*_write_burst)(uint8_t* pBuf, uint16_t len);
	}SPI;
	
}_MEMCHIP;

//extern _MEMCHIP  MEMCHIP;

typedef struct __STATUS
{
	uint8_t SRP0;
	uint8_t SEC;
	uint8_t TB;
	uint8_t BP;
	uint8_t WEL;
	uint8_t BUSY;
	
	uint8_t SUS;
	uint8_t CMP;
	uint8_t LB;
	uint8_t QE;
	uint8_t SRP1;

	uint8_t RFU;
	uint8_t W;
	uint8_t LC;
}_STATUS;

extern _STATUS MEM_STATUS;
	
void reg_memchip_cs_cbfunc(void(*cs_sel)(void), void(*cs_desel)(void));
void reg_memchip_spiburst_cbfunc(void(*spi_rb)(uint8_t* pBuf, uint16_t len), void(*spi_wb)(uint8_t* pBuf, uint16_t len));

void memchip_reset();
void memchip_read_status();
void memchip_read(uint32_t address, uint8_t* pBuf, uint16_t len);
void memchip_read_sfdp(uint8_t* pBuf);

#endif