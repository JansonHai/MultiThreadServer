//all global variables declare here
#include "../engine/global.h"

#include <mysql/mysql.h>
#include <pthread.h>
#include <stdint.h>
#include "net.h"

//int g_lisetn_fd = -1;  //socket listen fd
int g_exit_flag = false;
MYSQL * g_mysql_conn;
int * g_client_conn_fds;
int * g_client_conn_sessions;
struct fl_net_message_queue * g_client_conn_mq;
pthread_mutex_t * g_client_conn_mutex;
int MAX_CLIENT_CONNECTIONS = 2048;
uint32_t MAX_MESSAGE_LENGTH = 2097152;


