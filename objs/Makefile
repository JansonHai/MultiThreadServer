OBJS = $(shell ls | grep .o$)

all:server

server:$(OBJS)
	$(CC) $(CCFLAG) $(OBJS) /usr/local/lib/liblua.a -o $(BIN_DIR)/server

