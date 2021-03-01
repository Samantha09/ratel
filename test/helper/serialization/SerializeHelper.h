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


#include "enums/ServerEventCode.h"
#include "entity/ClientSide.h"


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
	ClientTransferData() : id(0), code(ServerEventCode(0)), data("") {}
	ClientTransferData(int i, ServerEventCode c, const std::string &s)
		: id(i), code(c), data(s)
	{}
	ClientTransferData(const ClientTransferData &rhs)
	{
		data = rhs.data;
		code = rhs.code;
		id = rhs.id;
	}

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
		ar & id;
		ar & code;
		ar & data;
    }

public:
    int id;
	ServerEventCode code;
	std::string data;
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
		roomClientList = std::vector<ClientSide*>();
		pokers = std::vector<Poker>();
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
    	ar & nextClientNickname;
    	ar & nextClientId;
    	ar & roomClientList;
		ar & pokers;
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

};




#endif /* LANDLORDS_COMMON_HELPER_SERIALIZEHELPER_H_ */
