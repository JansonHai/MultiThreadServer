#子目录的Makefile直接读取其子目录就行
SUBDIRS=$(shell ls -l | grep ^d | awk '{print $$9}')
#以下同根目录下的makefile的相同代码的解释
CUR_SOURCE=${wildcard *.cpp}
CUR_OBJS=${patsubst %.cpp, %.so, $(CUR_SOURCE)}

all:$(SUBDIRS) $(CUR_OBJS)

$(SUBDIRS):ECHO
	make -C $@
	
$(CUR_OBJS):%.so:%.cpp
	$(CC) $(CCFLAG) $^ -shared -o $(LIB_DIR)/$@
	
ECHO:
	@echo $(SUBDIRS)


#all : flapi.so fldataread.so fldatawrite.so
#
#flapi.so : flapi.cpp
#	$(CC) $(CCFLAG) flapi.cpp -shared -o $(LIB_DIR)/flapi.so
#	
#fldataread.so : fldataread.cpp
#	$(CC) $(CCFLAG) fldataread.cpp -shared -o $(LIB_DIR)/fldataread.so
#	
#fldatawrite.so : fldatawrite.cpp
#	$(CC) $(CCFLAG) fldatawrite.cpp -shared -o $(LIB_DIR)/fldatawrite.so