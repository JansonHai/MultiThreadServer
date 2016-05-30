OBJS = $(shell ls | grep .o$)

all:server

server:$(OBJS)
	$(CC) $(CCFLAG) $(OBJS) -o $(BIN_DIR)/server

