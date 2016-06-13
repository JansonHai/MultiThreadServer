#ifndef SRC_HEADER_GAMELOGIC_H_
#define SRC_HEADER_GAMELOGIC_H_

#include "net.h"

struct fl_gamelogic_ctx
{
	class fl_connection * conn;
	struct fl_message_data * message;
};

void fl_start_gamelogic();
void fl_stop_gamelogic();
void fl_dispatch_message(class fl_connection * conn, struct fl_message_data * message);



#endif /* SRC_HEADER_GAMELOGIC_H_ */
