CFLAGS += -g -I.. -Wall

VPATH = ..

.PHONY: all clean

all: mkmovie mknews

clean:
	-rm *.o mkmovie mknews

mkmovie : mkmovie.c

mknews : mknews.o gamedata.o
