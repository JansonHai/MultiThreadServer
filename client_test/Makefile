CUR_OBJS = $(shell ls ../objs | grep .o$ | awk '{if ($$0 != "main.o") print $$0}')
OBJS=${patsubst %.o, $(OBJS_DIR)/%.o, $(CUR_OBJS)}
MY_LIB_A = 
MY_LIB_SO = /usr/local/lib/liblua.so /usr/lib/mysql/libmysqlclient.so

all : client client2 gate_helper

client:client.cpp
	$(CC) $(CCFLAG) client.cpp -o $(BIN_DIR)/client
	
client2:client2.cpp
	$(CC) $(CCFLAG) client2.cpp -o $(BIN_DIR)/client2

gate_helper:gate_helper.cpp
	$(CC) $(CCFLAG) gate_helper.cpp $(OBJS) -L$(LIB_DIR) $(MY_LIB_A) $(MY_LIB_SO) -o $(BIN_DIR)/gate_helper -Wl,-E