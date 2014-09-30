CXXFLAGS =	-O0 -g -Wall -fmessage-length=0 -DNDEBUG  -DUSE_LOG4CXX -DUSE_LOG4CXX_PTR

OBJS =		alarmLogic.o init_configure.o consumers.o \
 			logic_operate.o json_opt.o local_transport.o \
 			redis_opt.o redis_conn_pool.o

LIBS =		-lboost_thread -lboost_regex -lhiredis \
			-lnet_interface_log \
			-llog4cxx -levent -ljson  -Wl,-rpath,/usr/local/lib 

TARGET =	alarmLogic

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
