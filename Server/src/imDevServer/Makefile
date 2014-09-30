DEBUG ?= 1
ifeq (DEBUG, 1)
	CXXFLAGS = -DDEBUG -DUSE_LOG4CXX -DUSE_LOG4CXX_PTR  -O0 -pg -g -Wall -fmessage-length=0
else
	CXXFLAGS = -DNDEBUG -DUSE_LOG4CXX -DUSE_LOG4CXX_PTR  -O0 -pg -g -Wall -fmessage-length=0
endif

OBJS =		imDevServer.o init_configure.o net_core.o local_transport.o work_thread.o json_opt.o  redis_opt.o message_opt.o relay_message_opt.o redis_conn_pool.o sql_conn_pool.o sql_opt.o

LIBS =

TARGET =	imDevServer

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS) -lboost_system -lboost_filesystem -llog4cxx -levent -ljson -lhiredis -lmysqlcppconn -lclientchannel -Wl,-rpath,/usr/local/lib

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
