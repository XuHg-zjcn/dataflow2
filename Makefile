# Makefile for dataflow2
# Copyright (C) 2024-2025  Xu Ruijun
#
# Copying and distribution of this file, with or without modification,
# are permitted in any medium without royalty provided the copyright
# notice and this notice are preserved.  This file is offered as-is,
# without any warranty.

CXXFLAGS = -Iinclude -g -DWHEAD_FORCE_ONLY=1
CFLAGS = -Iinclude -g -DWHEAD_FORCE_ONLY=1
OBJS = buffer.o writer.o misc.o sender.o source.o

all: test view

%.o: src/%.cpp
	${CXX} ${CXXFLAGS} $^ -c -o $@

%.o: src/%.c
	${CC} ${CFLAGS} $^ -c -o $@

test: src/test.cpp ${OBJS}
	${CXX} ${CXXFLAGS} $^ -lcjson -o $@

view: src/view.cpp ${OBJS}
	${CXX} ${CXXFLAGS} $^ -lcjson -o $@

clean:
	rm *.o test
