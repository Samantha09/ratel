/*
 * test_serialize_pokers.cc
 *
 *  Created on: 2021年2月22日
 *      Author: san
 */

#include "boost/serialization/serialization.hpp"
#include "boost/archive/binary_oarchive.hpp"
#include "boost/archive/binary_iarchive.hpp"
#include <boost/serialization/export.hpp>
#include "boost/foreach.hpp"
#include "boost/any.hpp"
#include <boost/serialization/vector.hpp>



#include <string>
#include <iostream>
#include <sstream>
#include <vector>

class Poker {
public:
	Poker(int type, int level)
			: type_(type), level_(level)
	{
	}

	Poker() : Poker(0, 0){};
	~Poker(){};

public:
	int type_;
	int level_;
};

namespace boost {
	namespace serialization {

		template<class Archive>
		void serialize(Archive & ar, Poker & p, const unsigned int version)
		{
			// serialize base class information
			ar & p.type_;
			ar & p.level_;
		}

	} // namespace serialization
} // namespace boost


//使用STL容器
class CMyData_ContainerSTL
{
private:
	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		ar & pokers_;
	}


public:
	std::vector<Poker> pokers_;

	void push_back(const Poker & p)
	{
		pokers_.push_back(p);
	}

	std::vector<Poker> &getPokers()
	{
		return pokers_;
	}
};

void TestPointerArchiveWithSTLCollections()
{
	std::string content;
	{
		Poker p1(0,10);

		CMyData_ContainerSTL containter;
		containter.push_back(p1);

		std::ostringstream os;
		boost::archive::binary_oarchive oa(os);
		oa << containter;

		content = os.str();
	}

	//反序列化
	{
		CMyData_ContainerSTL container;
		std::istringstream is(content);
		boost::archive::binary_iarchive ia(is);
		ia >> container;

		std::cout<<"Test STL collections:\n";
		std::cout << container.pokers_[0].level_;
		std::cout << std::endl;
	}
}

void mytest()
{
	// 序列化
	std::string content;
	{
		std::vector<Poker> pokers_;
		pokers_.push_back(Poker(0,1));
		pokers_.push_back(Poker(0,2));

		std::ostringstream os;
		boost::archive::binary_oarchive oa(os);
		oa << pokers_;

		content = os.str();
	}

	// 反序列化
	{
		std::vector<Poker> pokers;
		std::istringstream is(content);
		boost::archive::binary_iarchive ia(is);
		ia >> pokers;
		std::cout << pokers[1].level_;
	}

}

int main()
{
//	TestPointerArchiveWithSTLCollections();
	mytest();
	return 0;
}
