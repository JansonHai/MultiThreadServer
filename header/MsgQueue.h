#ifndef HEADER_MSGQUEUE_H_
#define HEADER_MSGQUEUE_H_

#include <stdint.h>
#include <pthread.h>
#include <queue>

template <typename T>
class MsgQueue
{
private:
	pthread_mutex_t m_mutex;
	std::queue<T> m_queue;
	int m_size;
public:
	MsgQueue();
	~MsgQueue();
	void push_message(T message);
	bool pop_message(T &message);
	int Size();
};



#endif /* HEADER_MSGQUEUE_H_ */
