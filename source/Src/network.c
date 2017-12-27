
#include "network.h"

networkStatus nstatus;

void network_w5500_init(void)
{
	nstatus = OFF;
}

networkStatus network_w5500_run(void) 
{
	return nstatus;	
}