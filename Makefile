
CC ?= gcc

PNGLIBS:=$(shell pkg-config --libs libpng)
PNGCFLAGS:=$(shell pkg-config --cflags libpng)

O?=1

ifeq (${O},1)
DEBUGFLAG=
OPTIMIZEFLAG=-O3
else
DEBUGFLAG=-g
OPTIMIZEFLAG=
endif

ifeq (${P},1)
PROFILEFLAG=-pg
OPTIMIZEFLAG=-O3
DEBUGFLAG=
else
PROFILEFLAG=
endif

ifeq (${E},1)
STOP_ON_WARN=-Werror
else
STOP_ON_WARN=
endif

# -rdynamic is used by gcc for runtime stack traces (see stacktrace.c)
# but clang complains about it.
ifeq (${CC},clang)
RDYNAMIC=
else
RDYNAMIC=-rdynamic
endif

ifeq (${V},1)
Q=
ECHO=echo
else
Q=@
ECHO=echo
endif

MYCFLAGS=-DPREFIX=${PREFIX} ${DEBUGFLAG} ${PROFILEFLAG} ${OPTIMIZEFLAG}\
	--pedantic -Wall ${STOP_ON_WARN} -pthread -std=gnu99 ${RDYNAMIC} \
	-Wno-extended-offsetof -Wno-gnu-folding-constant $(CFLAGS)


all:	ggvf_to_flowmap

OBJS=ggvf_to_flowmap.o quat.o mtwist.o mathutils.o mikktspace/mikktspace.o mesh.o matrix.o vec4.o open-simplex-noise.o png_utils.o

GGLIBS=-lm ${LRTLIB} -lpng
GGLINK=$(ECHO) '  LINK' $@ && $(CC) ${MYCFLAGS} -o $@ ${OBJS} ${GGLIBS} $(LDFLAGS)

.c.o:
	$(Q)$(ECHO) '  COMPILE' $@ && $(CC) -c $(MYCFLAGS) $(CPPFLAGS) -o $@ $<

ggvf_to_flowmap:	${OBJS}
	$(Q)$(GGLINK)

mikktspace/mikktspace.o:
	(cd mikktspace && ${MAKE} )

clean:
	rm -f *.o ggvf_to_flowmap mikktspace/*.o

