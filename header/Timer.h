#ifndef HEADER_TIMER_H_
#define HEADER_TIMER_H_

#include <stdio.h>
#include <time.h>
#include <pthread.h>

class Timer
{
private:
	static Timer _instance;
	static void * start_thread(void *arg);
	pthread_t m_tid;
	Timer();
	~Timer();
	void Init();
	void * timer_run(void * arg);
public:
	Timer * getInstance();
	int add_timer(struct timeval, void (*func)(void * arg), char * arg = NULL, int size = 0);
	void remove_timer(int timer_id);
};

#endif /* HEADER_TIMER_H_ */
