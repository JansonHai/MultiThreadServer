#ifndef SRC_ENVIRMENT_H_
#define SRC_ENVIRMENT_H_

bool fl_load_config(const char * configName);
bool fl_reload_config(const char * configName);

void fl_setenv(const char * key, const char * value);
const char * fl_getenv(const char * key);
const char * fl_getenv(const char * key, const char * def);
int fl_getenv(const char * key, const int def);
double fl_getenv(const char * key, const double def);


#endif /* SRC_ENVIRMENT_H_ */
