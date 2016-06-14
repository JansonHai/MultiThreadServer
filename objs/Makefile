OBJS = $(shell ls | grep .o$)

all:server

server:$(OBJS)
	$(CC) $(CCFLAG) $(OBJS) -L$(LIB_DIR) /usr/local/lib/liblua.a -o $(BIN_DIR)/server

