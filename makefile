CC:=arm-linux-gnueabihf-gcc
CXX:=arm-linux-gnueabihf-g++
AR:=arm-linux-gnueabihf-ar

.PHONY: libs 

libs: i2c fmt

i2c: 3rdParty/lib/libi2c.a
3rdParty/lib/libi2c.a:
	make -C 3rdParty/i2c-tools/ CC=${CC} CXX=${CXX} AR=${AR}
	cp 3rdParty/i2c-tools/lib/libi2c.a 3rdParty/lib/libi2c.a

fmt: 3rdParty/lib/libfmt.a
3rdParty/lib/libfmt.a:
	cmake -DCMAKE_CXX_COMPILER=${CXX} -DCMAKE_C_COMPILER=%{CC} -S 3rdParty/lib/fmt -B 3rdParty/lib/fmt/build
	${MAKE} -C 3rdParty/lib/fmt/build CC=${CC} CXX=${CXX} AR=${AR}
	cp 3rdParty/lib/fmt/build/libfmt.a 3rdParty/lib/libfmt.a
