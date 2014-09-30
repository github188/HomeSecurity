DEBUG ?= 1
ifeq (DEBUG, 1)
	CXXFLAGS = -DDEBUG  -O0 -pg -g -Wall -fmessage-length=0
else
	CXXFLAGS =   -O0 -pg -g -Wall -fmessage-length=0 -DNDEBUG
endif

OBJS =		imServer.o init_configure.o master_thread.o worker_threads.o \
			json_opt.o  redis_opt.o logic_opt.o local_transport.o \
			relay_message_opt.o redis_conn_pool.o

LIBS =		-lboost_system -lboost_filesystem -llog4cxx \
			-levent -ljson -lhiredis -Wl,-rpath,/usr/local/lib

TARGET =	imServer

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS) 

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
