CC=g++
CF=-Wall -g
MODEL=models/

main: newcloudrf.cc ${MODEL}fspl.o ${MODEL}hata.o
	${CC} ${CF} newcloudrf.cc ${MODEL}fspl.o  ${MODEL}hata.o -o main

fspl.o: ${MODEL}fspl.cc ${MODEL}fspl.hh
	${CC} ${CF} ${MODEL}fspl.cc -o fspl.o

hata.o: ${MODEL}hata.cc ${MODEL}hata.hh
	${CC} ${CF} ${MODEL}hata.cc -o hata.o	

clean:
	rm -f main *.o