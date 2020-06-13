CC=g++
CF=-Wall -g
MODEL=models/

main: newcloudrf.cc ${MODEL}fspl.o 
	${CC} ${CF} newcloudrf.cc ${MODEL}fspl.o -o main

fspl.o: ${MODEL}fspl.cc ${MODEL}fspl.hh
	${CC} ${CF} ${MODEL}fspl.cc -o fspl.o	
clean:
	rm -f main *.o