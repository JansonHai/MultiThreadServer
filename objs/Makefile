OBJS = $(shell ls | grep .o$)
MY_LIB_A = 
MY_LIB_SO = /usr/local/lib/liblua.so /usr/lib/mysql/libmysqlclient.so

all:server

server:$(OBJS)
	$(CC) $(CCFLAG) $(OBJS) -L$(LIB_DIR) $(MY_LIB_A) $(MY_LIB_SO) -o $(BIN_DIR)/server -Wl,-E

