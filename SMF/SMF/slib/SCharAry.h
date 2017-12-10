/*
	Copyright © 2016 by Phillip Chang

	SCharAry

	字串
*/
#pragma once

#include "SStrLib.h"

/*
	所有函數參數 length 皆不包含 NULL
	結構內部 data_, length_ 都有包含 NULL 結尾
	結構保證 data_ 尾端有 NULL 結尾

	Length() 回傳不含 NULL 結尾長度
*/
class SCharAry {
public:
	//	initialize
	SCharAry() {
		data_ = NULL;
		length_ = 0;
	}
	SCharAry(SIN const TCHAR* str, const size_t length) {
		data_ = NULL;
		Copy(str, length);
	}
	SCharAry(SIN const TCHAR* str) {
		data_ = NULL;	//	防止 clear 誤刪無效指針
		Copy(str);
	}
	SCharAry(SIN const SCharAry& ary) {
		data_ = NULL;
		*this = ary;
	}
	~SCharAry() {
		Clear();
	}

	//	access
	inline TCHAR* Ptr() { return data_; }
	inline const TCHAR* Ptr() const { return data_; }
	//	回傳不含 NULL 結尾長度
	inline const size_t Length() const { return length_ - 1; }

	//	allocate
	void Allocate(SIN const size_t length) {
		Clear();

		length_ = length + 1;
		data_ = new TCHAR[length_];
		data_[0] = data_[length] = 0;	//	NULL ending
	}
	void Clear() {
		if (data_ != NULL) {
			delete[]data_;
			data_ = NULL;
			length_ = 0;
		}
	}
	/*
		copy
	*/
	void Copy(SIN const TCHAR* str, const size_t length) {
		Clear();
		
		length_ = length + 1;
		data_ = new TCHAR[length_];
		
		sis::str::Copy(str, length, data_);
		data_[length] = 0;
	}
	inline void Copy(SIN const TCHAR* str) {
		Copy(str, sis::str::Length(str));
	}

	//	access
	TCHAR& operator[](SIN const size_t index) { return data_[index]; }
	const TCHAR& operator[](SIN const size_t index) const { return data_[index]; }

	SCharAry& operator=(SIN const SCharAry& ary) {
		Copy(ary.data_, ary.Length());
		return *this;
	}
	SCharAry& operator=(SIN const TCHAR* ary) {
		Copy(ary);
		return *this;
	}
	bool operator<(SIN const SCharAry& ary) const {
		return sis::str::Compare(data_, Length(), ary.data_, ary.Length()) == sis::str::LESSER;
	}
	bool operator==(SIN const SCharAry& ary) const {
		return sis::str::Equal(data_, Length(), ary.data_, ary.Length());
	}
	bool operator!=(SIN const SCharAry& ary) const {
		return !sis::str::Equal(data_, Length(), ary.data_, ary.Length());
	}
	bool operator>(SIN const SCharAry& ary) const {
		return sis::str::Compare(data_, Length(), ary.data_, ary.Length()) == sis::str::LARGER;
	}
private:
	TCHAR* data_;
	size_t length_;
};
