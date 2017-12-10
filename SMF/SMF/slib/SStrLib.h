/*
	Copyright © 2016 by Phillip Chang

	SStrLib

	*	定義常用字串函數
*/
#pragma once

#include "SBase.h"

/*
	CompareResult			- 定義字串比較結果，LESSER 表示字典序較小(Ex: a LESSER b)
	
	size_t Length			- 回傳字串長度
	CompareResult Compare	- 回傳兩字串(str1,str2)比較的結果 R，表示 str1 R str2
	bool IsPrefix			- 回傳兩字串(src,prefix)中，prefix 是不是 src 的前綴
	void Copy				- 允許兩種不同型態的字元陣列複製
	TCHAR Upper/Lower		- 回傳大寫/小寫字元
	void Upper/Lower		- 輸出大寫/小寫字串
*/
namespace sis {
	namespace str {
		enum CompareResult {
			LARGER,
			LESSER,
			EQUAL
		};
		//	max = 0xFFFFFF
		size_t Length(SIN const TCHAR* str) {
			for (size_t i = 0;; ++i) if (str[i] == 0 || i > 0xFFFFFF) return i;
		}

		bool Equal(SIN const TCHAR* str1, const size_t length1, const TCHAR* str2, const size_t length2) {
			size_t i;
			if (length1 != length2) return false;
			SRP(i, length1) {
				if (str1[i] != str2[i]) return false;
			}
			return true;
		}
		bool Equal(SIN const TCHAR* str1, const TCHAR* str2) {
			return Equal(str1, Length(str1), str2, Length(str2));
		}

		CompareResult Compare(SIN const TCHAR* str1, const size_t length1, const TCHAR* str2, const size_t length2) {
			size_t i, length = length1 > length2 ? length1 : length2;
			SRP(i, length) {
				if (str1[i] < str2[i]) return LESSER;
				else if (str1[i]>str2[i]) return LARGER;
			}
			if (length1 == length2) return EQUAL;
			return length1>length2 ? LARGER : LESSER;
		}
		CompareResult Compare(SIN const TCHAR* str1, const TCHAR* str2) {
			return Compare(str1, Length(str1), str2, Length(str2));
		}

		bool IsPrefix(SIN const TCHAR* src, const size_t length, const TCHAR* prefix, const size_t prefix_length) {
			return length>=prefix_length && Equal(src, prefix_length, prefix, prefix_length);
		}
		/*
			回傳 parent 是否為 path 的祖先目錄
		*/
		bool IsParentPath(SIN const TCHAR* path, const size_t path_length, const TCHAR* parent, const size_t parent_length) {
			return path_length > parent_length && IsPrefix(path, path_length, parent, parent_length) && path[parent_length] == TEXT('\\');
		}

		void Copy(SIN const TCHAR* src, const size_t length, SOUT TCHAR* des) {
			size_t i;
			SRP(i, length) des[i] = src[i];
		}
		void Copy(SIN const TCHAR* src, SOUT TCHAR* des) {
			size_t i, length = Length(src) + 1;	//	include NULL
			SRP(i, length) des[i] = src[i];
		}

		TCHAR Lower(SIN const TCHAR ch) {
			if (ch >= 'A' && ch <= 'Z') return ch - 'A' + 'a';
			else return ch;
		}
		TCHAR Upper(SIN const TCHAR ch) {
			if (ch >= 'a' && ch <= 'z') return ch - 'a' + 'A';
			else return ch;
		}
		void Lower(SINOUT TCHAR* str, SIN const size_t length) {
			size_t i;
			SRP(i, length) str[i] = Lower(str[i]);
		}
		void Upper(SINOUT TCHAR* str, SIN const size_t length) {
			size_t i;
			SRP(i, length) str[i] = Upper(str[i]);
		}
	}
}
