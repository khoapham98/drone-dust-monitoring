#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "sys/log.h"
#include "device_setup.h"
#include "src/drivers/uart.h"

extern pthread_t thread[MAX_THREADS];
extern int uart1_fd;
int main()
{
	deviceSetup();	

	for (int i = 0; i < MAX_THREADS; i++) 
	{
		void* tmp;
		pthread_join(thread[i], &tmp);
	}

	close(uart1_fd);
	return 0;
}
