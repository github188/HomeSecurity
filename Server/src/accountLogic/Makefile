CXXFLAGS =	-O0 -g -Wall -fmessage-length=0 -DNDEBUG 
OBJS =		accountLogic.o init_configure.o local_transport.o consumers.o \
 			logic_operate.o json_opt.o sql_opt.o sql_conn_pool.o \
 			redis_opt.o redis_conn_pool.o md5.o sqlite_opt.o

LIBS =		-lboost_thread -lboost_regex -lboost_system  \
			-llog4cxx -levent -ljson -lmysqlcppconn -lhiredis \
			-lsqlite3 -lmailsend -Wl,-rpath,/usr/local/lib

TARGET =	accountLogic

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)  

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
