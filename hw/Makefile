TARGET=p16ndeposit
CC=arm-linux-gcc
TFTP=atftp
HOST=192.168.1.58
STRIP=arm-linux-strip
CFLAGS=-DDEBUG_LOG -DNO_SHA256 -Wall
LDLIBS=-Llib -lcurl -lrf_card -lp18_dcgraph -laes -lother -lsqlite3 -lpthread -ldl -lm -liconv -ljpeg

SOURCES=\
p16main\
p16log\
menu\
cardop\
deposit\
utils\
syspara\
allowance\
sha1\
cJSON\
service\
message\
md5\
hmac\
gnudes

.SUFFIXES: .c .o .h

.c.o:
	$(CC) $(CFLAGS) -c $<
	
	
$(TARGET):$(SOURCES:=.o)
	$(CC) $(SOURCES:=.o) -o $@ $(LDLIBS)
	$(STRIP) $@
	
upload: $(TARGET)
	$(TFTP) -p -l $(TARGET) -r $(TARGET) $(HOST)

clean:
	rm -rf $(TARGET) *.o
	
