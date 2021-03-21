LIB_SRC =                                               \
        landlords-server/event/EventFuncs.cc            \
	landlords-server/event/ServerEventListener.cc   \
	landlords-server/event/ServerContains.cc        \
	landlords-server/robot/RobotEventListener.cc    \
	landlords-server/robot/RobotEventFuncs.cc       \
	landlords-client/event/eventFuncs.cc            \

BINARIES = server client

all: $(BINARIES)

BASE_DIR = $(shell pwd)
MUDUO_LIB := $(BASE_DIR)
MUDUO_LIB := $(shell pwd)/lib/muduo/lib

CXXFLAGS = -g -I lib/muduo/include -I landlords-common  -pthread -L$(MUDUO_LIB)
LDFLAGS = -L.  -lravel  -lboost_serialization -lprotobuf -lz -lmuduo_base -lpthread -lmuduo_net  -lz
landlord_root = landlords-common
BASE_SRC = $(landlord_root)/protobuf/codec.cc $(landlord_root)/protobuf/query.pb.cc $(landlord_root)/helper/PokerHelper.cc $(landlord_root)/entity/PokerSell.cc $(landlord_root)/robot/RobotDecisionMakers.cc
MUDUO_SRC = $(notdir $(LIB_SRC) $(BASE_SRC))
OBJS = $(patsubst %.cc,%.o,$(MUDUO_SRC))

libravel.a: $(BASE_SRC) $(LIB_SRC)
	g++ $(CXXFLAGS) -c $^
	ar rcs $@ $(OBJS)

$(BINARIES): libravel.a
	g++ $(CXXFLAGS) -o $@ $(filter %.cc,$^) $(LDFLAGS)

clean:
	rm -f $(BINARIES) *.o *.a core

server: landlords-server/server.cc
client: landlords-client/client.cc
