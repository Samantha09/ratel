/*
 * SerializeHelper.h
 *
 *  Created on: 2021年2月22日
 *      Author: san
 */

#ifndef LANDLORDS_COMMON_HELPER_SERIALIZEHELPER_H_
#define LANDLORDS_COMMON_HELPER_SERIALIZEHELPER_H_

#include "boost/serialization/serialization.hpp"
#include "boost/archive/binary_oarchive.hpp"
#include "boost/archive/binary_iarchive.hpp"
#include <boost/serialization/export.hpp>
#include "boost/foreach.hpp"
#include "boost/any.hpp"
#include <boost/serialization/vector.hpp>
#include "boost/serialization/string.hpp"


#include "enums/ServerEventCode.h"
#include "entity/ClientSide.h"
#include "enums/SellType.h"


#include <string>
#include <iostream>
#include <sstream>
#include <vector>


class SerializeHelper {
public:
	template <class T>
	static std::string SerializeToString(T t)
	{
		std::ostringstream os;
		boost::archive::binary_oarchive oa(os);
		oa << t;
		return os.str();
	}

	template <class T>
	static T parseStringToData(const std::string &content)
	{
		T temp;
		std::istringstream is(content);
		boost::archive::binary_iarchive ia(is);
		ia >> temp;

		return temp;
	}
	SerializeHelper(){}

};


class ClientTransferData {
public:
	ClientTransferData() : id(0), code(ServerEventCode(0)), data(""), levels(std::vector<PokerLevel>()) {}
	ClientTransferData(int i, ServerEventCode c, const std::string &s, const std::vector<PokerLevel> &l)
		: id(i), code(c), data(s), levels(l)
	{}
	ClientTransferData(int i, ServerEventCode c, const std::string &s)
		: id(i), code(c), data(s), levels(std::vector<PokerLevel>())
	{}
	ClientTransferData(const ClientTransferData &rhs)
	{
		data = rhs.data;
		code = rhs.code;
		id = rhs.id;
		levels = rhs.levels;
	}

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
		ar & id;
		ar & code;
		ar & data;
		ar & levels;
    }

public:
    int id;
	ServerEventCode code;
	std::string data;
	std::vector<PokerLevel> levels;
};

class ServerTransferData {
public:

	void init()
	{
		roomOwner = "";
		roomClientCount = 0;
		preClientId = 0;
		preClientNickname = "";
		nextClientNickname = "";
		nextClientId = 0;
		roomClientList = std::vector<ClientSide>();
		pokers = std::vector<Poker>();

		landlordNickname = "";
		landlordId = 0;
		additionalPokers = std::vector<Poker>();
	}

	void setLandlordNickname(const std::string &s)
	{
		landlordNickname = s;
	}

	void setLandlordId(int id)
	{
		landlordId = id;
	}

	void setAdditionalPokers(const std::vector<Poker>& p)
	{
		additionalPokers = p;
	}

	void setPokers(const std::vector<Poker>& p)
	{
		pokers = p;
	}

	void setRoomId(int room)
	{
		roomId = room;
	}

	void setRoomOwner(const std::string & s)
	{
		roomOwner = s;
	}

	void setRoomClientCount(int count)
	{
		roomClientCount = count;
	}

	void setnextClientNickname(const std::string &nickname)
	{
		nextClientNickname = nickname;
	}

	void setNextClientId(int id)
	{
		nextClientId = id;
	}

	void setClientOrderList(const std::vector<ClientSide> &list)
	{
		roomClientList = list;
	}

	ServerTransferData()
		: roomId(0)
	{
		init();
	}

	ServerTransferData(int rId)
		: roomId(rId)
	{
		init();
	}

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
    	ar & roomId;
    	ar & roomOwner;
    	ar & roomClientCount;
    	ar & preClientNickname;
    	ar & preClientId;
    	ar & nextClientNickname;
    	ar & nextClientId;
    	ar & roomClientList;
		ar & pokers;
		ar & landlordNickname;
		ar & landlordId;
		ar & additionalPokers;
    }

public:
    int roomId;
    std::string roomOwner;
    int roomClientCount;
    std::string preClientNickname;
    int preClientId;
    std::string nextClientNickname;
    int nextClientId;
    std::vector<ClientSide> roomClientList;
	std::vector<Poker> pokers;

	std::string landlordNickname;
	int landlordId;
	std::vector<Poker> additionalPokers;

};

class ClientInfo {

public:

	ClientInfo(): ClientInfo(0, "nihao", ClientType::PEASANT, 3, "up") {}

	ClientInfo(int id, const std::string &name,
			ClientType t, int size, const std::string &p)
		:clientId(id), clientNickname(name), type(t), surplus(size), position(p)
	{
	}

	~ClientInfo(){}


    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
    	ar & clientId;
    	ar & clientNickname;
    	ar & type;
    	ar & surplus;
    	ar & position;
    }

	int clientId;
	std::string clientNickname;
	ClientType type;
	int surplus;
	std::string position;
};

