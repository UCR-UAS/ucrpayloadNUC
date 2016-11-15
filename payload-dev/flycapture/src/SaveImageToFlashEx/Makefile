# SaveImageToFlashEx makefile
CC = g++
OUTPUTNAME = SaveImageToFlashEx${D}
INCLUDE = -I../../include -I/usr/include/flycapture
LIBS = -L../../lib -lflycapture${D} ${FC2_DEPS}

OUTDIR = ../../bin

OBJS = SaveImageToFlashEx.o

${OUTPUTNAME}: ${OBJS}
	${CC} -o ${OUTPUTNAME} ${OBJS} ${LIBS} ${COMMON_LIBS}
ifneq (${D}, d)
	strip --strip-unneeded ${OUTPUTNAME}
endif
	mv ${OUTPUTNAME} ${OUTDIR}

%.o: %.cpp
	${CC} ${CFLAGS} ${INCLUDE} -Wall -c $*.cpp

clean_obj:
	rm -f ${OBJS}	@echo "all cleaned up!"

clean:
	rm -f ${OUTDIR}/${OUTPUTNAME} ${OBJS}	@echo "all cleaned up!"
