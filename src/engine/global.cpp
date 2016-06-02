//all global variables declare here
#include <mysql/mysql.h>
#include <pthread.h>
#include <stdint.h>
#include "net.h"
#include "global.h"

//int g_lisetn_fd = -1;  //socket listen fd
int g_exit_flag = false;
MYSQL * g_mysql_conn;
uint32_t MAX_MESSAGE_LENGTH = 2097152;


