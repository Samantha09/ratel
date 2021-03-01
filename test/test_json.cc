// Serialization.cpp : Defines the entry point for the console application.
//


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


//测试序列化
class CMyData
{
private:
	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		ar & _tag;
		ar & _text;
	}


public:
	CMyData():_tag(0), _text(""){}

	CMyData(int tag, std::string text):_tag(tag), _text(text){}
	virtual ~CMyData(){}

	int GetTag() const {return _tag;}
	std::string GetText() const {return _text;}

private:
	int _tag;
	std::string _text;
};


void TestArchive1()
{
	std::string content;

	{
		CMyData d1(2012, "China, good luck");
		std::ostringstream os;
		boost::archive::binary_oarchive oa(os);
		oa << d1;//序列化到一个ostringstream里面

		content = os.str();//content保存了序列化后的数据。
	}


	{
		CMyData d2;
		std::istringstream is(content);
		boost::archive::binary_iarchive ia(is);
		ia >> d2;//从一个保存序列化数据的string里面反序列化，从而得到原来的对象。

		std::cout << "CMyData tag: " << d2.GetTag() << ", text: " << d2.GetText() << "\n";
	}

}


class CMyData2
{
public:
	CMyData2():_tag(0), _text(""){}

	CMyData2(int tag, std::string text):_tag(tag), _text(text){}

	int _tag;
	std::string _text;
};

namespace boost {
	namespace serialization {

		template<class Archive>
		void serialize(Archive & ar, CMyData2 & d, const unsigned int version)
		{
			ar & d._tag;
			ar & d._text;
		}

	} // namespace serialization
} // namespace boost

void TestArchive2()
{
	CMyData2 d1(2012, "China, good luck");
	std::ostringstream os;
	boost::archive::binary_oarchive oa(os);
	oa << d1;//序列化到一个ostringstream里面

	std::string content = os.str();//content保存了序列化后的数据。

	CMyData2 d2;
	std::istringstream is(content);
	boost::archive::binary_iarchive ia(is);
	ia >> d2;//从一个保存序列化数据的string里面反序列化，从而得到原来的对象。

	std::cout << "CMyData2 tag: " << d2._tag << ", text: " << d2._text << "\n";
}


//序列化子类,侵入式
class CMyData_Child: public CMyData
{
private:
	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		// serialize base class information
		ar & boost::serialization::base_object<CMyData>(*this);
		ar & _number;
	}


public:
	CMyData_Child():_number(0.0){}

	CMyData_Child(int tag, std::string text, float number):CMyData(tag, text), _number(number){}

	float GetNumber() const{return _number;}

private:
	float _number;
};

BOOST_CLASS_EXPORT_GUID(CMyData_Child, "CMyData_Child")


void TestArchive3()
{
	CMyData_Child d1(2012, "China, good luck", 1.2);
	std::ostringstream os;
	boost::archive::binary_oarchive oa(os);
	oa << d1;//序列化到一个ostringstream里面

	std::string content = os.str();//content保存了序列化后的数据。

	CMyData_Child d2;
	std::istringstream is(content);
	boost::archive::binary_iarchive ia(is);
	ia >> d2;//从一个保存序列化数据的string里面反序列化，从而得到原来的对象。

	std::cout << "CMyData_Child tag: " << d2.GetTag() << ", text: " << d2.GetText() << ", number: "<<d2.GetNumber() << "\n";
}

//序列化子类，非侵入式
class CMyData2_Child: public CMyData2
{
public:
	CMyData2_Child():_number(0.0){}

	CMyData2_Child(int tag, std::string text, float number):CMyData2(tag, text), _number(number){}

	float _number;
};

namespace boost {
	namespace serialization {

		template<class Archive>
		void serialize(Archive & ar, CMyData2_Child & d, const unsigned int version)
		{
			// serialize base class information
			ar & boost::serialization::base_object<CMyData2>(d);
			ar & d._number;
		}

	} // namespace serialization
} // namespace boost

void TestArchive4()
{
	CMyData2_Child d1(2012, "test non-intrusive child class", 5.6);
	std::ostringstream os;
	boost::archive::binary_oarchive oa(os);
	oa << d1;//序列化到一个ostringstream里面

	std::string content = os.str();//content保存了序列化后的数据。

	CMyData2_Child d2;
	std::istringstream is(content);
	boost::archive::binary_iarchive ia(is);
	ia >> d2;//从一个保存序列化数据的string里面反序列化，从而得到原来的对象。

	std::cout << "CMyData2_Child tag: " << d2._tag << ", text: " << d2._text << ", number: "<<d2._number<<"\n";
}


//指针数据成员

class CMyData_Container
{
private:
	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		ar & pointers;
	}
public:
	CMyData* pointers[3];
};



void TestPointerArchive()
{
	std::string content;
	{
		CMyData d1(1, "a");
		CMyData_Child d2(2, "b", 1.5);

		CMyData_Container containter;
		containter.pointers[0] = &d1;
		containter.pointers[1] = &d2;
		containter.pointers[2] = &d1;

		std::ostringstream os;
		boost::archive::binary_oarchive oa(os);
		oa << containter;

		content = os.str();
	}

	//反序列化
	{
		CMyData_Container container;
		std::istringstream is(content);
		boost::archive::binary_iarchive ia(is);
		ia >> container;

		for (int i = 0; i < 3; i++)
		{
			CMyData* d = container.pointers[i];
			std::cout << "pointer" << i + 1 <<": " << d->GetText() << "\n";

			if (i == 1)
			{
				CMyData_Child* child = reinterpret_cast<CMyData_Child*>(d);
				std::cout << "pointer" << i + 1 <<", number: " << child->GetNumber() << "\n";
			}
		}
	}
}

//使用STL容器
class CMyData_ContainerSTL
{
private:
	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		ar & vPointers;
	}
public:
	std::vector<CMyData*> vPointers;
};



void TestPointerArchiveWithSTLCollections()
{
	std::string content;
	{
		CMyData d1(1, "parent obj");
		CMyData_Child d2(2, "child obj", 2.5);

		CMyData_ContainerSTL containter;
		containter.vPointers.push_back(&d1);
		containter.vPointers.push_back(&d2);
		containter.vPointers.push_back(&d1);


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
		BOOST_FOREACH(CMyData* p, container.vPointers)
		{
			std::cout << "object text: " << p->GetText() << "\n";

			CMyData_Child* child = dynamic_cast<CMyData_Child*>(p);
			if (child)
			{
				std::cout << "child object number: " << child->GetNumber() << "\n";
			}
		}
	}
}

int main(int argc, char* argv[])
{
	TestArchive1();

	TestArchive2();

	TestArchive3();

	TestArchive4();

	TestPointerArchive();

	TestPointerArchiveWithSTLCollections();

	return 0;
}
