CC:=arm-linux-gnueabihf-gcc
CXX:=arm-linux-gnueabihf-g++

SRCS:=${wildcard src/*.cpp}
SRCDIR:=src

INCLUDEDIR:=include

OBJDIR:=build
OBJ_FILES := $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SRCS))

LIBS := ${wildcard ./3rdParty/lib/lib*.a}
CMD_LIBS := $(patsubst ./3rdParty/lib/lib%.a,-l%, ${LIBS})

OPTS:= -Iinclude -pthread -std=c++2a -Wno-psabi -L./3rdParty/lib ${CMD_LIBS} 

.PHONY: default

default: ${OBJ_FILES}
	if not exist bin mkdir bin
	${CXX} $^ main/main.cpp -o bin/mocap ${OPTS}


${OBJDIR}/%.o: ${SRCDIR}/%.cpp
	if not exist build mkdir build
	g++ $^ -c -o $@ ${OPTS}

%: ${OBJ_FILES} tools/%.cpp
	if not exist bin mkdir bin
	${CXX} ${OBJ_FILES} $^ -o bin/$@ ${OPTS}