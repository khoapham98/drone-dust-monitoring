#include <pthread.h>
#include "sys/log.h"
#include "device_setup.h"

extern pthread_t thread[MAX_THREADS];
extern int threadCount;

int main(void)
{
	LOG_INF("=== APP START ===");

	deviceSetup();	

	for (int i = 0; i < threadCount; i++) {
		void* tmp;
		pthread_join(thread[i], &tmp);
	}

	LOG_INF("=== APP END ===");
	return 0;
}
