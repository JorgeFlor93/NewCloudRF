CC=g++
CF=-Wall -g
MODEL=models/
TOOLS=Prop-tools/

main: newcloudrf.cc ${MODEL}fspl.o ${MODEL}hata.o ${TOOLS}prop-tools.o
	${CC} ${CF} newcloudrf.cc ${MODEL}fspl.o ${MODEL}hata.o ${TOOLS}prop-tools.o -o main

fspl.o: ${MODEL}fspl.cc ${MODEL}fspl.hh
	${CC} ${CF} -c ${MODEL}fspl.cc -o ${MODEL}fspl.o

hata.o: ${MODEL}hata.cc ${MODEL}hata.hh
	${CC} ${CF} -c ${MODEL}hata.cc -o ${MODEL}hata.o	

prop-tools.o: ${TOOLS}prop-tools.cc ${TOOLS}prop-tools.hh
	${CC} ${CF} -c ${TOOLS}prop-tools.cc -o ${TOOLS}prop-tools.o 


clean:
	rm -f main ${MODEL}*.o ${TOOLS}*.o