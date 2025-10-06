#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdint.h>
#include "inc/pms7003.h"
#include "inc/uart.h"

#define 	UART1_FILE_PATH			"/dev/ttyS1"
#define 	MAX_THREADS				2

typedef struct 
{
	uint8_t data[32];
	pthread_cond_t cond;
	pthread_mutex_t lock;
	volatile int data_rcv;
} queue_t;

int uart1_fd;
queue_t queue;

void* thread1_read_dust_sensor(void* arg)
{
	while (1)
	{
		pthread_mutex_lock(&queue.lock);
		pms7003_read(uart1_fd, queue.data, sizeof(queue.data));
		queue.data_rcv = 1;
		pthread_cond_signal(&queue.cond);
		pthread_mutex_unlock(&queue.lock);
		sleep(1);
	}

	return arg;
}

void* thread2_send_to_esp(void* arg)
{	
	while (1)
	{
		pthread_mutex_lock(&queue.lock);

		while (queue.data_rcv == 0)
			pthread_cond_wait(&queue.cond, &queue.lock);
		
		char json_data[100] = {0};
		pms7003_data_to_json(json_data, sizeof(json_data), queue.data, 10.1235, 123.345);
		printf("%s\n", json_data);
		queue.data_rcv = 0;
		pthread_mutex_unlock(&queue.lock);
	}
	
	return arg;
}

int main()
{
	pthread_cond_init(&queue.cond, NULL);
	pthread_mutex_init(&queue.lock, NULL);
	queue.data_rcv = 0;

	uart1_fd = uart_init(UART1_FILE_PATH);

	pthread_t thread[MAX_THREADS];
	pthread_create(&thread[0], NULL, thread1_read_dust_sensor, NULL);
	pthread_create(&thread[1], NULL, thread2_send_to_esp, NULL);

	for (int i = 0; i < MAX_THREADS; i++) 
	{
		void* tmp;
		pthread_join(thread[i], &tmp);
	}

	close(uart1_fd);
	return 0;
}
