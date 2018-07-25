#include "PlatformPrecomp.h"
#include "VariantDB.h"
#include "util/ResourceUtils.h"


VariantDB::VariantDB()
{
	ResetNext();
}

VariantDB::~VariantDB()
{
	DeleteAll();
}

void VariantDB::DeleteAll()
{
	dataList::iterator itor = m_data.begin();
	while (itor != m_data.end())
	{
		SAFE_DELETE (itor->second);
		itor++;
	}
	
	m_data.clear();
	{ //so I can use "itor" again
	functionList::iterator itor = m_functionData.begin();
	while (itor != m_functionData.end())
	{
		delete (itor->second);
		itor++;
	}
	}
	m_functionData.clear();
}


int VariantDB::DeleteVar(string keyName)
{
	int deleted = 0;

	dataList::iterator itor = m_data.begin();

	while (itor != m_data.end())
	{
		if (itor->first == keyName)
		{
			//match!
			delete (itor->second);
			dataList::iterator itorTemp = itor;
			itor++;

			m_data.erase(itorTemp);
			deleted++;
			continue;
		}
		itor++;
	}

	return deleted;
}

Variant * VariantDB::GetVarIfExists(const string &keyName)
{
	dataList::iterator itor = m_data.find(keyName);

	if (itor != m_data.end())
	{
		//bingo!
		return &( (*itor->second));
	}

	return NULL;
}



Variant * VariantDB::GetVarWithDefault(const string &keyName, const Variant &vDefault)
{

	Variant *pData = GetVarIfExists(keyName);

	if (!pData)
	{
		//create it
		pData = new Variant(vDefault);
		m_data[keyName]=pData;
	}

	return pData;
}

Variant * VariantDB::GetVar(const string &keyName)
{

	Variant *pData = GetVarIfExists(keyName);

	if (!pData)
	{
		//create it
		pData = new Variant;
		m_data[keyName]=pData;
	}

	return pData;
}

FunctionObject * VariantDB::GetFunctionIfExists(const string &keyName)
{
	functionList::iterator itor = m_functionData.find(keyName);

	if (itor != m_functionData.end())
	{
		//bingo!
		return &( (*itor->second));
	}
	return NULL; //doesn't exist
}


FunctionObject * VariantDB::GetFunction( const string &keyName )
{
	FunctionObject *pData = GetFunctionIfExists(keyName);

	if (!pData)
	{
		//create it
		pData = new FunctionObject;
		m_functionData[keyName]=pData;
	}
	
	return pData;
}

void VariantDB::CallFunctionIfExists( const string &keyName, VariantList *pVList )
{
	FunctionObject *pFunc = GetFunctionIfExists(keyName);
	if (pFunc)
	{
		pFunc->sig_function(pVList);
	}
}

bool VariantDB::Save(const string &fileName, bool bAddBasePath)
{
	
	string f;

	if (bAddBasePath)
	{
		f = GetSavePath()+fileName;
	} else
	{
		f = fileName;
	}

	FILE *fp = fopen( f.c_str(), "wb");
	
	if (!fp)
	{
		LogError("Unable to save data");
		return false;
	}
	uint32 version = C_VARIANT_DB_FILE_VERSION;
	fwrite(&version, sizeof(uint32), 1, fp);

	dataList::iterator itor = m_data.begin();
	
	while (itor != m_data.end())
	{
		Variant *pV = itor->second;
		
		if (!pV->Save(fp, itor->first))
		{
			LogError("Unable to save data");
			fclose(fp);
			return false;
		}

		itor++;
	}

	//add out done marker
	uint32 doneMarker = Variant::TYPE_UNUSED;
	fwrite(&doneMarker, sizeof(uint32), 1, fp);

	fclose(fp);
	return true;
}

