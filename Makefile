ROOT_DIR = $(shell pwd)
SUBDIRS=$(shell ls -l | grep ^d | awk '{if ($$9 == "src") print $$9}')
OBJS_DIR = $(ROOT_DIR)/objs
LIB_DIR = $(ROOT_DIR)/libs
BIN_DIR = $(ROOT_DIR)/bin
#获取当前目录下的cpp文件集，放在变量CUR_SOURCE中
CUR_SOURCE=${wildcard *.cpp}
#将对应的cpp文件名转为o文件后放在下面的CUR_OBJS变量中
CUR_OBJS=${patsubst %.cpp, $(OBJS_DIR)/%.o, $(CUR_SOURCE)}

#配置编译器跟编译器选项跟
CC = g++
CCFLAG = -I$(ROOT_DIR)/src/header -L/usr/lib64/mysql/ -L$(ROOT_DIR)/libs \
-Wall -lpthread -lmysqlclient -llua -lm -ldl

export CC CCFLAG ROOT_DIR SUBDIRS OBJS_DIR BIN_DIR LIB_DIR

all : echo_objects $(SUBDIRS) $(CUR_OBJS) server

echo_objects:
	@echo $(CUR_OBJS)

$(SUBDIRS):echo
	make -C $@
    
$(CUR_OBJS):%.o:%.cpp
	$(CC) $(CCFLAG) -c $^ -o $(OBJS_DIR)/$@
    
echo:
	@echo $(SUBDIRS)
    
server:
	make -C $(OBJS_DIR)

client:$(ROOT_DIR)/client_test/client.cpp
	$(CC) $(CCFLAG) $(ROOT_DIR)/client_test/client.cpp -o $(BIN_DIR)/client
	
client2:$(ROOT_DIR)/client_test/client2.cpp
	$(CC) $(CCFLAG) $(ROOT_DIR)/client_test/client2.cpp -o $(BIN_DIR)/client2
	
clean:
	rm -f $(OBJS_DIR)/*.o
	rm -f $(BIN_DIR)/*