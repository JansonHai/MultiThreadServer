#ifndef SRC_EXIT_H_
#define SRC_EXIT_H_

//main exit function
void fl_add_main_exit_handle(void (*func)(void * args),void * args = 0, size_t size = 0);
void fl_main_exit();

//child exit function
void fl_add_child_exit_handle(void (*func)(void * args),void * args = 0, size_t size = 0);
void fl_child_exit();

//common
void fl_exit_log(const char * msg);

#endif /* SRC_EXIT_H_ */
