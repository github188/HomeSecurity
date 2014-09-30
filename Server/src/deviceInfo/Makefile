CXXFLAGS =	-O0 -g -Wall -fmessage-length=0 -DNDEBUG
#-DNDEBUG 

OBJS =		deviceInfo.o init_configure.o local_transport.o consumers.o \
			logic_operate.o json_opt.o sql_opt.o base64.o \
            sql_conn_pool.o redis_opt.o redis_conn_pool.o \
            thread.o ontime_thread.o \
            cloudsee_transport.o cloudsee_message_opt.o cloudsee_data_opt.o

TARGET =	deviceInfo

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS) -lboost_system -lboost_filesystem -lboost_thread -lboost_regex -luuid -llog4cxx -levent -ljson -lmysqlcppconn -lhiredis -Wl,-rpath,/usr/local/lib

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