class ServerGamePlayData
{
public:
	ServerGamePlayData()
	{
		init();
	}

	ServerGamePlayData(const std::vector<ClientInfo> &ifo,
			     const std::vector<Poker> &p,
				 const std::vector<Poker> &lsp,
				 int lsi,
				 int sci,
				 const std::string &name)
		:infos(ifo), pokers(p), lastSellPokers(lsp), lastSellClientId(lsi),
		 sellClientId(sci), sellClinetNickname(name)
	{

	}

	~ServerGamePlayData()
	{
	}

	void init()
	{
		infos = std::vector<ClientInfo>();
		pokers = std::vector<Poker>();
		lastSellPokers = std::vector<Poker>();
		lastSellClientId = 0;
		sellClientId = 0;
		sellClinetNickname = "nihao";
	}

	void setInfos(const std::vector<ClientInfo> &ifo)
	{
		infos = ifo;
	}

	void setPokers(const std::vector<Poker> &p)
	{
		pokers = p;
	}

	void setLastSellPokers(const std::vector<Poker> &p)
	{
		lastSellPokers = p;
	}

	void setLastSellClientId(int id)
	{
		lastSellClientId = id;
	}

	void setSellClientId(int id)
	{
		sellClientId = id;
	}

	void setSellClinetNickname(const std::string &name)
	{
		sellClinetNickname = name;
	}

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
    	ar & infos;
    	ar & pokers;
    	ar & lastSellPokers;
    	ar & lastSellClientId;
    	ar & sellClientId;
    	ar & sellClinetNickname;
    }

public:
    std::vector<ClientInfo> infos;
	std::vector<Poker> pokers;
	std::vector<Poker> lastSellPokers;
	int lastSellClientId;
	int sellClientId;
	std::string sellClinetNickname;
};


class CodeShowPokersData
{
public:
	CodeShowPokersData()
		: clientId(0), clientNickname(""),
		clientType(ClientType::PEASANT),
		pokers(std::vector<Poker>()),
		lastSellClientId(0),
		lastSellPokers(std::vector<Poker>()),
		sellClinetNickname("")
	{}

	void init()
	{
		clientId = 0;
		clientNickname = "";
		clientType = ClientType::PEASANT;
		pokers = std::vector<Poker>();
		lastSellClientId = 0;
		lastSellPokers = std::vector<Poker>();
		sellClinetNickname = "";
	}

	void setClientId(int id)
	{
		clientId = id;
	}

	void setClientNickname(const std::string& name)
	{
		clientNickname = name;
	}

	void setClientType(ClientType type)
	{
		clientType = type;
	}

	void setPokers(const std::vector<Poker> p)
	{
		pokers = p;
	}

	void setlastSellClientId(int id)
	{
		lastSellClientId = id;
	}

	void setLastSellPokers(const std::vector<Poker> p)
	{
		lastSellPokers = p;
	}

	void setSellClinetNickname(const std::string &name)
	{
		sellClinetNickname = name;
	}

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
    	ar & clientId;
    	ar & clientNickname;
    	ar & clientType;
    	ar & pokers;
    	ar & lastSellClientId;
    	ar & lastSellPokers;
    	ar & sellClinetNickname;
    }

	int clientId;
	std::string clientNickname;
	ClientType clientType;
	std::vector<Poker> pokers;
	int lastSellClientId;
	std::vector<Poker> lastSellPokers;

	std::string sellClinetNickname;
};


class ClientGamePlayData {
public:
	ClientGamePlayData()
		: playType(SellType::ILLEGAL), playCount(0),
		  preType(SellType::ILLEGAL), preCount(0),
		  playScore(0), preScore(0), winnerNickname(""),
		  winnerType(ClientType::PEASANT)
	{
	}

	void setPlayType(SellType pt)
	{
		playType = pt;
	}

	void setPlayCount(int count)
	{
		playCount = count;
	}

	void setPreType(SellType pt)
	{
		preType = pt;
	}

	void setPreCount(int count)
	{
		preCount = count;
	}

	void setPlayScore(int score)
	{
		playScore = score;
	}

	void setPreScore(int score)
	{
		preScore = score;
	}

	void setWinnerNickname(const std::string &name)
	{
		winnerNickname = name;
	}

	void setWinnerType(ClientType t)
	{
		winnerType = t;
	}

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
    	ar & playType;
    	ar & playCount;
    	ar & preType;
    	ar & preCount;
    	ar & playScore;
    	ar & preScore;
    	ar & winnerNickname;
    	ar & winnerType;
    }

	SellType playType;
	int playCount;
	SellType preType;
	int preCount;
	int playScore;
	int preScore;

	std::string winnerNickname;
	ClientType winnerType;
};




#endif /* LANDLORDS_COMMON_HELPER_SERIALIZEHELPER_H_ */
