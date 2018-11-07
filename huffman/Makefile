CC = cc
CFLAGs = -g -O2

.SUFFIXES: .c .o

.PHONY: all clean

all: freq hcode_freq

.c.o:
	$(CC) $(CFLAGs) -c $<

freq: freq.o
	$(CC) $(CFLAGs) -o $@ $^ $(LIBs)

hcode_freq: hcode_freq.o
	$(CC) $(CFLAGs) -o $@ $^ $(LIBs)

clean:
	$(RM) -f *.o *.so *~ *.bak
