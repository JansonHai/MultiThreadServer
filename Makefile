CC = g++
CCFLAG = -Wall -lpthread -lmysqlclient -L/usr/lib64/mysql/
ROOT_DIR = $(shell pwd)
SUBDIRS=$(shell ls -l | grep ^d | awk '{print $$9}')
OBJS_DIR = $(ROOT_DIR)/objs
BIN_DIR = $(ROOT_DIR)/bin
CUR_SOURCE=${wildcard *.cpp}
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
    
server:$(CUR_OBJS)
	$(CC) $(CCFLAG) $(CUR_OBJS) -o $(BIN_DIR)/server
	
clean:
	rm -f $(OBJS_DIR)/*.o
	rm -f $(BIN_DIR)/*