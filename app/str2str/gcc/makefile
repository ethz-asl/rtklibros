# makefile for str2str

BINDIR = /usr/local/bin
SRC    = ../../../src

# for beagleboard
#CTARGET= -mfpu=neon -mfloat-abi=softfp -ffast-math
CTARGET=

CFLAGS = -Wall -O3 -ansi -pedantic -I$(SRC) -DTRACE $(CTARGET)
LDLIBS  = -lm -lpthread

all        : str2str
str2str    : str2str.o stream.o rtkcmn.o solution.o sbas.o geoid.o
str2str    : rcvraw.o novatel.o ublox.o ss2.o crescent.o skytraq.o gw10.o javad.o
str2str    : streamsvr.o

str2str.o  : ../str2str.c
	$(CC) -c $(CFLAGS) ../str2str.c
stream.o   : $(SRC)/stream.c
	$(CC) -c $(CFLAGS) $(SRC)/stream.c
streamsvr.o: $(SRC)/streamsvr.c
	$(CC) -c $(CFLAGS) $(SRC)/streamsvr.c
rtkcmn.o   : $(SRC)/rtkcmn.c
	$(CC) -c $(CFLAGS) $(SRC)/rtkcmn.c
solution.o : $(SRC)/solution.c
	$(CC) -c $(CFLAGS) $(SRC)/solution.c
sbas.o     : $(SRC)/sbas.c
	$(CC) -c $(CFLAGS) $(SRC)/sbas.c
geoid.o    : $(SRC)/geoid.c
	$(CC) -c $(CFLAGS) $(SRC)/geoid.c
rcvraw.o   : $(SRC)/rcvraw.c
	$(CC) -c $(CFLAGS) $(SRC)/rcvraw.c
novatel.o  : $(SRC)/rcv/novatel.c
	$(CC) -c $(CFLAGS) $(SRC)/rcv/novatel.c
ublox.o    : $(SRC)/rcv/ublox.c
	$(CC) -c $(CFLAGS) $(SRC)/rcv/ublox.c
ss2.o      : $(SRC)/rcv/ss2.c
	$(CC) -c $(CFLAGS) $(SRC)/rcv/ss2.c
crescent.o : $(SRC)/rcv/crescent.c
	$(CC) -c $(CFLAGS) $(SRC)/rcv/crescent.c
skytraq.o  : $(SRC)/rcv/skytraq.c
	$(CC) -c $(CFLAGS) $(SRC)/rcv/skytraq.c
gw10.o     : $(SRC)/rcv/gw10.c
	$(CC) -c $(CFLAGS) $(SRC)/rcv/gw10.c
javad.o    : $(SRC)/rcv/javad.c
	$(CC) -c $(CFLAGS) $(SRC)/rcv/javad.c

str2str.o  : $(SRC)/rtklib.h
stream.o   : $(SRC)/rtklib.h
streamsvr.o: $(SRC)/rtklib.h
rtkcmn.o   : $(SRC)/rtklib.h
solution.o : $(SRC)/rtklib.h
sbas.o     : $(SRC)/rtklib.h
geoid.o    : $(SRC)/rtklib.h
rcvraw.o   : $(SRC)/rtklib.h
novatel.o  : $(SRC)/rtklib.h
ublox.o    : $(SRC)/rtklib.h
ss2.o      : $(SRC)/rtklib.h
crescent.o : $(SRC)/rtklib.h
skytraq.o  : $(SRC)/rtklib.h
gw10.o     : $(SRC)/rtklib.h
javad.o    : $(SRC)/rtklib.h

install:
	cp str2str $(BINDIR)

#DEVICE=ttyACM0
DEVICE=ttyS1:115200:8:n:1:off
#DEVICE=ttyUSB0:115200:8:n:1:off

test1:
	./str2str -in serial://$(DEVICE) -out file://test1.out -t 2
test2:
	./str2str -in serial://$(DEVICE) -out tcpsvr://:2102 -c ../../../data/oem4_raw_10hz.cmd -t 5
test3:
	./str2str -in serial://$(DEVICE) -out ntrips://:sesam@localhost:8000/BUCU0 -c ../../../data/oem4_raw_10hz.cmd -t 5
clean:
	rm -f str2str str2str.exe *.o *.out *.trace

