CPP = g++
CFLGA = -lm -g -Wall -Wsign-compare -Wformat -std=c++0x -O2

SRC=fastrr.cc rulesutils.cc fdd.cc set.cc
OBJ=$(SRC:.cc=.o)


%.o: %.cc
	${CPP} ${CFLGA} -c $^ -o $@ 

sp: ${OBJ}
	${CPP} ${CFLGA} -o fastrr ${OBJ} 

all: fastrr 

clean: 
	rm -f *.o
	rm -f fastrr 
