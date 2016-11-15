# A more complex Makefile
CC = g++

GCHNAME = Precompiled.h.gch
GCH_INCLUDES = `pkg-config --cflags gtkmm-2.4` `pkg-config --cflags libglademm-2.4` `pkg-config --cflags giomm-2.4`

INCLUDE = -I../../include -I/usr/include/flycapture ${GCH_INCLUDES}

LIBS = -L../../lib -lflycapture${D} -lflycapturegui${D} -lpthread `pkg-config --libs gthread-2.0` `pkg-config --libs giomm-2.4` `pkg-config gtkmm-2.4 --libs` `pkg-config libglademm-2.4 --libs` ${FC2_DEPS}

NAME = SerialPortEx${D}

XMLNAME = SerialPortEx

OUTDIR = ../../bin

OBJS = main.o SerialPortWindow.o

all: ${GCHNAME} ${NAME} glade

${GCHNAME}: Precompiled.h
	${CC} ${CFLAGS} ${GCH_INCLUDES} -Wall -c Precompiled.h

${NAME}: ${OBJS}
	${CC} -Wall -o ${NAME} ${OBJS} ${LIBS} ${COMMON_LIBS} -export-dynamic
ifneq (${D}, d)
	strip --strip-unneeded ${NAME}
endif
	mv ${NAME} ${OUTDIR}/${NAME}

%.o: %.cpp
	${CC} ${CFLAGS} ${INCLUDE} -Wall -c $*.cpp

glade:
	cp ${XMLNAME}.glade ${OUTDIR}/${XMLNAME}.glade

clean_obj:
	rm -f ${OBJS} Precompiled.h.gch

clean:
	rm -f ${OUTDIR}/${NAME} ${OBJS} Precompiled.h.gch
