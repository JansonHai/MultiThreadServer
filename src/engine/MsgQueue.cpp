#include "MsgQueue.h"

template <typename T>
MsgQueue<T>::MsgQueue()
{
	pthread_mutex_init(&m_mutex, NULL);
	m_size = 0;
}

template <typename T>
MsgQueue<T>::~MsgQueue()
{
	pthread_mutex_destroy(&m_mutex);
}

template <typename T>
void MsgQueue<T>::push_message(T message)
{
	pthread_mutex_lock(&m_mutex);
	m_queue.push(message);
	++m_size;
	pthread_mutex_unlock(&m_mutex);
}

template <typename T>
bool MsgQueue<T>::pop_message(T &message)
{
	pthread_mutex_lock(&m_mutex);
	if (0 == m_size)
	{
		message = NULL;
		pthread_mutex_unlock(&m_mutex);
		return false;
	}
	message = m_queue.front();
	m_queue.pop();
	--m_size;
	pthread_mutex_unlock(&m_mutex);
	return true;
}

template <typename T>
int MsgQueue<T>::Size()
{
	return m_size;
}
