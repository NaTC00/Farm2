#SHELL = /bin/bash

CC = gcc

INCLUDE = -I ./HEADER
CFLAGS = -Wall -pedantic -std=c99 -pthread $(INCLUDE)


EXEC1 = farm
EXEC2 = collector
EXEC3 = generafile

Q = -q 8

N = -n 4

T = -t 1000

D = -d testdir

DIR = testdir


OBJECT = OBJECT/masterWorker.o OBJECT/myThreadPool.o OBJECT/util.o OBJECT/worker.o OBJECT/collector.o OBJECT/list.o

FILE = file1.dat file2.dat file3.dat file4.dat file5.dat file10.dat file12.dat file13.dat file14.dat file15.dat file16.dat file17.dat file18.dat file20.dat file100.dat file116.dat file117.dat

.PHONY : clean cleanall test generafile exec valg

all: $(EXEC1) $(EXEC2) $(EXEC3) 

$(EXEC1) : OBJECT/masterWorker.o OBJECT/myThreadPool.o OBJECT/util.o OBJECT/worker.o
	$(CC) $(CFLAGS) $^ -o $(EXEC1)
	
$(EXEC2) : OBJECT/collector.o OBJECT/util.o OBJECT/list.o
	$(CC) $(CFLAGS) $^ -o $(EXEC2)
	
$(EXEC3) : generafile.o
	$(CC) $(CFLAGS) $^ -o $(EXEC3)
	
	
generafile.o: generafile.c
	$(CC) -std=c99 generafile.c -c generafile.c

OBJECT/util.o : SOURCE/util.c HEADER/util.h 
	$(CC) $(CFLAGS) -c $< -o OBJECT/util.o

OBJECT/myThreadPool.o : SOURCE/myThreadPool.c HEADER/myThreadPool.h HEADER/util.h 
	$(CC) $(CFLAGS) -c $< -o OBJECT/myThreadPool.o 

OBJECT/worker.o : SOURCE/worker.c HEADER/worker.h HEADER/util.h HEADER/collector.h HEADER/myThreadPool.h
	$(CC) $(CFLAGS) -c $< -o OBJECT/worker.o 
	
OBJECT/list.o : SOURCE/list.c HEADER/list.h 
	$(CC) $(CFLAGS) -c $< -o OBJECT/list.o 

OBJECT/collector.o : SOURCE/collector.c HEADER/util.h HEADER/collector.h HEADER/list.h
	$(CC) $(CFLAGS) -c $< -o OBJECT/collector.o

OBJECT/masterWorker.o : SOURCE/masterWorker.c HEADER/util.h HEADER/collector.h HEADER/myThreadPool.h HEADER/worker.h 
	$(CC) $(CFLAGS) -c $< -o OBJECT/masterWorker.o

test :	farm collector 
	@chmod +x ./test.sh
	./test.sh
	
mytest: farm collector 
	@chmod +x ./my_test.sh
	./my_test.sh	
	
clean :
	-rm -f *~ *.dat farm2.sck expected.txt core \
	rm -r $(DIR)

cleanall :
	-rm -f $(EXEC1) $(EXEC2) generafile $(OBJECT) generafile.o *~ *.dat testdir ./farm2.sck expected.txt core \
	rm -r $(DIR)

exec: 
	./$(EXEC1) $(D) $(T) $(N) $(Q) $(FILE)

valg:
	valgrind --error-exitcode=1 --leak-check=full ./$(EXEC1) $(D) $(T) $(N) $(Q) $(FILE)
	




