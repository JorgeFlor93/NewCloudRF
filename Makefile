CC=g++
CF=-Wall -g
MODEL=models/
BUILD=build/

main: appPathloss.cc ${MODEL}fspl.o ${MODEL}hata.o inputs.o outputs.o auxFuncts.o distance.o tiles.o coordinate.o txrx.o common.h 
	${CC} ${CF} appPathloss.cc ${MODEL}fspl.o ${MODEL}hata.o ${BUILD}inputs.o ${BUILD}outputs.o ${BUILD}distance.o ${BUILD}tiles.o ${BUILD}coordinate.o ${BUILD}txrx.o ${BUILD}auxFuncts.o -o main

fspl.o: ${MODEL}fspl.cc ${MODEL}fspl.hh
	${CC} ${CF} -c ${MODEL}fspl.cc -o ${MODEL}fspl.o

hata.o: ${MODEL}hata.cc ${MODEL}hata.hh
	${CC} ${CF} -c ${MODEL}hata.cc -o ${MODEL}hata.o

auxFuncts.o: auxFuncts.cc auxFuncts.hh 
	${CC} ${CF} -c	auxFuncts.cc -o ${BUILD}auxFuncts.o

inputs.o: inputs.cc inputs.hh 
	${CC} ${CF} -c inputs.cc -o ${BUILD}inputs.o

outputs.o: outputs.cc outputs.hh 
	${CC} ${CF} -c outputs.cc -o ${BUILD}outputs.o

distance.o: distance.cc distance.hh
	${CC} ${CF} -c distance.cc -o ${BUILD}distance.o

tiles.o: tiles.cc tiles.hh
	${CC} ${CF} -c tiles.cc -o ${BUILD}tiles.o

coordinate.o: coordinate.cc coordinate.hh
	${CC} ${CF} -c coordinate.cc -o ${BUILD}coordinate.o

txrx.o: txrx.cc txrx.hh
	${CC} ${CF} -c txrx.cc -o ${BUILD}txrx.o

clean:
	rm -f main ${MODEL}*.o ${TOOLS}*.o ${BUILD}*.o *.o *.gp *.png