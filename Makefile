# Makefile for dataflow2
# Copyright (C) 2024  Xu Ruijun
#
# Copying and distribution of this file, with or without modification,
# are permitted in any medium without royalty provided the copyright
# notice and this notice are preserved.  This file is offered as-is,
# without any warranty.

CXXFLAGS = -Iinclude -g

all: test

buffer.o: src/buffer.cpp
	${CXX} ${CXXFLAGS} $^ -c -o $@

test: src/test.cpp buffer.o
	${CXX} ${CXXFLAGS} $^ -o $@

clean:
	rm buffer.o test
