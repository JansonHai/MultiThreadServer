#include <pthread.h>
#include <unistd.h>
#include "Timer.h"

Timer Timer::_instance = Timer();

Timer::Timer()
{
	Init();
}

Timer::~Timer()
{
}

void Timer::Init()
{
	pthread_create(&m_tid, NULL, start_thread, (void *) this);
}

void * Timer::start_thread(void *arg)
{
	Timer * timer = (Timer *) arg;
	timer->timer_run(NULL);
	pthread_exit(0);
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

int Timer::add_timer(struct timeval, void (*func)(void * arg), char * arg, int size)
{
	return 0;
}

void remove_timer(int timer_id)
{

}
