#ifndef HEADER_TIMER_H_
#define HEADER_TIMER_H_

#include <stdio.h>
#include <time.h>
#include <pthread.h>

class Timer
{
private:
	pthread_t m_tid;
	static Timer _instance = Timer();
	Timer();
	~Timer();
	void Init();
	void * timer_run(void * arg);
public:
	Timer * getInstance();
	int add_timer(struct timeval,void (*func)(void * arg),char * arg = NULL,int size = 0);
	void remove_timer(int timer_id);
};

#endif /* HEADER_TIMER_H_ */
