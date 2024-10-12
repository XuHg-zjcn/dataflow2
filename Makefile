CXXFLAGS = -Iinclude -g

all: test

buffer.o: src/buffer.cpp
	${CXX} ${CXXFLAGS} $^ -c -o $@

test: src/test.cpp buffer.o
	${CXX} ${CXXFLAGS} $^ -o $@

clean:
	rm buffer.o test
