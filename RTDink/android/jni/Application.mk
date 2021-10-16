# The ARMv7 is significanly faster due to the use of the hardware FPU

APP_STL := gnustl_static
#APP_STL := stlport_static
APP_CPPFLAGS = -fexceptions
APP_CPPFLAGS += -Wno-error=format-security
APP_CPPFLAGS += -std=c++11
#APP_LDFLAGS = -latomic
#STLPORT_FORCE_REBUILD := true
APP_OPTIM:=release
#APP_OPTIM:=debug
APP_ABI := armeabi armeabi-v7a
