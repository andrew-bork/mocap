set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_VERSION Linux)
set(CMAKE_SYSTEM_PROCESSOR ARM)

# set(bruh C:/SysGCC/raspberry/bin/ )
set(bruh )
set(CMAKE_AR                ${bruh}arm-linux-gnueabihf-ar)
set(CMAKE_ASM_COMPILER      ${bruh}arm-linux-gnueabihf-as)
set(CMAKE_C_COMPILER        ${bruh}arm-linux-gnueabihf-gcc)
set(CMAKE_CXX_COMPILER      ${bruh}arm-linux-gnueabihf-g++)
set(CMAKE_LINKER            ${bruh}arm-linux-gnueabihf-ld)
set(CMAKE_OBJCOPY           ${bruh}arm-linux-gnueabihf-objcopy)
set(CMAKE_RANLIB            ${bruh}arm-linux-gnueabihf-ranlib)
set(CMAKE_SIZE              ${bruh}arm-linux-gnueabihf-size)
set(CMAKE_STRIP             ${bruh}arm-linux-gnueabihf-strip)


# set(triple arm-linux-gnueabihf)

# set(CMAKE_C_COMPILER clang)
# set(CMAKE_C_COMPILER_TARGET ${triple})
# set(CMAKE_CXX_COMPILER clang++)
# set(CMAKE_CXX_COMPILER_TARGET ${triple})

# set(CMAKE_C_FLAGS               <c_flags>)
# set(CMAKE_CXX_FLAGS             <cpp_flags>)
# set(CMAKE_C_FLAGS_DEBUG         <c_flags_for_debug>)
# set(CMAKE_C_FLAGS_RELEASE       <c_flags_for_release>)
# set(CMAKE_CXX_FLAGS_DEBUG       <cpp_flags_for_debug>)
# set(CMAKE_CXX_FLAGS_RELEASE     <cpp_flags_for_release>)
# set(CMAKE_EXE_LINKER_FLAGS      <linker_flags>)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM     NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY     ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE     ONLY)

# Optionally reduce compiler sanity check when cross-compiling.
set(CMAKE_TRY_COMPILE_TARGET_TYPE         STATIC_LIBRARY)