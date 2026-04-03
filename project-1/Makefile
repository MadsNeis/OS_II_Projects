CC=gcc
CCOPTS=-Wall -Wextra
LIBS=
MAKESKEL=../../../bin/makeskeleton

SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)
TARGET=rrsim

ZIPBASE=project-1
ZIP=$(ZIPBASE).zip

.PHONY: all clean pristine zip

all: rrsim

$(TARGET): rrsim.o queue.o
	$(CC) $(CCOPTS) -o $@ $^

%.o: %.c
	$(CC) $(CCOPTS) -c $<

rrsim.o: rrsim.c queue.h

queue.o: queue.c queue.h

rrsim_skel.c: rrsim.c
	$(MAKESKEL) $< $@
    
zip: rrsim_skel.c
	rm -rf $(ZIPBASE)
	mkdir $(ZIPBASE)
	cp Makefile queue.c queue.h $(ZIPBASE)
	cp rrsim_skel.c $(ZIPBASE)/rrsim.c
	zip -r $(ZIP) $(ZIPBASE)
	rm -rf $(ZIPBASE)

clean:
	rm -f $(OBJS) rrsim_skel.c
	rm -rf $(ZIPBASE)

pristine: clean
	rm -f $(TARGET)

