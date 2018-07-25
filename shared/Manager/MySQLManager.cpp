#include "PlatformPrecomp.h"
#include "MySQLManager.h"

#define HAVE_INT32
#define HAVE_UINT32
#define byte_defined
#include <my_global.h>
#include <mysql.h>

#if defined _CONSOLE
uint32 GetTick();
#else
#include "BaseApp.h"
#endif

#define C_MYSQL_PING_TIMER_MS (1000*60*60)*4
#include "util/MiscUtils.h"

MySQLManager::MySQLManager()
{
	m_conn = NULL;
	m_pingTimer = 0;
	m_bLostServerConnection = false;
	m_opsDone = 0;
}

MySQLManager::~MySQLManager()
{
	Kill();
}

void MySQLManager::Kill()
{
	if (m_conn)
	{
		mysql_close(m_conn);
		m_conn = NULL;
	}
}




std::string MySQLManager::GetLastError()
{
	if (!m_conn) return "(not connected to mysql)";

	int error = mysql_errno(m_conn);

	return string(" SQL Error: ")+toString(error)+": "+string(mysql_error(m_conn));
}

//sorry, you need to implement this somewhere.
void LogMsgChat ( const string s );

int MySQLManager::ShowError(string optionalLabel)
{
	if (!m_conn) return 0;

	int error = mysql_errno(m_conn);

	//label could contain %s and such, so let's log it a safer way to avoid a possible crash

	if (optionalLabel.length() > 128)
	{
	   TruncateString(optionalLabel, 128);
	}

	LogMsgChat("MySQLManager error: "+toString(error)+" "+mysql_error(m_conn)+" ("+optionalLabel+")");
	return error;
}

bool MySQLManager::DoesTableExist(string tableName, bool bShowErrors)
{
	if (!m_conn)
	{
		LogError("Why you trying toDoesTableExist when SQL isn't initted?");
		return false;
	}

	assert(m_conn);
	MYSQL_RES *result = NULL;

	//old way
	//bool bSuccess = Query("SELECT COUNT(*) FROM "+tableName, bShowErrors);
	bool bSuccess = Query("SHOW TABLES LIKE '"+tableName+"'", bShowErrors);

	if (!bSuccess) return false;

	result = mysql_store_result(m_conn);
	
	int fields = mysql_num_fields(result);
	int rows = (int)mysql_num_rows(result);
	mysql_free_result(result);
	
	return rows > 0;
}

bool MySQLManager::DoResultsExist()
{

	if (!m_conn)
	{
		LogError("Why you trying to DoResultsExist when SQL isn't initted?");
		return false;
	}

	MYSQL_RES *result = NULL;
	result = mysql_store_result(m_conn);

	if (!result)
	{
		return false;
	}
	int rows = (int)mysql_num_rows(result);

	mysql_free_result(result);
	return rows != 0;
}


