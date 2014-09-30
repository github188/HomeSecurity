CXXFLAGS =	-O0 -g -Wall -fmessage-length=0 -DNDEBUG 
OBJS =		autoUpdate.o init_configure.o local_transport.o consumers.o \
 			logic_operate.o json_opt.o sql_opt.o sql_conn_pool.o 

LIBS =		-lboost_thread -lboost_regex -lboost_system  \
			-llog4cxx -levent -ljson -lmysqlcppconn \
			 -Wl,-rpath,/usr/local/lib

TARGET =	autoUpdate

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)  

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
