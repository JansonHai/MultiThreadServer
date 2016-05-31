#ifndef HEADER_BUFFER_H_
#define HEADER_BUFFER_H_

struct fl_buffer
{
	void * buffer;
	int size;
	struct fl_buffer * next;
};

void fl_init_buffer();
struct fl_buffer * fl_malloc(int size);
bool fl_realloc(struct fl_buffer * buffer,int size);
void fl_free(struct fl_buffer * buffer);

#endif /* HEADER_BUFFER_H_ */
