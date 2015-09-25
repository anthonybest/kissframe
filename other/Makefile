
SRCS = frame.c fcs16.c main.c
OBJS = ${SRCS:.c=.o}
DEPS = $(SRCS:.c=.d)

CFLAGS = -g
override CFLAGS := -MD -MP -Wall -Werror $(CFLAGS)
override LDFLAGS := $(LDFLAGS)
TARGET = kiss


.PHONY: all clean distclean

all:${TARGET}

${TARGET}: ${OBJS}
	$(CC) $(LDFLAGS) $^ $(LOADLIBS) $(LDLIBS) -o $@

%.o : %.c 
	$(COMPILE.c) $< -o $@

-include $(DEPS)
