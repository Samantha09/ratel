/*
 * PokerBasic.h
 *
 *  Created on: 2021年2月5日
 *      Author: san
 */

//#ifndef LANDLORDS_COMMON_POKERBASIC_H_
//#define LANDLORDS_COMMON_POKERBASIC_H_
//
//#include <vector>
//#include <string>
//
//struct PokerBasic {

//	enum class PokerType{
//		BLANK,
//		DIAMOND,
//		CLUB,
//		SPADE,
//		HEART
//	};

//	enum class PokerLevel{
//		LEVEL_3,
//		LEVEL_4,
//		LEVEL_5,
//		LEVEL_6,
//		LEVEL_7,
//		LEVEL_8,
//		LEVEL_9,
//		LEVEL_10,
//		LEVEL_J,
//		LEVEL_Q,
//		LEVEL_K,
//		LEVEL_A,
//		LEVEL_2,
//		LEVEL_SMALL_KING,
//		LEVEL_BIG_KING,
//	};

//	enum class ClientType {
//		LANDLORD,     // 地主
//		PEASANT       // 农民
//	};

//	enum class ClientStatus{
//	TO_CHOOSE,
//	NO_READY,
//	READY,
//	WAIT,
//	CALL_LANDLORD,
//	PLAYING
//	};

//	enum class ClientRole{
//		PLAYER,
//		ROBOT
//	};
//
//	static const std::vector<std::string> POKERTYPE;
//	static const std::vector<std::string> POKERLEVEL;
//	static const std::vector<std::string> SHELLTYPE;
//};

//typedef typename PokerBasic::PokerLevel PokerLevel;
//typedef typename PokerBasic::PokerType PokerType;
//typedef typename PokerBasic::ClientType ClientType;
//typedef typename PokerBasic::ClientStatus ClientStatus;
//typedef typename PokerBasic::ClientRole ClientRole;

//namespace boost {
//	namespace serialization {
//
//		template<class Archive>
//		void serialize(Archive & ar, ClientRole & d, const unsigned int version)
//		{
//			// serialize base class information
//			ar & d;
//		}
//	} // namespace serialization
//} // namespace boost


//#endif /* LANDLORDS_COMMON_POKERBASIC_H_ */
