CC = g++
CCFLAG = -Wall -lpthread -lmysqlclient -L/usr/lib64/mysql/
ROOT_DIR = $(shell pwd)
SUBDIRS=$(shell ls -l | grep ^d | awk '{if ($$9 == "src") print $$9}')
OBJS_DIR = $(ROOT_DIR)/objs
BIN_DIR = $(ROOT_DIR)/bin
#获取当前目录下的cpp文件集，放在变量CUR_SOURCE中
CUR_SOURCE=${wildcard *.cpp}
#将对应的cpp文件名转为o文件后放在下面的CUR_OBJS变量中
CUR_OBJS=${patsubst %.cpp, %.o, $(CUR_SOURCE)}

export CC CCFLAG ROOT_DIR SUBDIRS OBJS_DIR BIN_DIR

all : echo_objects $(SUBDIRS) $(CUR_OBJS) server

echo_objects:
	@echo $(CUR_OBJS)

$(SUBDIRS):echo
	make -C $@
    
$(CUR_OBJS):%o:%cpp
	$(CC) $(CCFLAG) -c $^ -o $(OBJS_DIR)/$@
    
echo:
	@echo $(SUBDIRS)
    
server:$(OBJS_DIR)/*.o
	$(CC) $(CCFLAG) $(CUR_OBJS)/*.o -o $(BIN_DIR)/server
	
clean:
	rm -f $(OBJS_DIR)/*.o
	rm -f $(BIN_DIR)/*