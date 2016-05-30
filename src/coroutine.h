#ifndef SRC_COROUTINE_H_
#define SRC_COROUTINE_H_

#include <unistd.h>
#include <pthread.h>

class fl_coroutine
{
private:
	pthread_cond_t cond;
	int state;
public:
	void create();
	void stop();
	void resume();
};




#endif /* SRC_COROUTINE_H_ */
