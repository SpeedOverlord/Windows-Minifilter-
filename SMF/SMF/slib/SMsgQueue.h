/*
	Copyright © 2016 by Phillip Chang

	SMsgQueue

	顯示路徑的物件
*/
#pragma once

#include "SBase.h"

template<class MsgData>
class SMsgQueue {
public:
	/*
		取得 queue 第一個 MSG
		回傳 true 當 msg 有效
		回傳 false 當物件忙碌(Timeout)或 queue 沒有資料
	*/
	bool PeekMsg(SIN const int time_out_msec, SOUT MsgData& msg) {
		bool result = false;
		if (mutex_.try_lock_for(std::chrono::milliseconds(time_out_msec))) {
			if (!queue_.empty()) {
				result = true;
				msg = queue_.front();
				queue_.pop();
			}
			mutex_.unlock();
		}
		return result;
	}
	/*
		加入訊息
	*/
	void PushMsg(SIN const MsgData& msg) {
		mutex_.lock();
		queue_.push(msg);
		mutex_.unlock();
	}
	/*
		清空訊息
	*/
	void Clear() {
		mutex_.lock();
		while (!queue_.empty()) queue_.pop();
		mutex_.unlock();
	}
protected:
	std::recursive_timed_mutex mutex_;
	std::queue<MsgData> queue_;
};
