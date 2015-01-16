###############################################################################
## Makefile
## UDP packet logger as data logger for OD device
## David Andrew Crawford Beck
## dacb@uw.edu
## Original:
##	Mon Apr 1 10:30:16 PDT 2013
## Modified:
###############################################################################

CC=gcc
OPTIONS = -Wall -funsigned-char -funsigned-bitfields -ffunction-sections -fpack-struct -fshort-enums
COPTS   = 
CFLAGS	= $(COPTS) $(INLINE) $(OPTIONS) 

DS_SRCS = dataServer.c httpd.c file.c error.c net.c socket.c sndup.c
DS_OBJS = $(DS_SRCS:.c=.o)

TEST_SRCS = dataServer_test.c
TEST_OBJS = $(TEST_SRCS:.c=.o)

SRCS = $(DS_SRCS) $(TEST_SRCS)
OBJS = $(DS_OBJS) $(TEST_OBJS)

BINS = dataServer dataServer_test

default: all

all: $(BINS)

dataServer: $(DS_OBJS)
	$(CC) $(CFLAGS) -o $@ $(DS_OBJS)

dataServer_test: $(TEST_OBJS)
	$(CC) $(CFLAGS) -o $@ $<

.c.o: Makefile packet.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	/bin/rm -rf $(BINS) $(OBJS)

test: $(BINS)
	./dataServer 1993 test.txt 7251&
	sleep 1
	./dataServer_test 127.0.0.1 1993
