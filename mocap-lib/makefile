CC:=arm-linux-gnueabihf-gcc
CXX:=arm-linux-gnueabihf-g++

SRCS:=${wildcard src/*.cpp}
SRCDIR:=src

INCLUDEDIR:=include ../3rdParty/fmt/include ../3rdParty/i2c-tools/include

INCLUDEFLAG:=${patsubst %,-I%, ${INCLUDEDIR}}

OBJDIR:=build
OBJ_FILES := $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SRCS))

LIBS := ${wildcard ./3rdParty/lib/lib*.a}
CMD_LIBS := $(patsubst ./3rdParty/lib/lib%.a,-l%, ${LIBS})

OPTS:= ${INCLUDEFLAG} -posix -lpthread -pthread -std=c++2a -Wno-psabi -L./3rdParty/lib ${CMD_LIBS} -D _GLIBCXX_USE_PTHREAD_MUTEX_CLOCKLOCK

.PHONY: default

default: ${OBJ_FILES}
	if not exist bin mkdir bin
	${CXX} $^ main/main.cpp -o bin/mocap ${OPTS}


${OBJDIR}/%.o: ${SRCDIR}/%.cpp
	if not exist build mkdir build
	g++ $^ -c -o $@ ${OPTS}

%: ${OBJ_FILES} tools/%.cpp
	if not exist bin mkdir bin
	${CXX} ${OBJ_FILES} $^ -o bin/$@ -H ${OPTS}

test:
	if not exist bin mkdir bin
	${CXX} tools/test.cpp -o bin/test -H ${OPTS}