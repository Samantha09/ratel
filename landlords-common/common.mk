CXXFLAGS = -g -I /home/san/eclipse-workspace/ratel/landlords-common  -pthread -L/home/san/apps/build/debug-install-cpp11/lib
LDFLAGS = -L. -lravel  -lboost_serialization -lprotobuf -lz -lmuduo_base -lpthread -lmuduo_net -L/home/san/apps/build/debug-install-cpp11/lib -lz
landlord_root = /home/san/eclipse-workspace/ratel/landlords-common
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
