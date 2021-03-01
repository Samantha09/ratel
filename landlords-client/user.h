/*
 * user.h
 *
 *  Created on: 2021年2月7日
 *      Author: san
 */

#ifndef LANDLORDS_CLIENT_USER_H_
#define LANDLORDS_CLIENT_USER_H_

#include "muduo/base/Singleton.h"

class User {
public:
	bool isPlaying() { return isPlaying_; }
	void setPlaying(bool playing) { isPlaying_ = playing; }

	bool isWatching() { return isWatching_; }
	void setWatching(bool watching) { isWatching_ = watching; }

private:
	User() {}

private:
	// 是否游戏中
	bool isPlaying_ = false;
	// 是否观战中
	bool isWatching_ = false;
};
#endif /* LANDLORDS_CLIENT_USER_H_ */
