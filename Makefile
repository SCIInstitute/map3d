# Makefile.linux
#
# Written by Chris Moulding, June 2000
# Last update: Mon Dec 11 21:35:40 2000 by Rob MacLeod
#    - updated by Rob to use with central libaries (CVRTI setup)

include Makefile.obj # Holds the OBJS variable
#######################	 EDIT HERE  #########################
# if MASTER_FLAGS_SET is set, then use the vars from the external makefile
ifeq ("$(MASTER_FLAGS_SET)","yes")
  include ../Makefile.incl
else
  ifneq ("$(MASTER_FLAGS_SET)","yes")
    MAP3D_INC=-I$(ROBHOME)/include
    MAP3D_LIB=-L$(ROBHOME)/lib
    OPT_FLAGS=-g
    OS=$(shell uname -s)
    # check is redundant, but we can pattern-match this and get rid of it for the release
    ifeq ("$(OS)","Darwin")
      CXX=g++
      ARCH_FLAGS=-DOSX -MMD -I/usr/X11R6/include -I/sw/include
      SYSEXT=-osx

      GTK_INC   = /sw/include
      GTK_LIB   = /sw/lib
      #GTKGL_INC = -I/sw/include/gtkglext-1.0 -I/sw/lib/gtkglext-1.0/include
      #GTKGL_LIB =
      GTKGL_INC=/sw/include
      GTKGL_LIB=/sw/lib
      #LOCAL_INC = -I/usr/sci/local/include -I/usr/sci/projects/map3d/include
      #LOCAL_LIB = -L/usr/sci/local/lib -L/usr/sci/projects/map3d/lib
      LOCAL_INC=/usr/local/include
      LOCAL_LIB=/usr/local/lib
    else
    ifeq ("$(OS)","IRIX64")
      CXX=CC
      # grap "sci" or "cvrti" from hostname
      HOST = $(shell hostname -d | awk -F "." '{print $1}')
      #### SCI setup ####
      ifeq ($(HOST),sci)
        GTK_INC   = /usr/sci/local/include
        GTK_LIB   = /usr/sci/local/lib
        GTKGL_INC = -I/usr/sci/projects/map3d/include/gtk-2.0
        GTKGL_LIB = -Wl,-rpath -Wl,${GTK_LIB} \
                    -Wl,-rpath -Wl,/usr/sci/projects/map3d/sgi/lib \
                    -L/usr/sci/projects/map3d/sgi/lib
        LOCAL_INC = -I/usr/sci/local/include -I/usr/sci/projects/map3d/include
        LOCAL_LIB = -L/usr/sci/local/lib -L/usr/sci/projects/map3d/lib

      else
      #### CVRTI setup ####
      ifeq ($(HOST),cvrti)
        GTK_INC   = /usr/freeware/include
        GTK_LIB   = /usr/freeware/lib32
        GTKGL_INC = -I/usr/local/include/gtk-2.0 -I/usr/local/lib32/gtk-2.0/include
        GTKGL_LIB = -Wl,-rpath -Wl,${GTK_LIB} \
                    -Wl,-rpath -Wl,/usr/local/lib32 -L/usr/local/lib32
        LOCAL_INC = -I/usr/freeware/include -I/usr/local/include
        LOCAL_LIB = -L/usr/local/lib
      endif
      endif
      SYSEXT=-sgi
      ARCH_FLAGS=-DUNIX -n32 -LANG:std -MDupdate depend.mk
      LFLAGS=-n32 -LANG:std
    else
    ifeq ("$(OS)","Linux")
      CXX=g++
      ARCH_FLAGS=-DLINUX -MMD
      SYSEXT=-linux
      GTK_INC   = /usr/include
      GTK_LIB   = /usr/include
      GTKGL_INC = -I/usr/local/include/ -I/usr/local/include/gtkglext-1.0/ -I/usr/local/lib/gtkglext-1.0/include/
      GTKGL_LIB = -Wl,-rpath -Wl,${GTK_LIB} \
                  -Wl,-rpath -Wl,/usr/local/lib -L/usr/local/lib
      LOCAL_INC = -I/usr/sci/projects/map3d/include
      LOCAL_LIB = -L/usr/sci/projects/map3d/lib
      LFLAGS = -lpthread
    else
      OS=Unknown
    endif
    endif
    endif

  endif
endif

EXENAME  = map3d
REV := $(shell svn info | grep Revision | sed "s/Revision\:\ //g")



LIBS     = -lgtk-X11-2.0 -lgdk-X11-2.0 -lgdk_pixbuf-2.0 -lgdkglext-x11-1.0 -lgtkglext-x11-1.0 \
	   -lpango-1.0 -lglib-2.0 -lgobject-2.0 -lgmodule-2.0 \
           -lGLU -lGL \
	   -lXi -lXext -lXmu -lX11 \
	   -lgfile${SYSEXT} -lfids${SYSEXT} -lfi${SYSEXT} \
           -ltsdf${SYSEXT} -lcutil${SYSEXT} -lgraphicsio${SYSEXT} \
           -lMatlabIO${SYSEXT} -lgdbmp${SYSEXT} -lm -lpng -lz -ljpeg

ifeq ($(OS),IRIX64)
  LIBS += -lGLcore
endif

LIBPATHS =  -L/usr/X11R6/lib -L${GTK_LIB} -L${GTKGL_LIB} -L${LOCAL_LIB} ${MAP3D_LIB}

ifneq ($(OS),Darwin)
  LIBPATHS += -Wl,-rpath -Wl,${GTK_LIB} -Wl,-rpath -Wl,${GTKGL_LIB}
endif

INCPATHS = -I${GTK_INC}/gtk-2.0 \
	-I${GTK_INC}/glib-2.0 \
	-I${GTK_INC}/pango-1.0 \
	-I${GTK_INC}/atk-1.0 \
	-I${GTK_INC}/cairo \
	-I${GTK_INC} \
	-I${GTK_LIB}/gtk-2.0/include \
	-I${GTK_LIB}/glib-2.0/include \
	-I${GTKGL_INC}/gtk-2.0 \
	-I${GTKGL_INC}/gtkglext-1.0 \
	-I${GTKGL_LIB}/gtkglext-1.0/include \
	-I${LOCAL_INC} ${MAP3D_INC}

CFLAGS= -c $(ARCH_FLAGS) $(OPT_FLAGS) $(INCPATHS) -DREVISION=${REV}
RM = rm -f

##############################################################################
#
# The description blocks
#

.SUFFIXES: .c .cc

all: $(EXENAME)

$(EXENAME):  $(OBJS)
	$(CXX) $(LFLAGS) $(LIBPATHS) $(OBJS) $(LIBS) -o $(EXENAME)

.c.o:
	$(CXX) $(CFLAGS) $<

.cc.o:
	$(CXX) $(CFLAGS) $<

ifeq ($(OS), IRIX64)
  -include depend.mk 
else
  -include $(SRCS:.cc=.d) 
endif


##############################################################################
#
# Utilities
#

clean:
	-$(RM) *.o
	-$(RM) *.d

binclean: 
	-$(RM) $(EXENAME) map3d

allclean: clean binclean


tar maketar: 
	echo "Tarring files into  ${EXENAME}_source.tar "
	tar cvf  ${EXENAME}_source.tar ${OBJS:.o=.cc} *.h  Makefile*



