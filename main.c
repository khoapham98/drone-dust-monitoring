#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "device_setup.h"
#include "src/drivers/uart.h"
#include "src/drivers/log.h"

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
