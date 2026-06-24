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
	rm -f $(BINARIES) *.o *.a core gateway

server: landlords-server/server.cc
client: landlords-client/client.cc

# Gateway target (WebSocket + JSON, no protobuf)
GATEWAY_SRC = landlords-server/server.cc \
              landlords-server/event/EventFuncs.cc \
              landlords-server/event/ServerEventListener.cc \
              landlords-server/event/ServerContains.cc \
              landlords-server/robot/RobotEventListener.cc \
              landlords-server/robot/RobotEventFuncs.cc \
              landlords-common/helper/PokerHelper.cc \
              landlords-common/entity/PokerSell.cc \
              landlords-common/robot/RobotDecisionMakers.cc

GATEWAY_CXXFLAGS = -g -std=c++14 -I lib/muduo/include -I landlords-common -I lib/json -pthread -L lib/muduo/lib
GATEWAY_LDFLAGS = -L lib/muduo/lib -lmuduo_net -lmuduo_base -lpthread -lz

gateway: $(GATEWAY_SRC)
	g++ $(GATEWAY_CXXFLAGS) -o gateway $(GATEWAY_SRC) $(GATEWAY_LDFLAGS)

# Self-test targets
test: sha1_base64_test websocket_test jsonmap_test

sha1_base64_test: ; g++ $(GATEWAY_CXXFLAGS) landlords-common/web/sha1_base64_test.cc -o /tmp/$@ && /tmp/$@
websocket_test:   ; g++ $(GATEWAY_CXXFLAGS) landlords-common/web/websocket_test.cc   -o /tmp/$@ && /tmp/$@
jsonmap_test:     ; g++ $(GATEWAY_CXXFLAGS) landlords-common/web/jsonmap_test.cc       -o /tmp/$@ && /tmp/$@

.PHONY: all clean test gateway sha1_base64_test websocket_test jsonmap_test
