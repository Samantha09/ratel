LIB_SRC =                              \
        event/EventFuncs.cc            \
	event/ServerEventListener.cc   \
	event/ServerContains.cc        \
	robot/RobotEventListener.cc    \
	robot/RobotEventFuncs.cc       \

BINARIES = server

all: $(BINARIES)

include ../landlords-common/common.mk

server: server.cc
