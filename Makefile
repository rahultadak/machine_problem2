CC = g++
OPT = -O3
OPT = -g
WARN = -Wall
#ERR = -Werror

CFLAGS = $(OPT) $(WARN) $(ERR) $(INC) $(LIB)

SIM_SRC = main.cc cache.cc msi.cc mesi.cc dragon.cc

SIM_OBJ = main.o cache.o msi.o mesi.o dragon.o

all: smp_cache
	@echo "Compilation Done ---> nothing else to make :) "

smp_cache: $(SIM_OBJ)
	$(CC) -o smp_cache $(CFLAGS) $(SIM_OBJ) -lm
	@echo "----------------------------------------------------------"
	@echo "-----------FALL14-506 SMP SIMULATOR (SMP_CACHE)-----------"
	@echo "----------------------------------------------------------"
 
.cc.o:
	$(CC) $(CFLAGS)  -c $*.cc

clean:
	rm -f *.o smp_cache

clobber:
	rm -f *.o


