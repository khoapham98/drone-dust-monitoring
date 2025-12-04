#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "sys/log.h"
#include "device_setup.h"

extern pthread_t thread[MAX_THREADS];

int main()
{
	deviceSetup();	

	for (int i = 0; i < MAX_THREADS; i++) 
	{
		void* tmp;
		pthread_join(thread[i], &tmp);
	}

	return 0;
}
