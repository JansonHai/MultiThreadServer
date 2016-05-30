CC = g++
CFLAGS = -Wall -lpthread -lmysqlclient -L/usr/lib64/mysql/
ROOT = .
OBJPATH = $(ROOT)/objs
SERVEROBJS = $(OBJPATH)/main.o $(OBJPATH)/logger.o $(OBJPATH)/envirment.o \
$(OBJPATH)/global.o $(OBJPATH)/server.o $(OBJPATH)/hotfix.o $(OBJPATH)/exit.o \
$(OBJPATH)/handler.o $(OBJPATH)/database.o $(OBJPATH)/net.o

all : server client client2

server : $(SERVEROBJS)
	$(CC) $(CFLAGS) $(SERVEROBJS) -o server
	
$(OBJPATH)/main.o : main.cpp 
	$(CC) $(CFLAGS) -c main.cpp -o $(OBJPATH)/main.o
	
$(OBJPATH)/logger.o : logger.cpp
	$(CC) $(CFLAGS) -c logger.cpp -o $(OBJPATH)/logger.o
	
$(OBJPATH)/envirment.o : envirment.cpp
	$(CC) $(CFLAGS) -c envirment.cpp -o $(OBJPATH)/envirment.o

$(OBJPATH)/server.o : server.cpp
	$(CC) $(CFLAGS) -c server.cpp -o $(OBJPATH)/server.o
	
$(OBJPATH)/global.o : global.cpp
	$(CC) $(CFLAGS) -c global.cpp -o $(OBJPATH)/global.o

$(OBJPATH)/hotfix.o : hotfix.cpp
	$(CC) $(CFLAGS) -c hotfix.cpp -o $(OBJPATH)/hotfix.o
	
$(OBJPATH)/exit.o : exit.cpp
	$(CC) $(CFLAGS) -c exit.cpp -o $(OBJPATH)/exit.o

$(OBJPATH)/handler.o : handler.cpp
	$(CC) $(CFLAGS) -c handler.cpp -o $(OBJPATH)/handler.o

$(OBJPATH)/database.o : database.cpp
	$(CC) $(CFLAGS) -c database.cpp -o $(OBJPATH)/database.o
	
$(OBJPATH)/net.o : net.cpp
	$(CC) $(CFLAGS) -c net.cpp -o $(OBJPATH)/net.o
	
client : $(OBJPATH)/client.o
	$(CC) $(CFLAGS) $(OBJPATH)/client.o -o client
	
$(OBJPATH)/client.o : client.cpp
	$(CC) $(CFLAGS) -c client.cpp -o $(OBJPATH)/client.o
	
client2 : $(OBJPATH)/client2.o
	$(CC) $(CFLAGS) $(OBJPATH)/client2.o -o client2
	
$(OBJPATH)/client2.o : client2.cpp
	$(CC) $(CFLAGS) -c client2.cpp -o $(OBJPATH)/client2.o
	
clean:
	rm -f $(OBJPATH)/*.o server client