#pragma once

#include "SSimpleHash.h"
#include "SIorq.h"

#include "slib\SBase.h"
#include "slib\SCharAry.h"
#include "slib\SPathLib.h"

const int HASH_CNT = 3;

struct SStrItem {
	uint32_t hash_[HASH_CNT];
	SCharAry str_;

	void Set(SIN SCharAry& str, SHasher& hasher) {
		str_ = str;
		Hash(hasher);
	}

	void Hash(SIN SHasher& hasher) {
		int i;
		SRP(i, HASH_CNT) hash_[i] = hasher.Hash((const char*)str_.Ptr(), str_.Length()*sizeof(TCHAR), false, i);
	}

	void operator=(const SStrItem& item) {
		int i;
		SRP(i, HASH_CNT) hash_[i] = item.hash_[i];
		str_ = item.str_;
	}

	bool operator<(const SStrItem& item) const {
		int i;
		SRP(i, HASH_CNT) {
			if (hash_[i] < item.hash_[i]) return true;
			if (hash_[i] > item.hash_[i]) return false;
		}
		return str_ < item.str_;
	}
	bool operator==(const SStrItem& item) const {
		int i;
		SRP(i, HASH_CNT) {
			if (hash_[i] != item.hash_[i]) return false;
		}
		return str_ == item.str_;
	}
	bool operator!=(const SStrItem& item) const {
		int i;
		SRP(i, HASH_CNT) {
			if (hash_[i] != item.hash_[i]) return true;
		}
		return str_ != item.str_;
	}
	bool operator>(const SStrItem& item) const {
		int i;
		SRP(i, HASH_CNT) {
			if (hash_[i] > item.hash_[i]) return true;
			if (hash_[i] < item.hash_[i]) return false;
		}
		return str_ > item.str_;
	}
};
	

/*
	使用 map 紀錄 exe -> vector<file>
*/
class SCache {
public:
	SCache(SIN SHasher* hasher) { hasher_ = hasher; }
	void Cache(SIN SIORQ* iorq) {
		SStrItem exe, file;
		CreateItem(iorq, exe, file);

		MIT mit = cache_.find(exe);
		if (mit == cache_.end()) {
			std::vector<SStrItem> vec;
			cache_[exe] = vec;
			cache_[exe].push_back(file);
		}
		else {
			std::vector<SStrItem>* vec = &(mit->second);
			VIT vit = std::lower_bound(vec->begin(), vec->end(), file);
			if (vit != vec->end() && *vit == file) {
				//	重複的 cache
				return;
			}
			vec->insert(vit, file);
		}
	}
	bool IsCached(SIN SIORQ* iorq) {
		SStrItem exe, file;
		CreateItem(iorq, exe, file);

		MIT mit = cache_.find(exe);
		if (mit == cache_.end()) return false;

		std::vector<SStrItem>* vec = &(mit->second);
		VIT vit = std::lower_bound(vec->begin(), vec->end(), file);
		return vit != vec->end() && *vit == file;
	}
	void GetExeList(SIN std::vector<SStrItem>& list) {
		for (MIT i = cache_.begin(); i != cache_.end(); ++i) {
			list.push_back(i->first);
		}
	}
	void DeleteExeList(SIN std::vector<SStrItem>& list) {
		MIT mit;
		int i;
		SRP(i, list.size()) {
			mit = cache_.find(list[i]);
			if (mit != cache_.end()) {
				cache_.erase(mit);
			}
		}
	}
	typedef std::map<SStrItem, std::vector<SStrItem>>::iterator MIT;
	typedef std::vector<SStrItem>::iterator VIT;
private:
	void CreateItem(SIN SIORQ* iorq, SOUT SStrItem& exe, SStrItem& file) {
		TCHAR buffer[MAX_PATH];

		sis::path::GetFileName(iorq->exe_.Ptr(), buffer);
		exe.str_ = buffer;
		exe.Hash(*hasher_);

		file.Set(iorq->file_, *hasher_);
	}

	std::map<SStrItem, std::vector<SStrItem>> cache_;
	SHasher* hasher_;
};

/*
class SCache {
public:
	void CleanIORQ(SIN SIORQ* iorq) {
		IT it = Find(iorq);
		if (it == cache_.end()) return;

		InstanceCount *cnt = &(it->second);
		IIT iit = InstanceFind(cnt, iorq);
		if (iit == cnt->end()) return;

		if (iorq->access_.IO_CLEANUP == 1) iit->second = 0;
		else --iit->second;
		if (iit->second == 0) {
			cnt->erase(iit);

			if (cnt->size() == 0) {
				cache_.erase(it);
			}
		}
	}
	void Cache(SIN SIORQ* iorq) {
		IT it = Find(iorq);
		if (it != cache_.end()) {
			InstanceCount *cnt = &(it->second);
			IIT iit = InstanceFind(cnt, iorq);
			if (iit != cnt->end()) {
				++iit->second;
			}
			else {
				(*cnt)[iorq->pid_] = 1;
			}
		}
		else {
			InstanceCount cnt;
			cnt[iorq->pid_] = 1;
			cache_[iorq->file_] = cnt;
		}
	}
	bool IsCached(SIN SIORQ* iorq) {
		IT it = Find(iorq);
		if (it == cache_.end()) return false;

		return InstanceFind(&(it->second), iorq) != it->second.end();
	}
private:
	typedef std::map<SCharAry, std::map<HANDLE, USHORT>>::iterator IT;
	typedef std::map<HANDLE, USHORT> InstanceCount;
	typedef std::map<HANDLE, USHORT>::iterator IIT;

	inline IT Find(SIN SIORQ* iorq) {
		return cache_.find(iorq->file_);
	}
	inline IIT InstanceFind(SIN InstanceCount* cnt, SIORQ* iorq) {
		return cnt->find(iorq->pid_);
	}

	std::map<SCharAry, std::map<HANDLE, USHORT>> cache_;
};*/