bool VariantDB::Load( const string &fileName, bool *pFileExistedOut, bool bAddBasePath )
{
	string f;

	if (bAddBasePath)
	{
		f = GetSavePath()+fileName;
	} else
	{
		f = fileName;
	}

	FILE *fp = fopen( f.c_str(), "rb");

	if (!fp)
	{
		if (pFileExistedOut) *pFileExistedOut = false;
#ifdef _DEBUG
		LogMsg("%s doesn't exist", f.c_str());
#endif
		return true; //not here, but that's not really an error
	}

	//get the version
	uint32 version;
	if (pFileExistedOut) *pFileExistedOut = true;
	size_t bytesRead = fread(&version, 1, sizeof(uint32), fp);

	if (bytesRead == 0 || version != 1)
	{
		LogMsg("%s - unexpected version. Deleting file", f.c_str());
		fclose(fp);

		RemoveFile(f, false);
		
		assert(!"Unexpected version?!");
		return false;
	}

	string s;
	uint32 varType;
	
	while (!feof(fp))
	{
		fread(&varType, 1, sizeof(uint32), fp);

		if (varType == Variant::TYPE_UNUSED)
		{
			//out signal that we're done reading this part
			break;
		}

		//get the var name
		LoadFromFile(s, fp);
		
#ifdef _DEBUG
		if (GetVarIfExists(s) != NULL) {
			LogMsg("VariantDB: variable %s already exists in database while loading from file %s. The previous value gets overwritten!", s.c_str(), fileName.c_str());
		}
#endif

		//get the actual data too
		switch(varType)
		{

		case Variant::TYPE_STRING:
			{
				string v;
				LoadFromFile(v, fp);
				GetVar(s)->Set(v);
				break;
			}

		case Variant::TYPE_UINT32:
			{
				uint32 v;
				LoadFromFile(v, fp);
				GetVar(s)->Set(v);
				break;
			}
	
		case Variant::TYPE_INT32:
			{
				int32 v;
				LoadFromFile(v, fp);
				GetVar(s)->Set(v);
				break;
			}

		case Variant::TYPE_FLOAT:
			{
				float v;
				LoadFromFile(v, fp);
				GetVar(s)->Set(v);
				break;
			}

		case Variant::TYPE_VECTOR2:
			{
				CL_Vec2f v;
				LoadFromFile(v, fp);
				GetVar(s)->Set(v);
				break;
			}
	
		case Variant::TYPE_VECTOR3:
			{
				CL_Vec3f v;
				LoadFromFile(v, fp);
				GetVar(s)->Set(v);
				break;
			}

		case Variant::TYPE_RECT:
			{
				CL_Rectf v;
				LoadFromFile(v, fp);
				GetVar(s)->Set(v);
				break;
			}
		
		default:
			LogMsg("%s - unknown var type", f.c_str());

			assert(!"Unknown var type");

			fclose(fp);
			return false;
		}


	}

	fclose(fp);

	return true;

}

void VariantDB::Print()
{
	dataList::iterator itor = m_data.begin();

	LogMsg("Listing VariantDB contents");
	LogMsg("*********************");
	while (itor != m_data.end())
	{
		Variant *pV = itor->second;
		string s = itor->first + ": "+pV->Print();

		LogMsg( s.c_str());
		itor++;
	}
	LogMsg("*********************");
}

int VariantDB::DeleteVarsStartingWith( string deleteStr )
{
	int deleted = 0;
	
	dataList::iterator itor = m_data.begin();

	while (itor != m_data.end())
	{
		if (itor->first.compare(0, deleteStr.size(), deleteStr) == 0)
		{
			//match!
			delete (itor->second);
			dataList::iterator itorTemp = itor;
			itor++;

			m_data.erase(itorTemp);
			deleted++;
			continue;
		}
		itor++;
	}

	return deleted;
}

std::string VariantDB::DumpAsString()
{
	string log = "*********************\r\n";

	dataList::iterator itor = m_data.begin();

	while (itor != m_data.end())
	{
		Variant *pV = itor->second;
		string s = itor->first + ": "+pV->Print();

		log += s + " ";
		itor++;
	}

	log += "\r\n";
	return log;
}

void VariantDB::ResetNext()
{
	m_nextItor = m_data.begin();
}

Variant * VariantDB::GetNext(string &keyOut)
{
	Variant *pReturn = NULL;

	if (m_nextItor == m_data.end())
	{
		//all done
		ResetNext();
		return NULL;
	}

	keyOut = m_nextItor->first;
	pReturn = m_nextItor->second;
	m_nextItor++;
	return pReturn;
}

int VariantDB::AddVarPointersToVector( vector<pair<const string*, Variant*> > *varListOut, const string keyMustStartWithThisText/*=""*/ )
{
	int count = 0;
	dataList::iterator itor = m_data.begin();

	while (itor != m_data.end())
	{
		//Variant *pV = itor->second;
		
		if (keyMustStartWithThisText.empty() || StringFromStartMatches(itor->first,keyMustStartWithThisText))
		{
			varListOut->push_back(make_pair(&itor->first, itor->second));
			count++;
		}

		itor++;
	}

	return count;
}

void VariantDB::Clear()
{
	m_data.clear();
	m_functionData.clear();
	ResetNext();
}