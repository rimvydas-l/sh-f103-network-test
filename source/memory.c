#include "memory.h"

_MEMCHIP  MEMCHIP;
_STATUS MEM_STATUS;

uint8_t commandBuff[6];
uint8_t transferBuff[6];

void reg_memchip_cs_cbfunc(void(*cs_sel)(void), void(*cs_desel)(void))
{
	MEMCHIP.CS._select   = cs_sel;
	MEMCHIP.CS._deselect = cs_desel;
}

void reg_memchip_spiburst_cbfunc(void(*spi_rb)(uint8_t* pBuf, uint16_t len), void(*spi_wb)(uint8_t* pBuf, uint16_t len))
{
		MEMCHIP.SPI._read_burst   = spi_rb;
		MEMCHIP.SPI._write_burst  = spi_wb;
}

void memchip_read_status()
{
	commandBuff[0] = CMD_READ_STATUS_1;
	MEMCHIP.CS._select();
	MEMCHIP.SPI._write_burst(commandBuff, 1);
	MEMCHIP.SPI._read_burst(transferBuff, 1);
	MEMCHIP.CS._deselect();
	
	MEM_STATUS.SRP0 =	(transferBuff[0] & 0b10000000) >> 7;
	MEM_STATUS.SEC =	(transferBuff[0] & 0b01000000) >> 6;
	MEM_STATUS.TB =		(transferBuff[0] & 0b00100000) >> 5;
	MEM_STATUS.BP =		(transferBuff[0] & 0b00011100) >> 2;
	MEM_STATUS.WEL =	(transferBuff[0] & 0b00000010) >> 1;
	MEM_STATUS.BUSY =	(transferBuff[0] & 0b00000001) >> 0;
	
	commandBuff[0] = CMD_READ_STATUS_2;
	MEMCHIP.CS._select();
	MEMCHIP.SPI._write_burst(commandBuff, 1);
	MEMCHIP.SPI._read_burst(transferBuff, 1);
	MEMCHIP.CS._deselect();
	
	MEM_STATUS.SUS =	(transferBuff[0] & 0b10000000) >> 7;
	MEM_STATUS.CMP =	(transferBuff[0] & 0b01000000) >> 6;
	MEM_STATUS.LB =		(transferBuff[0] & 0b00111100) >> 2;
	MEM_STATUS.QE =		(transferBuff[0] & 0b00000010) >> 1;
	MEM_STATUS.SRP1 =	(transferBuff[0] & 0b00000001) >> 0;
	
	commandBuff[0] = CMD_READ_STATUS_3;
	MEMCHIP.CS._select();
	MEMCHIP.SPI._write_burst(commandBuff, 1);
	MEMCHIP.SPI._read_burst(transferBuff, 1);
	MEMCHIP.CS._deselect();
	
	MEM_STATUS.RFU =	(transferBuff[0] & 0b10000000) >> 7;
	MEM_STATUS.W =		(transferBuff[0] & 0b01110000) >> 4;
	MEM_STATUS.LC =		(transferBuff[0] & 0b00001111) >> 0;
}

void memchip_read(uint32_t address, uint8_t* pBuf, uint16_t len)
{
	commandBuff[0] = CMD_READ_DATA;
	commandBuff[1] = (address & 0xFF0000) >> 16;
	commandBuff[2] = (address & 0x00FF00) >> 8;
	commandBuff[3] = (address & 0x0000FF) >> 0;
	MEMCHIP.CS._select();
	MEMCHIP.SPI._write_burst(commandBuff, 4);
	MEMCHIP.SPI._read_burst(pBuf, len);
	MEMCHIP.CS._deselect();

}

void memchip_reset()
{
	commandBuff[0] = CMD_RESET_EN;
	MEMCHIP.CS._select();
	MEMCHIP.SPI._write_burst(commandBuff, 1);
	MEMCHIP.CS._deselect();

	commandBuff[0] = CMD_RESET;
	MEMCHIP.CS._select();
	MEMCHIP.SPI._write_burst(commandBuff, 1);
	MEMCHIP.CS._deselect();

}

void memchip_read_sfdp(uint8_t* pBuf)
{
	commandBuff[0] = CMD_READ_SFDP;
	commandBuff[1] = 0x00;
	commandBuff[2] = 0x00;
	commandBuff[3] = 0x00;
	commandBuff[4] = 0x00;
	MEMCHIP.CS._select();
	MEMCHIP.SPI._write_burst(commandBuff, 5);
	MEMCHIP.SPI._read_burst(pBuf, 256);
	MEMCHIP.CS._deselect();

}