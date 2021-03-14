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
#include "boost/serialization/unordered_map.hpp"


#include "enums/ServerEventCode.h"
#include "entity/ClientSide.h"
#include "enums/SellType.h"


#include <string>
#include <iostream>
#include <sstream>
#include <vector>

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

class MapHelper {
public:
	MapHelper()
		: strMap(std::unordered_map<std::string, std::string>()),
		  intMap(std::unordered_map<std::string, int>()),
		  pokerVecMap(std::unordered_map<std::string, std::vector<Poker> >()),
		  levelVecMap(std::unordered_map<std::string, std::vector<PokerLevel> >()),
		  clientSideVecMap(std::unordered_map<std::string, std::vector<ClientSide> >()),
		  clientInfos(std::unordered_map<std::string, std::vector<ClientInfo> >())
	{}

	MapHelper &put(const std::string &k, const std::string &v)
	{
		strMap.insert(std::make_pair(k, v));
		return *this;
	}

	MapHelper &put(const std::string &k, int v)
	{
		intMap.insert(std::make_pair(k, v));
		return *this;
	}

	MapHelper &put(const std::string &k, const std::vector<Poker> &v)
	{
		pokerVecMap.insert(std::make_pair(k, v));
		return *this;
	}

	MapHelper &put(const std::string &k, const std::vector<PokerLevel> &v)
	{
		levelVecMap.insert(std::make_pair(k, v));
		return *this;
	}

	MapHelper &put(const std::string &k, const std::vector<ClientSide> &v)
	{
		clientSideVecMap.insert(std::make_pair(k, v));
		return *this;
	}

	MapHelper &put(const std::string &k, const std::vector<ClientInfo> &v)
	{
		clientInfos.insert(std::make_pair(k, v));
		return *this;
	}

	std::string get(const std::string &k, const std::string &) const
	{
		if (strMap.find(k) != strMap.end())
		{
			return strMap.find(k)->second;
		}
		return "";
	}

	std::vector<Poker> get(const std::string &k, const std::vector<Poker> &) const
	{
		if (pokerVecMap.find(k) != pokerVecMap.end())
		{
			return pokerVecMap.find(k)->second;
		}
		return std::vector<Poker>();
	}

	int get(const std::string &k, int) const
	{
		if (intMap.find(k) != intMap.end())
		{
			return intMap.find(k)->second;
		}
		return 0;
	}

	std::vector<PokerLevel> get(const std::string &k, std::vector<PokerLevel>) const
	{
		if (levelVecMap.find(k) != levelVecMap.end())
		{
			return levelVecMap.find(k)->second;
		}
		return std::vector<PokerLevel>();
	}

	std::vector<ClientSide> get(const std::string &k, std::vector<ClientSide>)
	{
		if (clientSideVecMap.find(k) != clientSideVecMap.end())
		{
			return clientSideVecMap.find(k)->second;
		}
		return std::vector<ClientSide>();
	}

	std::vector<ClientInfo> get(const std::string &k, std::vector<ClientInfo>) const
	{
		if (clientInfos.find(k) != clientInfos.end())
		{
			return clientInfos.find(k)->second;
		}
		return std::vector<ClientInfo>();
	}

private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
		ar & strMap;
		ar & pokerVecMap;
		ar & intMap;
		ar & levelVecMap;
		ar & clientSideVecMap;
		ar & clientInfos;
    }

	std::unordered_map<std::string, std::string> strMap;
	std::unordered_map<std::string, std::vector<Poker> > pokerVecMap;
	std::unordered_map<std::string, int> intMap;
	std::unordered_map<std::string, std::vector<PokerLevel> > levelVecMap;
	std::unordered_map<std::string, std::vector<ClientSide> > clientSideVecMap;
	std::unordered_map<std::string, std::vector<ClientInfo> > clientInfos;
};


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

#endif /* LANDLORDS_COMMON_HELPER_SERIALIZEHELPER_H_ */
