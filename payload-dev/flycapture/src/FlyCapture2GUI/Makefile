# First itteration of the flycapture2 makefile
# To compile the debug verison need to overwrite CXXFLAGS variable to include -ggdb

# version of the software
MAJOR_VERSION = 2
MINOR_VERSION = 6
# 0 Alpha, 1 Beta, 2 RC, 3 Public release
RELEASE_TYPE = 3
RELEASE_BUILD = 4
VERSION_NUMBER = ${MAJOR_VERSION}.${MINOR_VERSION}.${RELEASE_TYPE}.${RELEASE_BUILD}

ifeq ($(PLATFORM),arm)
	CC = arm-linux-gnueabi-g++
	STRIP = arm-linux-gnueabi-strip
	CFLAGS += -mfloat-abi=softfp
	CFLAGS += -UMACHINE_ARCH_X32 -UMACHINE_ARCH_X64 -DMACHINE_ARCH_ARM
else
	ifeq ($(PLATFORM),armhf)
		CC = arm-linux-gnueabihf-g++
		STRIP = arm-linux-gnueabihf-strip
		CFLAGS += -mfloat-abi=hard
		CFLAGS += -UMACHINE_ARCH_X32 -UMACHINE_ARCH_X64 -DMACHINE_ARCH_ARM
	else
		CC = g++
		STRIP = strip
	endif
endif

LDFLAGS = -Wl,-z,defs,-soname,${SONAME}

LINKNAME = libflycapturegui${D}.so
SONAME = libflycapturegui${D}.so.${MAJOR_VERSION}
REALNAME = libflycapturegui${D}.so.${VERSION_NUMBER}
GCHNAME = Precompiled.h.gch

SOFTWARELIB = /softwarelib

ifeq ($(PLATFORM),arm)
	GTK_ROOT = $(SOFTWARELIB)/libgtk/precise/arm
	GTK_SUFFIX = arm-linux-gnueabi
else
	ifeq ($(PLATFORM),armhf)
		GTK_ROOT = $(SOFTWARELIB)/libgtk/precise/armhf
		GTK_SUFFIX = arm-linux-gnueabihf
	else
		ifeq ($(PLATFORM),amd64)
			GTK_ROOT = $(SOFTWARELIB)/libgtk/precise/amd64
			GTK_SUFFIX = x86_64-linux-gnu
		else
			GTK_ROOT = $(SOFTWARELIB)/libgtk/precise/i386
			GTK_SUFFIX = i386-linux-gnu
		endif
	endif
endif

ifeq ($(PLATFORM),)
GTK_INCLUDE = `pkg-config --cflags gtkmm-2.4` `pkg-config --cflags giomm-2.4` `pkg-config --cflags libglademm-2.4`
else
GTK_INCLUDE = -I${GTK_ROOT}/usr/include \
	-I${GTK_ROOT}/usr/include/atk-1.0 \
	-I${GTK_ROOT}/usr/include/atkmm-1.6 \
	-I${GTK_ROOT}/usr/include/cairo \
	-I${GTK_ROOT}/usr/include/cairomm-1.0 \
	-I${GTK_ROOT}/usr/include/fontconfig \
	-I${GTK_ROOT}/usr/include/freetype2 \
	-I${GTK_ROOT}/usr/include/gdkmm-2.4 \
	-I${GTK_ROOT}/usr/include/gdk-pixbuf-2.0 \
	-I${GTK_ROOT}/usr/include/giomm-2.4 \
	-I${GTK_ROOT}/usr/include/glib-2.0 \
	-I${GTK_ROOT}/usr/include/glibmm-2.4 \
	-I${GTK_ROOT}/usr/include/gtk-2.0 \
	-I${GTK_ROOT}/usr/include/gtk-2.0/gtk/gtk \
	-I${GTK_ROOT}/usr/include/gtkmm-2.4 \
	-I${GTK_ROOT}/usr/include/libglademm-2.4 \
	-I${GTK_ROOT}/usr/include/pango-1.0 \
	-I${GTK_ROOT}/usr/include/pangomm-1.4 \
	-I${GTK_ROOT}/usr/include/sigc++-2.0 \
	-I${GTK_ROOT}/usr/include/sigc++-2.0/sigc++ \
	-I${GTK_ROOT}/usr/lib/${GTK_SUFFIX}/cairomm-1.0/include \
	-I${GTK_ROOT}/usr/lib/${GTK_SUFFIX}/gdkmm-2.4/include \
	-I${GTK_ROOT}/usr/lib/${GTK_SUFFIX}/glib-2.0/include \
	-I${GTK_ROOT}/usr/lib/${GTK_SUFFIX}/glibmm-2.4/include \
	-I${GTK_ROOT}/usr/lib/${GTK_SUFFIX}/gtk-2.0/include \
	-I${GTK_ROOT}/usr/lib/${GTK_SUFFIX}/gtkmm-2.4/include \
	-I${GTK_ROOT}/usr/lib/${GTK_SUFFIX}/pangomm-1.4/include \
	-I${GTK_ROOT}/usr/lib/${GTK_SUFFIX}/sigc++-2.0/include
