#ifndef SRC_LOGGER_H_
#define SRC_LOGGER_H_

void fl_logger_init();
void fl_logger_close();

/*
 * level 0 : info
 * level 1 : warning
 * level 2 : error
 * level 9999 : exit info
 * */
void fl_log(int level, const char * msg, ...);
void fl_debug_log(const char * msg, ...);


#endif /* SRC_LOGGER_H_ */
