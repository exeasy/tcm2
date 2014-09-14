MYDIR=.
INCLUDE= -I$(MYDIR)/include/ -I/usr/include/pcap -I/usr/include/libxml2

MYLIB=-L /usr/lib -lpcap -lxml2 -lpthread

OBJ1=$(MYDIR)/src/main.c  \
	$(MYDIR)/src/pthread/pthread_cond_wait.c \
	$(MYDIR)/src/init/init_module.c \
	$(MYDIR)/src/collect/get_ethernet2_code.c \
	$(MYDIR)/src/collect/raw_socket.c \
	$(MYDIR)/src/analysis/analysis.c  $(MYDIR)/src/analysis/statistics.c \
	$(MYDIR)/src/policy/policy.c \
	$(MYDIR)/src/destory/destory.c \
	$(MYDIR)/src/util/util.c $(MYDIR)/src/util/interface.c  $(MYDIR)/src/util/queue.c \
	$(MYDIR)/src/comm/comm.c \
	$(MYDIR)/src/xml/xml.c 

TARGET1=traffic_control

all:$(TARGET1)
$(TARGET1): $(OBJ1)
	gcc -g -dH -o $(TARGET1) $(OBJ1) $(INCLUDE) $(MYLIB)
	@echo "$(TARGET1) Make success!"

clean:
	rm $(TARGET1)
	rm *log*
