/*
 * ClientStatus.h
 *
 *  Created on: 2021年2月25日
 *      Author: san
 */

#ifndef LANDLORDS_COMMON_ENUMS_CLIENTSTATUS_H_
#define LANDLORDS_COMMON_ENUMS_CLIENTSTATUS_H_

enum class ClientStatus{
	TO_CHOOSE,         // 待选择
	NO_READY,
	READY,
	WAIT,
	CALL_LANDLORD,     // 叫地主
	PLAYING
};


#endif /* LANDLORDS_COMMON_ENUMS_CLIENTSTATUS_H_ */