int MySQLManager::AddSelectResults(vector<VariantDB> &vdb)
{
	if (!m_conn)
	{
		LogError("Why you trying to AddSelectResults when SQL isn't initted?");
		return false;
	}

	MYSQL_RES *result = NULL;
	result = mysql_store_result(m_conn);

	if (!result)
	{
		return 0;
	}

	int num_fields = mysql_num_fields(result);
	int rows = (int)mysql_num_rows(result);

	MYSQL_ROW row;
	MYSQL_FIELD *field;
	
	vector<string> fieldNames;
	vector<enum_field_types> fieldType;
	vector<int> maxLength;
	vector<bool> isBinary;

	int curRow = (int)vdb.size();
	vdb.resize(curRow+rows);

	bool bGotFields = false;

	while ((row = mysql_fetch_row(result)))
	{
	
	  VariantDB &db = vdb[curRow++];

	  if (!bGotFields) 
	  {
		  bGotFields = true;

		  while(field = mysql_fetch_field(result)) 
		  {
			  fieldNames.push_back(field->name);
			  fieldType.push_back(field->type);
			  maxLength.push_back(field->max_length);
			  isBinary.push_back((field->flags&BINARY_FLAG)!=0);
		  }
	  }
	  for(int i = 0; i < num_fields; i++)
		{

			switch(fieldType[i])
			{
			case FIELD_TYPE_NEWDECIMAL:
			case FIELD_TYPE_DECIMAL:
			case FIELD_TYPE_FLOAT:
			case FIELD_TYPE_DOUBLE:
				if (!row[i])
				{
					db.GetVar(fieldNames[i])->Set((float)0);
				} else
				{
					db.GetVar(fieldNames[i])->Set((float)atof(row[i]));
				}
				break;

			case FIELD_TYPE_SHORT:
			case FIELD_TYPE_LONG:
			case FIELD_TYPE_LONGLONG:
				if (!row[i])
				{
					db.GetVar(fieldNames[i])->Set((int32)0);
				} else
				{
					db.GetVar(fieldNames[i])->Set((int32)atoi(row[i]));
				}
				break;

			case FIELD_TYPE_DATETIME:
				{
					if (!row[i])
					{
						//it's null, no date set. Guess we'll just call that a 0.
						db.GetVar(fieldNames[i])->Set(uint32(0));
					} else
					{
						uint	y, m, d, h, mn, s;
						uint nbScanned = sscanf(row[i], "%u-%u-%u %u:%u:%u", &y, &m, &d, &h, &mn, &s);
						assert(nbScanned == 6);
						tm	myTm;
						myTm.tm_year = y-1900;
						myTm.tm_mon = m-1;
						myTm.tm_mday = d;
						myTm.tm_hour = h;
						myTm.tm_min = mn;
						myTm.tm_sec = s;
				
						myTm.tm_isdst = -1; // let the C runtime determine daylight adjustment
						myTm.tm_wday = -1;
						myTm.tm_yday = -1;
						//assert( sizeof(time_t) == 4 && "Uh oh.. define _USE_32BIT_TIME_T somewhere for MSVC");
						uint32 t = (uint32)mktime(&myTm);
						db.GetVar(fieldNames[i])->Set(t);
					}
				}
				break;

			case FIELD_TYPE_DATE:
				{
					//convert the sql style date ('YYYY-MM-DD') into a unix style date with string processing
					if (!row[i])
					{
						//it's null, no date set. Guess we'll just call that a 0.
						db.GetVar(fieldNames[i])->Set(uint32(0));
					} else
					{
						uint	y, m, d;
						uint nbScanned = sscanf(row[i], "%u-%u-%u", &y, &m, &d);
						assert(nbScanned == 3);
						tm	myTm;
						myTm.tm_year = y-1900;
						myTm.tm_mon = m-1;
						myTm.tm_mday = d;
						myTm.tm_hour = 0;
						myTm.tm_min = 0;
						myTm.tm_sec = 0;

						myTm.tm_isdst = -1; 
						myTm.tm_wday = -1;
						myTm.tm_yday = -1;
						//assert( sizeof(time_t) == 4 && "Uh oh.. define _USE_32BIT_TIME_T somewhere for MSVC");
						uint32 t = (uint32) mktime(&myTm);
						db.GetVar(fieldNames[i])->Set(t);
					}
				}
				break;

			case FIELD_TYPE_TIMESTAMP:
			{
					tm	tm;
					memset( &tm, 0, sizeof( tm ));
					int nCnt = sscanf( row[i],"%4u%2u%2u%2u%2u%2u",
						&tm.tm_year, &tm.tm_mon, &tm.tm_mday,
						&tm.tm_hour, &tm.tm_min, &tm.tm_sec );

					if( nCnt == 6 ) 
					{
						tm.tm_year = tm.tm_year - 1900;
						tm.tm_mon--;
					} else
					{
						LogMsg("Error converting mysql timestamp");
					}

					//assert( sizeof(time_t) == 4 && "Uh oh.. define _USE_32BIT_TIME_T somewhere for MSVC");


					uint32 t = (uint32) mktime(&tm);
					db.GetVar(fieldNames[i])->Set(t);
				}
				break;

			/*case FIELD_TYPE_VAR_STRING:
				{
					if(isBinary[i])	// this is actually a VARBINARY
					{
						//first we'll get the size of the data in here
						db.GetVar(fieldNames[i])->Set(string()); //we need to register it as a string, the mega hack we do in a
						string &s = db.GetVar(fieldNames[i])->GetString();
						//second won't do it..
						if (maxLength[i] > 0)
						{
							//now put it into the string, keeping things like nulls and such.  (up to you to pull it out right though)
							s.resize(maxLength[i]);

							if (row[i])
								memcpy((void*)s.c_str(), &row[i][0], maxLength[i]);
						}
					}
					else	// this is a VARCHAR
					{
						
					}
				}
				break;
			*/
			case FIELD_TYPE_STRING:
			case FIELD_TYPE_VAR_STRING:
				if (!row[i])
				{
					//well, it's null.  Just pretend it's a blank string
					db.GetVar(fieldNames[i])->Set("");
				} else
				{
					if(isBinary[i])
					{
						db.GetVar(fieldNames[i])->Set(string());
						string &s = db.GetVar(fieldNames[i])->GetString();
						if (maxLength[i] > 0)
						{
							//now put it into the string, keeping things like nulls and such.  (up to you to pull it out right though)
							s.resize(maxLength[i]);

							if (row[i])
								memcpy((void*)s.c_str(), &row[i][0], maxLength[i]);
						}
					} 
					else
					{
						db.GetVar(fieldNames[i])->Set(string(row[i]));
					}
				}
				break;

			case FIELD_TYPE_BLOB:
				if (!row[i])
				{
					//well, it's null.  Just pretend it's a blank string
					db.GetVar(fieldNames[i])->Set("");
				} else
				{
					db.GetVar(fieldNames[i])->Set(string(row[i]));
				}
				break;

			default:;
				assert(!"Unknown mysql type");
				db.GetVar(fieldNames[i])->Set(string(row[i]));

			}

		}
	}
	//printf("\n");

	mysql_free_result(result);

  return rows;
}

