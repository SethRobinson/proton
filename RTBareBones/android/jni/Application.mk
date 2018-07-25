# The ARMv7 is significanly faster due to the use of the hardware FPU
APP_STL := gnustl_static
APP_CPPFLAGS = -fexceptions
#STLPORT_FORCE_REBUILD := true
APP_OPTIM=release
APP_ABI := armeabi


#use below instead if you want a dual binary, faster for new devices, but keep in mind the size cost
#APP_ABI := armeabi armeabi-v7a
