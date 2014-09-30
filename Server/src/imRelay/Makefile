CXXFLAGS =	-O0 -g -Wall -fmessage-length=0 -DNDEBUG 
OBJS =		imRelay.o init_configure.o json_opt.o local_transport.o \
			redis_opt.o logic_opt.o redis_conn_pool.o

LIBS =		-lboost_system  -llog4cxx \
			-levent -ljson -lhiredis -luuid \
			-Wl,-rpath,/usr/local/lib	

TARGET =	imRelay

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)  

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
