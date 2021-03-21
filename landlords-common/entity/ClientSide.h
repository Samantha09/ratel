/*
 * ClientSide.h
 *
 *  Created on: 2021年2月10日
 *      Author: san
 */

#ifndef LANDLORDS_COMMON_ENTITY_CLIENTSIDE_H_
#define LANDLORDS_COMMON_ENTITY_CLIENTSIDE_H_

#include "Poker.h"
#include "protobuf/codec.h"
#include "muduo/net/TcpClient.h"
#include "enums/ClientStatus.h"
#include "enums/ClientRole.h"
#include "enums/ClientType.h"
#include <memory>

#include <string>

class ClientSide {
public:
	// 构造函数和析构函数
	ClientSide(): id_(0), status_(0){ init(); };
	ClientSide(int id, ClientStatus status, const muduo::net::TcpConnectionPtr& conn)
		: id_(id), status_(int(status)), conn_(conn)
	{
		init();
		// TODO Auto-generated constructor stub
	}
	ClientSide(const ClientSide &cs)
	{
		id_ = cs.id_;
		roomId_ = cs.roomId_;
		nickname_ = cs.nickname_;
		pokers_ = cs.pokers_;
		status_ = cs.status_;
		role_ = cs.role_;
		type_ = cs.type_;
		pre_ = cs.pre_;
		next_ = cs.next_;
		pre_ = cs.pre_;
		next_ = cs.next_;
	}
	~ClientSide(){
		// FIXME 内存泄露
	};

	// getter and setter
	ClientRole getRole() { return ClientRole(role_); }
	std::string getNickname() { return nickname_; }
	int getRoomId() { return roomId_; }
	std::vector<Poker> &getPokers() { return pokers_; }
	ClientStatus getStatus() { return ClientStatus(status_); }
	ClientType getType() { return ClientType(type_); }
	int getId() { return id_; }
	std::shared_ptr<ClientSide> getNext() { return next_; }
	std::shared_ptr<ClientSide> getPre() { return pre_; }
	muduo::net::TcpConnectionPtr& getConn()
	{
		return conn_;
	}

	void setRole(ClientRole role) { role_ = int(role); }
	void setNickname(std::string nickname) { nickname_ = nickname; }
	void setRoomId(int roomId) { roomId_ = roomId; }
	void setPokers(std::vector<Poker> pokers) { pokers_ = pokers; }
	void setStatus(ClientStatus status) { status_ = int(status); }
	void setType(ClientType type) { type_ = int(type); }
	void setId(int id) { id_ = id; }
	void setNext(std::shared_ptr<ClientSide> next) { next_ = next; }
	void setPre(std::shared_ptr<ClientSide> pre) { pre_ = pre; }
	void setConn(const muduo::net::TcpConnectionPtr& conn)
	{
		conn_ = conn;
	}

	void init()
	{
		roomId_ = 0;
		nickname_ = "nihao";
		pokers_ = std::vector<Poker>();
		role_ = 0;
		type_ = 0;
		pre_ = NULL;
		next_ = NULL;
	};

public:
	int id_;
	int roomId_;
	std::string nickname_;
	std::vector<Poker> pokers_;
	int status_;
	int role_;              // player or robot
	int type_;              // 地主or农民
	std::shared_ptr<ClientSide> pre_;
	std::shared_ptr<ClientSide> next_;
	muduo::net::TcpConnectionPtr conn_;

private:
	friend class boost::serialization::access;
};



#endif /* LANDLORDS_COMMON_ENTITY_CLIENTSIDE_H_ */
