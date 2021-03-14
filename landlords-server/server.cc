#include "protobuf/codec.h"
#include "protobuf/dispatcher.h"
#include "protobuf/query.pb.h"
#include "entity/Poker.h"
#include "helper/PokerHelper.h"
#include "enums/ClientEventCode.h"
#include "helper/SerializeHelper.h"
#include "robot/RobotDecisionMakers.h"
#include "robot/RobotEventListener.h"

#include "muduo/base/Logging.h"
#include "muduo/base/Mutex.h"
#include "muduo/net/EventLoop.h"
#include "muduo/net/TcpServer.h"

#include "event/ServerEventListener.h"

#include "event/ServerContains.h"

#include <stdio.h>
#include <unistd.h>

using namespace muduo;
using namespace muduo::net;

typedef std::shared_ptr<muduo::Query> QueryPtr;
typedef std::shared_ptr<muduo::Answer> AnswerPtr;

class QueryServer : noncopyable
{
 public:
  QueryServer(EventLoop* loop,
              const InetAddress& listenAddr)
  : server_(loop, listenAddr, "QueryServer"),
    dispatcher_(std::bind(&QueryServer::onUnknownMessage, this, _1, _2, _3)),
    codec_(std::bind(&ProtobufDispatcher::onProtobufMessage, &dispatcher_, _1, _2, _3))
  {
	init();
    dispatcher_.registerMessageCallback<muduo::Query>(
        std::bind(&QueryServer::onQuery, this, _1, _2, _3));
    dispatcher_.registerMessageCallback<muduo::Answer>(
        std::bind(&QueryServer::onAnswer, this, _1, _2, _3));
    server_.setConnectionCallback(
        std::bind(&QueryServer::onConnection, this, _1));
    server_.setMessageCallback(
        std::bind(&ProtobufCodec::onMessage, &codec_, _1, _2, _3));
  }

  void start()
  {
    server_.start();
  }

 private:
  void init()
  {
  	Poker poker(PokerType::HEART, PokerLevel::LEVEL_K);
  	Poker poker1(PokerType::DIAMOND, PokerLevel::LEVEL_K);
  	Poker poker2(PokerType::BLANK, PokerLevel::LEVEL_K);
  	Poker poker3(PokerType::CLUB, PokerLevel::LEVEL_K);
  	Poker poker4(PokerType::HEART, PokerLevel::LEVEL_SMALL_KING);
  	pokers_.push_back(poker);
  	pokers_.push_back(poker1);
  	pokers_.push_back(poker2);
  	pokers_.push_back(poker3);
  	pokers_.push_back(poker4);
  }


  void onConnection(const TcpConnectionPtr& conn)
  {
    LOG_INFO << conn->peerAddress().toIpPort() << " -> "
        << conn->localAddress().toIpPort() << " is "
        << (conn->connected() ? "UP" : "DOWN");

    ClientSide *clientSide(new ClientSide(21, ClientStatus::TO_CHOOSE));
    clientSide->setRole(ClientRole::PLAYER);

    ServerContains::CLIENT_SIDE_MAP.insert(std::make_pair(clientSide->getId(), clientSide));

//    pushToClient(conn, ClientEventCode::CODE_CLIENT_CONNECT, std::to_string(clientSide.getId()));
    pushToClient(conn, ClientEventCode::CODE_CLIENT_NICKNAME_SET, MapHelper());
  }

  void pushToClient(const TcpConnectionPtr& conn, ClientEventCode code, const MapHelper &data)
  {
	  Answer answer;
	  std::string result = SerializeHelper::SerializeToString<MapHelper>(data);
	  answer.set_answerer("san");
	  answer.set_questioner("san");
	  answer.set_id(int(code));
	  answer.add_solution(result);
	  codec_.send(conn, answer);
  }

  void onUnknownMessage(const TcpConnectionPtr& conn,
                        const MessagePtr& message,
                        Timestamp)
  {
    LOG_INFO << "onUnknownMessage: " << message->GetTypeName();
    conn->shutdown();
  }

  void onQuery(const muduo::net::TcpConnectionPtr& conn,
               const QueryPtr& message,
               muduo::Timestamp)
  {
	  // FIXME 在这里不需要解析
	    LOG_INFO << "onQuery:\n" << message->GetTypeName() << message->DebugString();
	    LOG_INFO << "onQuery:\n";
		// FIXME： 我也不知道为什么。。。
	    MapHelper result = SerializeHelper::parseStringToData<MapHelper>(message->question(0));
	    serverEventListener(conn, ServerEventCode(message->id()), result);
  }

  void onAnswer(const muduo::net::TcpConnectionPtr& conn,
                const AnswerPtr& message,
                muduo::Timestamp)
  {
    LOG_INFO << "onAnswer: " << message->GetTypeName();
    conn->shutdown();
  }

 private:
  TcpServer server_;
  ProtobufDispatcher dispatcher_;
  ProtobufCodec codec_;
  std::vector<Poker> pokers_;
  ServerEventListener serverEventListener;
};



int main(int argc, char* argv[])
{

	muduo::Logger::setLogLevel(muduo::Logger::DEBUG);

	PokerHelper ph;
	PokerHelper::init();

	// FIXME: 不知道为啥，总之莫名其妙
//	SerializeHelper sl;
	  /*
   * 静态对象总是会莫名其妙
   */

	RobotDecisionMakers rbd;
	RobotDecisionMakers::init();

	RobotEventListener rl;
	RobotEventListener::init();


	ServerContains sc;


  LOG_INFO << "pid = " << getpid();
  if (argc > 1)
  {
    EventLoop loop;
    uint16_t port = static_cast<uint16_t>(atoi(argv[1]));
    InetAddress serverAddr(port);
    QueryServer server(&loop, serverAddr);
    server.start();
    loop.loop();
  }
  else
  {
    printf("Usage: %s port\n", argv[0]);
  }
}

