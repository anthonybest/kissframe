VPATH = $(SRCDIR)
SRCS = main.c fcs16.c frame.c protocol.c
OBJS = ${SRCS:.c=.o}
DEPS = $(SRCS:.c=.d)

override CFLAGS := -MD -MP -Wall -Werror $(CFLAGS)
override LDFLAGS := $(LDFLAGS)
TARGET = kissframe

.PHONY: all clean distclean


all:${TARGET}

${TARGET}: ${OBJS}
	$(CC) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o $@

%.o : %.c $(MAKEFILE_LIST) $(OTHER_MAKEFILES)
	$(COMPILE.c) $< -o $@


-include $(DEPS)


clean::
	-rm -f ${OBJS} ${DEPS} ${TARGET}

distclean:: clean

