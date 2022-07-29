SRCS:=${wildcard src/*.cpp}
SRCDIR:=src



INCLUDEDIR:=include

OBJDIR:=build
OBJ_FILES := $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SRCS))

LIBS := ${wildcard ./lib/lib*.a}
CMD_LIBS := $(patsubst ./lib/lib%.a,-l%, ${LIBS}) -lfmt

OPTS:= -Iinclude -pthread -lpthread -L../lib -li2c -std=c++2a -Wno-psabi -L./lib ${CMD_LIBS}

.PHONY: drone lib


all: drone

${OBJDIR}/%.o: ${SRCDIR}/%.cpp
	mkdir -p build
	g++ $^ -c -o $@ ${OPTS}

drone: $(OBJ_FILES)
	mkdir -p bin
	g++ main/main.cpp $^ -o bin/drone ${OPTS}

%: ${OBJ_FILES} tools/%.cpp
	mkdir -p bin
	g++ $^ -o bin/$@ ${OPTS}

# rampup: ${SRCS} tools/rampup.cpp
# 	mkdir -p bin
# 	g++ $^ -o bin/rampup ${OPTS}

# high_speed: ${SRCS} tools/high_speed.cpp
# 	mkdir -p bin
# 	g++ $^ -o bin/high_speed ${OPTS}

# low_speed: ${SRCS} tools/low_speed_test.cpp
# 	mkdir -p bin
# 	g++ $^ -o bin/low_speed ${OPTS}

# arm: ${SRCS} tools/arm.cpp
# 	mkdir -p bin
# 	g++ $^ -o bin/arm ${OPTS}

# calibration: ${SRCS} tools/calibrate.cpp
# 	mkdir -p bin
# 	g++ $^ -o bin/calibrate ${OPTS}

# pulse: ${SRCS} tools/pulse.cpp
# 	mkdir -p bin
# 	g++ $^ -o bin/pulse ${OPTS}

# pressure: ${SRCS} tools/pressure.cpp
# 	mkdir -p bin
# 	g++ $^ -o bin/pressure ${OPTS}

# accel: ${SRCS} tools/accl_test.cpp
# 	mkdir -p bin
# 	g++ $^ -o bin/accel ${OPTS}

# setmotors: ${SRCS} tools/set_motors.cpp
# 	mkdir -p bin
# 	g++ $^ -o bin/motors ${OPTS}

# servo: ${SRCS} tools/servo.cpp
# 	mkdir -p bin
# 	g++ $^ -o bin/servo ${OPTS}

# logger: ${SRCS} tools/logger.cpp
# 	mkdir -p bin
# 	g++ $^ -o bin/logger ${OPTS}

# drone_spd_ctl: ${SRCS} tools/drone_spd_ctl.cpp
# 	mkdir -p bin
# 	g++ $^ -o bin/spd ${OPTS}

# gamepad:  ${SRCS} tools/gamepad_test.cpp
# 	mkdir -p bin
# 	g++ $^ -o bin/game ${OPTS} -DGAMEPAD_DEBUG=1

# init: ${SRCS} tools/init_test.cpp
# 	mkdir -p bin
# 	g++ $^ -o bin/init ${OPTS}

# led_test: ${SRCS} tools/test_led.cpp
# 	mkdir -p bin
# 	g++ $^ -o bin/led_test ${OPTS}

# led: ${SRCS} tools/led_ctl.cpp
# 	mkdir -p bin
# 	g++ $^ -o bin/led ${OPTS}

# read:
# 	mkdir -p bin
# 	g++ $^ -o bin/read ${OPTS}

# ws:
# 	mkdir -p bin
# 	g++ $^ -o bin/wsserver ${OPTS}
	
# motor_ctl:
# 	mkdir -p bin
# 	g++ $^ -o bin/motor ${OPTS}

# parse:
# 	mkdir -p bin
# 	g++ $^ -o bin/parse ${OPTS}

# drone_server:
# 	mkdir -p bin
# 	g++ $^ -o bin/drone_server ${OPTS}