bool MySQLManager::Init(string name, string password, string host)
{

	LogMsg("MySQL client version: %s", mysql_get_client_info());
	Kill();
	//store these so we can re-connect ourselves if needed

	m_conn = mysql_init(NULL);
	
	if (!m_conn)
	{
		ShowError("");
		return false;
	}

	//actually connect?

	if (!mysql_real_connect(m_conn, host.c_str(), name.c_str(), password.c_str(), NULL, 0, NULL, 0))
	{
		ShowError("");
		Kill(); //this will make m_conn null again
		return false;
	}

	m_bLostServerConnection = false;

	return true;
}

bool MySQLManager::Query( string query, bool bShowError )
{
#ifdef _DEBUG
	//LogMsg("Querying %s", query.c_str() );
#endif

	if (!m_conn)
	{
		LogError("Why you trying to Query when SQL isn't initted?");
		return false;
	}

	m_opsDone++;

	if (mysql_query(m_conn, query.c_str()))
	{
		if (bShowError)
		{
			int error = ShowError(query);
			if (error == 2006) //this should be CR_SERVER_GONE_ERROR, but I can't find the define..
			{
				//let our user know something is desperately wrong.  mysql service probably died, requiring a restart
				m_bLostServerConnection = true;
			}
			
		}
		return false;
	}

	return true;
}


int MySQLManager::GetLastAutoIncrementInsertID()
{
	return (int)mysql_insert_id(m_conn);
}

void MySQLManager::Update()
{

	if (m_conn && m_pingTimer < GetTick() || GetTick() > m_pingTimer+C_MYSQL_PING_TIMER_MS )
	{
		//keep the DB connection alive, if there were no accesses it can time-out
		//LogMsg("Ping! pingtimer is %u, system is %u.  Internval is %d", m_pingTimer, GetSystemTimeTick(), C_MYSQL_PING_TIMER_MS);
		DoesTableExist("BogusTable", false);
		m_pingTimer = GetTick()+uint32(C_MYSQL_PING_TIMER_MS);
	}
}

std::string MySQLManager::EscapeString( const string &input )
{
	char *pBuffer = new char[input.length()*2+1];

	mysql_real_escape_string(m_conn, pBuffer, input.c_str(), (unsigned long)input.size());

	string ret = pBuffer;
	SAFE_DELETE_ARRAY(pBuffer);

	return ret;
}

int MySQLManager::GetTableRecordCount( string tableName )
{

#ifdef _DEBUG
	//LogMsg("GetTableRecordCount: %s", tableName.c_str() );
#endif
	string sql = "SELECT COUNT(*) FROM "+tableName;
	if (!Query(sql, true))
	{
		LogError("(Couldn't GetTableRecordCount)");
		return 0;
	}
	vector<VariantDB> vdb;
	AddSelectResults(vdb);
	return vdb[0].GetVar("COUNT(*)")->GetINT32();
}

bool MySQLManager::DoesTableRecordExistFast(string tableName, int record)
{
	string sql = "SELECT COUNT(ID) FROM "+tableName+" where ID = "+toString(record)+" LIMIT 1";
	if (!Query(sql, true))
	{
		LogError("(Couldn't DoesTableRecordExist, it must have an indexed ID member for this to work!");
		return 0;
	}
	vector<VariantDB> vdb;
	AddSelectResults(vdb);
	return vdb[0].GetVar("COUNT(ID)")->GetINT32() > 0;
}

int MySQLManager::GetTableRecordCountFast( string tableName )
{
	//it takes 1.2 seconds for SELECT COUNT(*) to work on a 6.5 million size player database.  Let seth do his own way for better speed
	
	int min = 0;
	int max = 1;

	while (DoesTableRecordExistFast(tableName, max))
	{
		min = max;
		max *= 2;
	}

	if (min <= 2) return max; //it's empty, 1, or 2

	max--; //actually one less

	//LogMsg("Well, we know the correct number is between %d and %d at least.", min, max);
	
	int lastMin = 0;
	int lastMax = 0;

	while (max != min+1 && !(lastMin == min && lastMax == max)) //the lastmin/max stuff is probably not needed, but it avoids a full freeze if I made a mistake
	{
		//LogMsg("Checking %d and %d", min, max);
		lastMin = min;
		lastMax = max;

		if (DoesTableRecordExistFast(tableName, min+ ( (max-min)/2)))
		{
			//higher
			min = min+ ((max-min)/2);
		} else
		{
			//lower
			max = (min+ ((max-min)/2))-1;
		}
		
	}
	
	return min;
}