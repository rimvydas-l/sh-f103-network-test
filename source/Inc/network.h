
#ifndef _PROJECT_NETWORK_H
#define _PROJECT_NETWORK_H

typedef enum status {
	OFF = 0,
	OK  = 100 
} networkStatus;

void network_w5500_init(void);
networkStatus network_w5500_run(void);

#endif
