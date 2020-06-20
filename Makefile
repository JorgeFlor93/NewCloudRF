CC=g++
CF=-Wall -g
MODEL=models/
TOOLS=Prop-tools/

main: newcloudrf.cc ${MODEL}fspl.o ${MODEL}hata.o inputs.o main.o common.h
	${CC} ${CF} newcloudrf.cc ${MODEL}fspl.o ${MODEL}hata.o inputs.o main.o -o main

fspl.o: ${MODEL}fspl.cc ${MODEL}fspl.hh
	${CC} ${CF} -c ${MODEL}fspl.cc -o ${MODEL}fspl.o

hata.o: ${MODEL}hata.cc ${MODEL}hata.hh
	${CC} ${CF} -c ${MODEL}hata.cc -o ${MODEL}hata.o

main.o: main.cc main.hh 
	${CC} ${CF} -c	main.cc -o main.o

inputs.o: inputs.cc inputs.hh 
	${CC} ${CF} -c inputs.cc -o inputs.o

clean:
	rm -f main ${MODEL}*.o ${TOOLS}*.o *.o