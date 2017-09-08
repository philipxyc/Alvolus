include ../BuildSystem/CommonDefs.mak

BIN_DIR = ../Bin

INC_DIRS = \
	../../Include \
	/usr/include/python2.7

SRC_FILES = xtion.cpp

USED_LIBS += OpenNI2
USED_LIBS += opencv_core
USED_LIBS += opencv_imgproc
USED_LIBS += opencv_highgui
USED_LIBS += pthread
USED_LIBS += python2.7

EXE_NAME = Lorem

FLAGS += -Wall


include ../BuildSystem/CommonCppMakefile
