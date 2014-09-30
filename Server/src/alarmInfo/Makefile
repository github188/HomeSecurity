CXXFLAGS =	-O0 -g -Wall -fmessage-length=0 -DNDEBUG 
OBJS =		alarmInfo.o init_configure.o local_transport.o consumers.o \
 			logic_operate.o json_opt.o

LIBS =		-lboost_thread -lboost_regex -lboost_systemd  \
			-llog4cxx -levent -ljson -lmysqlcppconn \
			 -Wl,-rpath,/usr/local/lib

TARGET =	alarminfo

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)  

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