endif

ifeq ($(PLATFORM),)
GTK_LIBS = `pkg-config --libs gtkmm-2.4` `pkg-config --libs giomm-2.4` `pkg-config --libs libglademm-2.4` `pkg-config --libs gthread-2.0`
else
GTK_LIBS = -L${GTK_ROOT}/usr/lib -lglademm-2.4 -lgdkglext-x11-1.0 -lgdkglextmm-x11-1.2 -lgtkglext-x11-1.0 -lgtkglextmm-x11-1.2 \
	-L${GTK_ROOT}/usr/lib/${GTK_SUFFIX} -latkmm-1.6 -lcairomm-1.0 -lglibmm-2.4 -lpangomm-1.4 -lsigc-2.0 -lgiomm-2.4 -lgthread-2.0 -lgio-2.0 -lgtk-x11-2.0 -lgdkmm-2.4 -lgtkmm-2.4 -lglib-2.0
endif

GCH_INCLUDES = -I../../include -I/usr/include/flycapture ${GTK_INCLUDE}
INCLUDES = ${GCH_INCLUDES}

LIBS = -L../../lib -lflycapture${D} ${GTK_LIBS} ${FC2_DEPS}

OUTDIR = ../../lib

OBJS = FlyCapture2GUI.o CamSelection.o CamControl.o BasePage.o \
	   	Format7Page.o DCAMFormatsPage.o CamSettingsPage.o CamInfoPage.o \
		RegisterPage.o GPIOPage.o AdvCameraSettingsPage.o HDRPage.o LUTPage.o \
		FrameBufferPage.o DataFlashPage.o  \
		SystemInfoPage.o BusTopologyPage.o HelpSupportPage.o LadybugSettingsPage.o \
        Format7DrawingArea.o BusTopologyDrawingArea.o NodeSurface.o

all: ${GCHNAME} ${REALNAME} glade

${GCHNAME}: Precompiled.h
	@echo Creating precompiled header...
	@${CC} ${CFLAGS} ${GCH_INCLUDES} -D_REENTRANT -Wall -fPIC -c Precompiled.h

${REALNAME}: ${OBJS}
	@echo Linking...
	@${CC} ${LDFLAGS} -shared -o ${REALNAME} ${OBJS} ${LIBS} -lc
ifneq (${D}, d)
	@echo Stripping unneeded symbols...
	@${STRIP} --strip-unneeded ${REALNAME}
endif
	@echo Changing permissions...
	@chmod 0644 ${REALNAME}
	@echo Running ldconfig...
	@ldconfig -v -n .
	@echo Creating links...
	@ln -sf ${SONAME} ${LINKNAME}
	@echo Moving to destination...
	@mv ${LINKNAME}* ${OUTDIR}

%.o: %.cpp
	@${CC} ${CFLAGS} ${INCLUDES} -D_REENTRANT -Winvalid-pch -Wall -fPIC -c $*.cpp

glade:
	@echo Copying glade files...
	@cp *.glade ../../bin

install:
	cp ../../lib/libflycapturegui* /usr/lib

uninstall:
	rm /usr/lib/libflycapturegui*

clean_obj:
	@rm -f ${OBJS} Precompiled.h.gch

clean:
	@rm -f ${OUTDIR}/${REALNAME} ${OUTDIR}/${SONAME} ${OUTDIR}/${LINKNAME} ${OBJS} Precompiled.h.gch
