#ifndef SRC_GATE_H_
#define SRC_GATE_H_

bool fl_start_net_gate_server();
void fl_stop_net_gate_server();
void fl_gate_send_message_to_client(int index, uint32_t session, const char * data, int length);
bool fl_start_net_gate_watchdog_server();
void fl_stop_net_gate_watchdog_server();

#endif /* SRC_GATE_H_ */
