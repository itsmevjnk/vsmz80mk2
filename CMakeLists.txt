cmake_minimum_required(VERSION 3.7)

# set up MinGW
if(UNIX)
set(CMAKE_SYSTEM_NAME Windows)
set(TOOLCHAIN_PREFIX i686-w64-mingw32) # use i686 for 32-bit binaries
set(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}-gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}-g++)
set(CMAKE_RC_COMPILER ${TOOLCHAIN_PREFIX}-windres)
set(CMAKE_FIND_ROOT_PATH /usr/${TOOLCHAIN_PREFIX})
add_link_options(-static -static-libgcc -static-libstdc++) # static link everything
endif(UNIX)

project(
	vsmz80mk2
	DESCRIPTION "Z80 CPU simulation model for Labcenter Proteus"
	LANGUAGES CXX
)

add_subdirectory(llz80emu)

add_library(
	vsmz80mk2 SHARED
	vsmz80mk2/vsmz80mk2.cpp vsmz80mk2/ActiveModel.cpp vsmz80mk2/DsimModel.cpp 
	vsmz80mk2/ActiveModel.h vsmz80mk2/DsimModel.h
	vsmz80mk2/sdk/vdm.hpp vsmz80mk2/sdk/vdm11.hpp vsmz80mk2/sdk/vdm51.hpp vsmz80mk2/sdk/vdmpic.hpp vsmz80mk2/sdk/vsm.hpp
)
set_target_properties(vsmz80mk2 PROPERTIES PREFIX "")
target_link_libraries(vsmz80mk2 PRIVATE llz80emu_static)