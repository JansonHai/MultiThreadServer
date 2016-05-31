#include <pthread.h>
#include "timer.h"
#include <unistd.h>

Timer::Timer()
{
	Init();
}

Timer::~Timer()
{
}

void Timer::Init()
{
	pthread_create(&m_tid, NULL, timer_run, NULL);
}

void * Timer::timer_run(void * arg)
{
	while (true)
	{
		usleep(15000);  //sleep 15 ms
	}
	pthread_exit(0);
}

Timer * Timer::getInstance()
{
	return &_instance;
}

int Timer::add_timer(struct timeval,void (*func)(void * arg),char * arg = NULL,int size = 0)
{
	return 0;
}

void remove_timer(int timer_id)
{

}
