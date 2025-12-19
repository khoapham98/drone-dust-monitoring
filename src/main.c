#include <pthread.h>
#include "sys/log.h"
#include "device_setup.h"

extern pthread_t thread[MAX_THREADS];

int main(void)
{
	LOG_INF("=== APP START ===");
	deviceSetup();	

	for (int i = 0; i < MAX_THREADS; i++) 
	{
		void* tmp;
		pthread_join(thread[i], &tmp);
	}

	LOG_INF("=== APP END ===");
	return 0;
}
