#include "web/WsCodec.h"
#include "web/JsonMapHelper.h"
#include "json.hpp"
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

class QueryServer : noncopyable
{
 public:
  QueryServer(EventLoop* loop,
              const InetAddress& listenAddr)
  : server_(loop, listenAddr, "QueryServer"),
    codec_(std::bind(&QueryServer::onWsMessage, this, _1, _2),
           std::bind(&QueryServer::onHandshake, this, _1)),
    serverEventListener()
  {
	server_.setConnectionCallback(std::bind(&QueryServer::onConnection, this, _1));
	server_.setMessageCallback(std::bind(&WsCodec::onMessage, &codec_, _1, _2, _3));
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

    if (conn->connected())
    {
        // Don't send anything here - wait for handshake to complete.
        // ClientSide will be created in onHandshake callback.
    }
    else
    {
        codec_.discard(conn);
    }
  }

  void onHandshake(const TcpConnectionPtr& conn)
  {
      // Handshake complete, create ClientSide and send idSet
      ClientSide *clientSide(new ClientSide(ServerContains::getClientId(), ClientStatus::TO_CHOOSE, conn));
      clientSide->setRole(ClientRole::PLAYER);
      clientSide->setConn(conn);

      ServerContains::CLIENT_SIDE_MAP.insert(std::make_pair(clientSide->getId(), clientSide));

      LOG_INFO << "Created ClientSide with ID: " << clientSide->getId() << " for connection";

      pushToClient(conn, ClientEventCode::CODE_GAME_ID_SET, MapHelper().put("clientId", clientSide->getId()));
  }

  void pushToClient(const TcpConnectionPtr& conn, ClientEventCode code, const MapHelper &data)
  {
	  codec_.sendEvent(conn, code, data);
  }

  void onWsMessage(const TcpConnectionPtr& conn, const std::string& text)
  {
      nlohmann::json j;
      try {
          j = nlohmann::json::parse(text);
      } catch (...) {
          LOG_WARN << "bad json frame";
          return;
      }
      ServerEventCode code = event_name_to_server_code(j.value("event", ""));
      MapHelper data = from_event_json(j);

      // Find the ClientSide for this connection and add clientId to data
      bool found = false;
      LOG_DEBUG << "CLIENT_SIDE_MAP size: " << ServerContains::CLIENT_SIDE_MAP.size();
      for (const auto& entry : ServerContains::CLIENT_SIDE_MAP) {
          LOG_DEBUG << "Checking ClientSide ID: " << entry.second->getId();
          if (entry.second->getConn() == conn) {
              data.put("clientId", entry.second->getId());
              LOG_INFO << "Found ClientSide for connection: " << entry.second->getId();
              found = true;
              break;
          }
      }

      if (!found) {
          LOG_WARN << "No ClientSide found for connection";
          return;
      }

      serverEventListener(conn, code, data);
  }

 private:
  TcpServer server_;
  WsCodec codec_;
  std::vector<Poker> pokers_;
  ServerEventListener serverEventListener;
};



int main(int argc, char* argv[])
{

	muduo::Logger::setLogLevel(muduo::Logger::DEBUG);

	PokerHelper ph;
	PokerHelper::init();
	RobotDecisionMakers rbd;
	RobotDecisionMakers::init();

	RobotEventListener rl;
	RobotEventListener::init();

	ServerContains sc;
  LOG_INFO << "pid = " << getpid();
  if (argc > 2)
  {
    EventLoop loop;
    uint16_t port = static_cast<uint16_t>(atoi(argv[2]));
    InetAddress serverAddr(argv[1], port);
    QueryServer server(&loop, serverAddr);
    server.start();
    loop.loop();
  }
  else
  {
    printf("Usage: %s ip port\n", argv[0]);
  }
}

