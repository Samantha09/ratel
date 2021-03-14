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
	ClientSide(int id, ClientStatus status)
		: id_(id), status_(int(status))
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
//		pre_ = new ClientSide(*cs.pre_);
//		next_ = new ClientSide(*cs.next_);
		pre_ = cs.pre_;
		next_ = cs.next_;
	}
	~ClientSide(){
//		delete pre_;
//		delete next_;
		// FIXME 内存泄露
	};

	template<class Archive>
	void tosave(Archive & ar)
	{
		// serialize base class information
		ar << BOOST_SERIALIZATION_NVP(id_);
		ar << BOOST_SERIALIZATION_NVP(nickname_);
		ar << BOOST_SERIALIZATION_NVP(pokers_);
		ar << BOOST_SERIALIZATION_NVP(role_);
		ar << BOOST_SERIALIZATION_NVP(roomId_);
		ar << BOOST_SERIALIZATION_NVP(status_);
		ar << BOOST_SERIALIZATION_NVP(type_);

		bool is_null;
		if (pre_ != NULL)
		{
			is_null = false;
			ar << boost::serialization::make_nvp("pre_", is_null);
			pre_->tosave(ar);
		}
		else
		{
			is_null = true;
			ar << boost::serialization::make_nvp("pre_", is_null);
		}

		if (next_ != 0)
		{
			is_null = false;
			ar << boost::serialization::make_nvp("next_", is_null);
			next_->tosave(ar);
		}
		else
		{
			is_null = true;
			ar << boost::serialization::make_nvp("next_", is_null);
		}
	}  // namespace serialization

	template<class Archive>
	void save(Archive& ar, const unsigned int version) const
	{
		ar << BOOST_SERIALIZATION_NVP(id_);
		ar << BOOST_SERIALIZATION_NVP(nickname_);
		ar << BOOST_SERIALIZATION_NVP(pokers_);
		ar << BOOST_SERIALIZATION_NVP(role_);
		ar << BOOST_SERIALIZATION_NVP(roomId_);
		ar << BOOST_SERIALIZATION_NVP(status_);
		ar << BOOST_SERIALIZATION_NVP(type_);

		bool is_null;
		if (pre_ != NULL)
		{
			is_null = false;
			ar << boost::serialization::make_nvp("pre_", is_null);
			pre_->tosave(ar);
		}
		else
		{
			is_null = true;
			ar << boost::serialization::make_nvp("pre_", is_null);
		}

		if (next_ != 0)
		{
			is_null = false;
			ar << boost::serialization::make_nvp("next_", is_null);
			next_->tosave(ar);
		}
		else
		{
			is_null = true;
			ar << boost::serialization::make_nvp("next_", is_null);
		}
	}


	template<class Archive>
	void load(Archive& ar,  const unsigned int version)
	{
		ClientSide *p = new ClientSide;
		ar >> BOOST_SERIALIZATION_NVP(p->id_);
		ar >> BOOST_SERIALIZATION_NVP(p->nickname_);
		ar >> BOOST_SERIALIZATION_NVP(p->pokers_);
		ar >> BOOST_SERIALIZATION_NVP(p->role_);
		ar >> BOOST_SERIALIZATION_NVP(p->roomId_);
		ar >> BOOST_SERIALIZATION_NVP(p->status_);
		ar >> BOOST_SERIALIZATION_NVP(p->type_);

		bool is_null;
		ar >> boost::serialization::make_nvp("pre_isnull", is_null);
		if (is_null)
		{
			p->pre_ = NULL;
		}
		else
		{
			p->pre_ = new ClientSide;
			Toload(ar, p->pre_);
		}

		ar >> boost::serialization::make_nvp("next_isnull", is_null);
		if (is_null)
		{
			p->next_ = NULL;
		}
		else
		{
			p->next_ = new ClientSide;
			Toload(ar, p->next_);
		}

		*this = *p;
		delete p;
		p = NULL;
	}

	template<class Archive>
	void Toload(Archive& ar,  ClientSide *p)
	{
		ar >> BOOST_SERIALIZATION_NVP(p->id_);
		ar >> BOOST_SERIALIZATION_NVP(p->nickname_);
		ar >> BOOST_SERIALIZATION_NVP(p->pokers_);
		ar >> BOOST_SERIALIZATION_NVP(p->role_);
		ar >> BOOST_SERIALIZATION_NVP(p->roomId_);
		ar >> BOOST_SERIALIZATION_NVP(p->status_);
		ar >> BOOST_SERIALIZATION_NVP(p->type_);

		bool is_null;
		ar >> boost::serialization::make_nvp("pre_isnull", is_null);
		if (is_null)
		{
			p->pre_ = NULL;
		}
		else
		{
			p->pre_ = new ClientSide;
			Toload(ar, p->pre_);
		}

		ar >> boost::serialization::make_nvp("next_isnull", is_null);
		if (is_null)
		{
			p->next_ = NULL;
		}
		else
		{
			p->next_ = new ClientSide;
			Toload(ar, p->next_);
		}
	}

	// getter and setter
	ClientRole getRole() { return ClientRole(role_); }
	std::string getNickname() { return nickname_; }
	int getRoomId() { return roomId_; }
	std::vector<Poker> &getPokers() { return pokers_; }
	ClientStatus getStatus() { return ClientStatus(status_); }
	ClientType getType() { return ClientType(type_); }
	int getId() { return id_; }
	ClientSide &getNext() { return *next_; }
	ClientSide &getPre() { return *pre_; }

	void setRole(ClientRole role) { role_ = int(role); }
	void setNickname(std::string nickname) { nickname_ = nickname; }
	void setRoomId(int roomId) { roomId_ = roomId; }
	void setPokers(std::vector<Poker> pokers) { pokers_ = pokers; }
	void setStatus(ClientStatus status) { status_ = int(status); }
	void setType(ClientType type) { type_ = int(type); }
	void setId(int id) { id_ = id; }
	void setNext(ClientSide *next) { next_ = next; }
	void setPre(ClientSide *pre) { pre_ = pre; }

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
	BOOST_SERIALIZATION_SPLIT_MEMBER();

public:
	int id_;
	int roomId_;
	std::string nickname_;
	std::vector<Poker> pokers_;
	int status_;
	int role_;              // player or robot
	int type_;              // 地主or农民
	ClientSide *pre_;
	ClientSide *next_;

private:
	friend class boost::serialization::access;
};



#endif /* LANDLORDS_COMMON_ENTITY_CLIENTSIDE_H_ */
