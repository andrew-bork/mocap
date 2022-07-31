CC:=arm-linux-gnueabihf-gcc
CXX:=arm-linux-gnueabihf-g++

SRCS:=${wildcard src/*.cpp}
SRCDIR:=src

INCLUDEDIR:=include ../3rdParty/fmt/include ../3rdParty/libi2c/include

INCLUDEFLAG:=${patsubst %,-I%, ${INCLUDEDIR}}

OBJDIR:=build
OBJ_FILES := $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SRCS))

LIBS := ${wildcard ../3rdParty/lib/lib*.a}
CMD_LIBS := $(patsubst ../3rdParty/lib/lib%.a,-l%, ${LIBS})

OPTS:= ${INCLUDEFLAG} -posix -lpthread -pthread -std=c++2a -Wno-psabi -L../3rdParty/lib ${CMD_LIBS} -static-libstdc++

.PHONY: default

default: ${OBJ_FILES}
	mkdir -p bin
	${CXX} $^ main/main.cpp -o bin/mocap ${OPTS}


${OBJDIR}/%.o: ${SRCDIR}/%.cpp
	mkdir -p build
	${CXX} $^ -c -o $@ ${OPTS}

%: ${OBJ_FILES} tools/%.cpp
	mkdir -p bin
	${CXX} $^ -o bin/$@ ${OPTS}

test:
	mkdir -p bin
	${CXX} tools/test.cpp -o bin/test ${OPTS}