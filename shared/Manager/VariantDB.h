//  ***************************************************************
//  SharedDB - Creation date: 04/11/2009
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2009 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef VariantDB_h__
#define VariantDB_h__
#include "util/Variant.h"

//modified so msvc2015 will compile, from hisadg123 
#if _MSC_VER >= 1700
#define _SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS
#endif
class Variant;


#define C_VARIANT_DB_FILE_VERSION 1

class FunctionObject
{
public:

	boost::signal<void (VariantList*)> sig_function;
};

#ifdef PLATFORM_BBX
	//I'd rather not use this, but I couldn't get the hash_map versions to work with bb
	#define C_USE_BOOSTS_HASHMAP 1
#endif
	
#ifdef C_USE_BOOSTS_HASHMAP

	#include "util/boost/boost/unordered/unordered_map.hpp"
	typedef boost::unordered_map<string, Variant*> dataList;
	typedef boost::unordered_map<string, FunctionObject*> functionList;

#else

#if defined( __APPLE__) || defined(RTLINUX)|| defined(PLATFORM_LINUX)||ANDROID_NDK || defined(PLATFORM_FLASH) || defined(PLATFORM_HTML5)

#ifdef PLATFORM_HTML5


	#include <unordered_map>
	typedef unordered_map<string, Variant*> dataList;
	typedef unordered_map<string, FunctionObject*> functionList;


#else
	 

	#define _GLIBCXX_PERMIT_BACKWARD_HASH

	#include <ext/hash_map>

	namespace __gnu_cxx {
		/**
		Explicit template specialization of hash of a string class,
		which just uses the internal char* representation as a wrapper.
		*/
		template <>
		struct hash<std::string> {
			size_t operator() (const std::string& x) const {
				return hash<const char*>()(x.c_str());
				// hash<const char*> already exists
			}
		};
	}


	typedef  __gnu_cxx::hash_map<string, Variant*> dataList;
	typedef  __gnu_cxx::hash_map<string, FunctionObject*> functionList;
#endif
#else
	#include <hash_map>

		#if defined RT_WEBOS_ARM || ANDROID_NDK 

		namespace __gnu_cxx {
			/**
			Explicit template specialization of hash of a string class,
			which just uses the internal char* representation as a wrapper.
			*/
			template <>
			struct hash<std::string> {
				size_t operator() (const std::string& x) const {
					return hash<const char*>()(x.c_str());
					// hash<const char*> already exists
				}
			};
		}

		typedef  __gnu_cxx::hash_map<string, Variant*> dataList;
		typedef  __gnu_cxx::hash_map<string, FunctionObject*> functionList;

		#else
				typedef stdext::hash_map<string, Variant*> dataList;
				typedef stdext::hash_map<string, FunctionObject*> functionList;
		#endif

	#endif

#endif

class VariantDB
{
public:
	VariantDB();
	virtual ~VariantDB();

	FunctionObject * GetFunction(const string &keyName); //created it needed, this is usually what you want
	FunctionObject * GetFunctionIfExists(const string &keyName); //returns null if not created yet
	void CallFunctionIfExists(const string &keyName, VariantList *pVList);

	Variant * GetVar(const string &keyName);  //created it needed, this is usually what you want
	Variant * GetVarIfExists(const string &keyName);  //returns null if not created yet
	Variant * GetVarWithDefault(const string &keyName, const Variant &vDefault);
	int DeleteVarsStartingWith(string deleteStr); //returns how many were deleted
	int DeleteVar(string keyName); //returns how many were deleted (0 or 1..)

	//you can load and save the variables in the DB on the fly.  (Does nothing with the functions)
	bool Save(const string &fileName, bool bAddBasePath = true);
	bool Load(const string &fileName, bool *pFileExistedOut = NULL, bool bAddBasePath = true);
	
	string DumpAsString();
	void Print(); //same as above, but sends to LogMsg()
	void DeleteAll();
	int GetVarCount() {return (int) m_data.size();}

	void Clear();
	//to get each var in our db manually, do this:
	void ResetNext(); //call before starting a search
	Variant * GetNext(string &keyOut); //call this in a loop until it returns NULL to signal the finish, callResetNext() before using!
	int AddVarPointersToVector(vector<pair<const string*, Variant*> > *varListOut, const string keyMustStartWithThisText="");


VariantDB & operator= (const VariantDB &rhs)
	{
		//m_functionData = rhs.m_functionData;
		dataList::const_iterator itor = rhs.m_data.begin();

		while (itor != rhs.m_data.end())
		{
			m_data[itor->first] = new Variant(*itor->second);
			itor++;
		}
		return *this; 
	}

VariantDB(const VariantDB & ref)
{
	//assert(!"Warning, slow operation!");
	//our special copy won't include the sig crap, that stuff can't be copied
	*this = ref;
}

private:

	dataList m_data;
	functionList m_functionData;
	dataList::iterator m_nextItor;
};

#endif // Variant_h__
