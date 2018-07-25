/* Copyright (C) Seth A. Robinson, 2003. 
 * All rights reserved worldwide.
 *
 * This software is provided "as is" without express or implied
 * warranties. You may freely copy and compile this source into
 * applications you distribute provided that the copyright text
 * below is included in the resulting source code, for example:
 * "Portions Copyright (C) Seth A. Robinson, 2003"
 */


#include "all.h"
#include "cassert"
#include <process.h>
#include <direct.h>
#include "time.h"
#include <climits>
#include <cmath>


long GetFileSize(const char *p_fname)
{
    
    //let's get the file size
    FILE *fp_temp = fopen(p_fname, "rb");
    if (!fp_temp)
    {
        LogError("Something went terribly wrong while reading from %s.", p_fname);
        return 0;
    }
    fseek(fp_temp,0,SEEK_END);
    
    long l_filesize = ftell(fp_temp);
    fclose(fp_temp);
    
    assert(l_filesize);
    
    return l_filesize;
} 


byte * LoadFileIntoBuffer(const char *pFilename, int *pFileSizeOut)
{
    byte *pDataBytes = NULL;
    
    long fileSize = GetFileSize(pFilename);

    if (fileSize == 0) return NULL;

    //init memory to hold the entire bitmap
    pDataBytes = new byte[fileSize+1]; //for an extra NULL, just in case it's useful later
    if (!pDataBytes) return NULL;
    pDataBytes[fileSize]=0; //add the extra NULL at the end

    FILE *fp_temp = fopen(pFilename, "rb");
    if (!fp_temp)
    {
        LogError("Something went terribly wrong while reading from %s.", pFilename);
        SAFE_DELETE_ARRAY(pDataBytes);
        return NULL;
    }
    
    fread(pDataBytes, fileSize, 1, fp_temp);
    fclose(fp_temp);

    if (pFileSizeOut) *pFileSizeOut = fileSize;

    return pDataBytes;
}


  
char *float_to_money( double num, char *buf, int dec)
{
      char tmp[256];
      int bf = 0, cm = 0, tm = 9 - dec + (!dec);

      sprintf(tmp, "%.9f", num);
      strrev(tmp);
      if(dec)
      {
            while( (buf[bf++] = tmp[tm++]) != '.')
                  ;
            while((buf[bf++] = tmp[tm++]) != 0)
            {
                  if(++cm % 3 == 0 && tmp[tm])
                        buf[bf++] = ',';
            }
      
      return strrev(buf);
      } else
      {
           //  while( (buf[bf++] = tmp[tm++]) != '.')
            //      ;
            while((buf[bf++] = tmp[tm++]) != 0)
            {
                  if(++cm % 3 == 0 && tmp[tm])
                        buf[bf++] = ',';
            }
      return strrev(buf);
   
      }
     return NULL;
}


#ifndef _USRDLL   //.dll's don't seem to like this, well, at least smartmute's doesn't

//launch a URL in the default web browser IN A NEW WINDOW
DWORD LaunchURL(LPTSTR addy){
/* while testing leave in all the ZeroMemory lines even though
 * they probably aren't all necessary. oh well
 */
	TCHAR lpBrowser[512];
	TCHAR holder[512];
	TCHAR addytype[512];
	DWORD dwLen = 500;
	DWORD dwType;
	DWORD urltype=0;
	HKEY hKey;
	LPTSTR lpPathValue;
	ZeroMemory(lpBrowser, 512 * sizeof(TCHAR));
	
	ZeroMemory(addytype, 512 * sizeof(TCHAR));
	ZeroMemory(holder, 512 * sizeof(TCHAR));	_tcsncpy(holder, addy, 4);
	if(_tcsicmp(holder, _T("ftp:")) == 0)
	{
		_tcscpy(addytype, _T("ftp\\shell\\open\\command"));
		urltype = 2;
	}
	else
	{
		ZeroMemory(holder, 512 * sizeof(TCHAR));	_tcsncpy(holder, addy, 7);
		if(_tcsicmp(holder, _T("mailto:")) == 0)
		{
			return (DWORD) ShellExecute(NULL, NULL, addy, NULL, NULL, SW_SHOWNORMAL);
		}
		else
		{
			_tcscpy(addytype, _T("http\\shell\\open\\command")); // default browser, this will run most often
		}
	}

	lpPathValue = (LPTSTR)GlobalAlloc(GMEM_FIXED, 500);
	
	if(RegOpenKeyEx(HKEY_CLASSES_ROOT,addytype, 0,KEY_QUERY_VALUE, &hKey ) == ERROR_SUCCESS)
	{
		if(RegQueryValueEx(hKey,_T(""),0,&dwType,(BYTE *)lpPathValue, &dwLen) ==  ERROR_SUCCESS)
		{
			ZeroMemory(holder, 512 * sizeof(TCHAR));
		
            if (lpPathValue[0] == '\"')
            {
            //let's get rid of the quote
            _tcscpy(holder, lpPathValue+1);
           _tcsncpy(lpBrowser, holder, _tcsrchr(holder, '\"') - holder);
          
            } else
            {
                //just copy the damn thing over
                _tcscpy(lpBrowser, lpPathValue);
            }
           //let's remove anything after the .exe part
           
           TCHAR *p_pos = _tcsrchr(lpBrowser, '.');
           if (p_pos)
           {
              p_pos[4] = 0; //add null to truncate it

           }   else
           {
#ifndef _NOLOGMSG
               LogMsg(_T("Don't understand format of your default browser.  Let Seth know.  Disable Open in new window for now."));
#endif
           }

		}
		else _tcscpy(lpBrowser, _T("iexplore"));
	}
	else _tcscpy(lpBrowser, _T("iexplore"));
	RegCloseKey(hKey);
	GlobalFree(lpPathValue);
   
    return (DWORD)ShellExecute(NULL, _T("open"), lpBrowser, addy, NULL, SW_SHOWDEFAULT);
}

#endif

//just like strstr but case insensitive.  Slow as hell.
TCHAR * stristr(const TCHAR text[], const TCHAR search[])
{
    TCHAR *p_text = NULL;
    TCHAR *p_search = NULL;

    p_text = new TCHAR[_tcslen(text)+1];
    p_search = new TCHAR[_tcslen(search)+1];

    _tcscpy(p_text, text);
    _tcscpy(p_search, search);

    //lowercase everything
    _tcslwr(p_text);
    _tcslwr(p_search);

    TCHAR *p_result = _tcsstr(p_text, p_search);

    //free the mem we just wasted
    SAFE_DELETE_ARRAY(p_text);
    SAFE_DELETE_ARRAY(p_search);

    return p_result;
}


BOOL LaunchControlPanelApplet(TCHAR * pApplet)
{
    PROCESS_INFORMATION pi;
    STARTUPINFO si;
    TCHAR CPLApplet[50];

    si.cb = sizeof(STARTUPINFO);
    si.lpReserved = NULL;
    si.lpDesktop = NULL;
    si.lpTitle = NULL;
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_SHOWNORMAL;
    si.cbReserved2 = 0;
    si.lpReserved2 = NULL;

    _stprintf(CPLApplet,_T("CONTROL.EXE %s"), pApplet);

    if (CreateProcess(NULL, CPLApplet, NULL, NULL,
        FALSE, NORMAL_PRIORITY_CLASS,
        NULL, NULL, &si, &pi))
    {
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        return TRUE;
    }

    return FALSE;
}



void ConvertLastErrorToString(TCHAR * szDest, int nMaxStrLen)
{
  LPVOID lpMsgBuf;
  FormatMessage( 
    FORMAT_MESSAGE_ALLOCATE_BUFFER | 
    FORMAT_MESSAGE_FROM_SYSTEM | 
    FORMAT_MESSAGE_IGNORE_INSERTS,
    NULL,
    GetLastError(),
    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
    (LPTSTR) &lpMsgBuf,
    0,
    NULL 
  );
  
#ifdef _UNICODE
  wcsncpy(szDest, reinterpret_cast<TCHAR*>(lpMsgBuf), nMaxStrLen);

#else
  strncpy(szDest, reinterpret_cast<TCHAR*>(lpMsgBuf), nMaxStrLen);

#endif

  LocalFree( lpMsgBuf );
}

void LogLastError()
{
    TCHAR t_temp[256];
    ConvertLastErrorToString(t_temp, 256);
    LogError(t_temp);
}

int CountLinesInTextFile( TCHAR * file)
{
    int count = 0;
    TCHAR line[1024];
    TCHAR *p_line;
    FILE *stream;  
    
    if (!exist(file)) return(0);
    
    if( (stream = _tfopen( file, _T("rb") )) != NULL )   
    {
        while(1)
        {
            if( (p_line = _fgetts( line, 1024, stream )) == NULL) 
            {
                break;
            }
            else    
            {
                if (p_line != line) break;
                // LogMsg(L"read in %s.", p_line);
               
                count++;
            }
        }
    }
     
    fclose( stream );  

return(count);
}

 
 //no single line can be more than 1023 bytes.
//unicode only, but can be used from ANSI builds (used in ANSI build of Toolfish)
 bool TrimTextFile(TCHAR *p_filename, int i_number_of_lines)
 {
     WCHAR line[1024];
     WCHAR *p_line;
     
     //first count how many lines there are
     int i_lines = CountLinesInTextFile(p_filename);
    // LogMsg("Counted %d lines.", i_lines);
     
     if (i_lines <= i_number_of_lines)
     {
         //no need to do anything, we're within the required range already.
         return true; //no error
     }

     int i_line_to_start_at = (i_lines - i_number_of_lines);

     TCHAR * p_temp_filename = _T("trim_file.tmp");
     _tunlink(p_temp_filename);
     _trename( p_filename, p_temp_filename);
     
     //now slowly copy back the part we needed
    
      FILE *fp_dest;

        if (! (fp_dest = _tfopen( p_filename, _T("wb"))))
        {
           LogError(_T("Error opening file in TrimtextFile"));
           return false;
        }
        
        //write the unicode marker
        CHAR *p_st_unicode = "ÿþ";
        fwrite(p_st_unicode, 2, 1, fp_dest);
        FILE *fp;  
        int i_line_counter = 0;
        if( (fp = _tfopen( p_temp_filename, _T("rb") )) == NULL )   
        {
            LogError(_T("Couldn't open file in TrimtextFile"));
            fclose(fp_dest);
            return false;
        }

       //   fseek(fp, 4, SEEK_CUR); //skip the unicode marker

       while (p_line = fgetws( line, 1024, fp )) 
          {

              if (p_line != line) break;
              if (++i_line_counter >= i_line_to_start_at)   
              {
                  
                  //copy this to our new file
              //   LogMsg(L"writing %s.", line) ;
                  fputws(line, fp_dest);  
              }
           }
       
       //all done
       fclose(fp);
       fclose(fp_dest);
       _tunlink(p_temp_filename);
       return true; //success
 }


unsigned int compute_checksum_from_string(char st_dir[])
{
    unsigned int ui_temp = 0;
    int i_length = strlen(st_dir);
    
    for (int i=0; i < i_length; i++)
    {
        ui_temp += 128+(i*i*(char)st_dir[i]);
    }
    return ui_temp;
}


unsigned long ul_compute_checksum_from_string(char *str)
{
   unsigned long hashNumber;
     //For each character of the entire string
     // (until the null character is reached)
     // left shift the previous iteration result
     // by 2 and XOR it with the current character.
     
    for(hashNumber = 0; *str; )
       hashNumber = (hashNumber << 2) ^ *str++;
    return(hashNumber); 
}




/* 
  compare - Compares two strings, not case sensitive.
*/

bool compare(char *orig, char *comp)
{
    return (strnicmp(orig,comp,INT_MAX) == 0);
}


/* 

  strip_beginning_spaces - Removes whitespaces from the beginning of
  a string.
  Author:  Seth A. Robinson ('97)
  
*/

void strip_beginning_spaces(char *s)
{
    char * h;
    h = s;
    
    if (s[0] != 32) 
    {
        return;
    }
    
    while(h[0] == 32)
    {
        h = &h[1];
    }
    strcpy(s, h);
}


//open a file in read mode

FILE * open_file(const char *fname)
{
    return(fopen( fname, "r" ));    
}

//get next line from a stream (opened with above)

bool get_line_from_file(FILE * fp, char * dest)
{
    if( fgets( dest, 255, fp ) == NULL) return(false); else
        return(true); 
}




/* does file exist? */ //need both versions as I call this with both ANSI and UNICODE strings
//from the same build ...

bool exist(const char * name)
{
    if (name[0] == NULL) return false;
    
    FILE *fp;
    fp = fopen(name, "rb");
    if (!fp)
    {
        return(false);
    }
    
    fclose(fp);
    return(true);
}


bool exist(const WCHAR * name)
{
    if (name[0] == NULL) return false;
    
    FILE *fp;
    fp = _wfopen(name, L"rb");
    if (!fp)
    {
       //	  fclose(fp);
        return(false);
    }
    
    fclose(fp);
    return(true);
}



//copy one file onto the end of another	 (Seth '01)

void append_file(char st_input[], const char st_output[])
{
    
    FILE *          fp_in;
    FILE *          fp_out;
    
    fp_in = fopen(st_input, "rb");
    
    if (!fp_in)
    {
        return;
    }
    
   	if (exist(st_output) == false)
    {
        
        fp_out = fopen(st_output, "wb");
    } else
    {
        fp_out = fopen(st_output, "ab");
    }
    
    if (!fp_out)
    {
        fclose(fp_in);
        return;
    }
    
    const int i_buffer_size = 1000;
    byte buffer[i_buffer_size];
    int i_read = 0;
    
    while (!feof(fp_in))
    {
        i_read = fread(&buffer, 1, i_buffer_size, fp_in);
        fwrite(&buffer, 1,i_read, fp_out);
    }
    
    fclose(fp_in);
    fclose(fp_out);
    
}

//you better hope your string is big enough

bool load_file_into_string(char *st_out, char st_file[], int i_max_size)
{
    FILE *fp;
    
    fp = fopen(st_file, "r");
    
    if (!fp)
    {
        //unable to find file
        return false;
    }
    
    fread(st_out, i_max_size, 1, fp );
    fclose(fp);
    return true;
}



/* Add text adds a line of text to a text file.  It creates it if it doesn't
exist. */

void add_text(const char *tex ,char *filename)
{
    if ( (tex == NULL) || ( filename == NULL) || ( filename[0] == NULL))
    {
        assert(0);
        return;
    }
  
    FILE *          fp = NULL;
    if (strlen(tex) < 1) return;
    if (exist(filename) == false)
    {
        
        fp = fopen(filename, "wb");
        if (!fp)
        {
         return;
        }
        fwrite( tex, strlen(tex), 1, fp);       
        fclose(fp);
        return;
    } else
    {
        fp = fopen(filename, "ab");
        fwrite( tex, strlen(tex), 1, fp);      
        fclose(fp);
    }
}


//replace all instances of szSearch with szReplace in the string

char *replace(char *szSearch, char *szReplace, char *szBuffer)
{
    int iSearchLen;
    char *p;
    char *p1;
    char *szDup;
    
    /* Start searching from the start */
    p1=szBuffer;
    iSearchLen=strlen(szSearch);
    
    /* Loop whilst search string is found in buffer */
    while((p=strstr(p1,szSearch))!=NULL)
    {
        szDup=strdup(szBuffer);
        
        /* Set found position to null */
        *p=0;
        
        /* Append replacement onto buffer */
        strcat(szBuffer,szReplace);
        
        /* Determine end of current replacement for next search */
        p1=szBuffer+strlen(szBuffer);
        
        /* Copy the rest of remaining string back on the end */
        strcat(szBuffer,(p-szBuffer)+iSearchLen+szDup);
        
        free(szDup);
    }
    
    return szBuffer;
}
/* 

  separate_string - Give it a string, a separator symbol and tell it
  which one to return.	  (1 for left most, 2 for after the first seperator and so forth)
  Author:  Seth A. Robinson ('97)
  
  PS:  Yes NOW I know this is spelled wrong... but to support legacy code...
 
*/

bool seperate_string (char str[], int num, char liney, char *return1) 
{
    int l;
    unsigned int k;
    
    l = 1;
    strcpy(return1 ,"");
    
    for (k = 0; k <= strlen(str); k++)
    {
        
        if (str[k] == liney)
        {
            l++;
            if (l == num+1)
                goto done;
            
            if (k < strlen(str)) strcpy(return1,"");
        }
        if (str[k] != liney)
            sprintf(return1, "%s%c",return1 ,str[k]);
    }
    if (l < num)  strcpy(return1,"");
    
    replace("\n","",return1); //Take the /n off it.
    
    return(false);
    
done:
    
    if (l < num)  strcpy(return1,"");
    
    replace("\n","",return1); //Take the /n off it.
    
    return(true);
}



//this one gets one line by number from a text file.  It puts the null
//terminated string into into *dest.

bool get_line_from_file(char file[255], int line_to_get,  char *dest)
{
    int count = 0;
    char line[500];
    
    FILE *stream;  
    
    if (!exist(file)) return false;
    
    if( (stream = fopen( file, "r" )) != NULL )   
    {
        while(1)
        {
            if( fgets( line, 255, stream ) == NULL) 
                goto done;
            else    
            {
                count++;
                
                if (count == line_to_get)
                {
                    //this is it boys, let's remove the carriage return
                    line[strlen(line)-1] = 0;
                    strcpy(dest, line);
                    goto done;
                }
                //figure_out(line);
            }
        }
        
done:
        fclose( stream );  
    } else
    {
        return false;
    }
    return(true);
    
    
}

// get number of lines in a text file
//0 indicates file not found or error

int get_num_lines_from_file( char file[255])
{
    int count = 0;
    char line[500];
    
    FILE *stream;  
    
    if (!exist(file)) return(0);
    
    
    if( (stream = fopen( file, "r" )) != NULL )   
    {
        while(1)
        {
            if( fgets( line, 255, stream ) == NULL) 
                goto done;
            else    
            {
                count++;
                //figure_out(line);
            }
        }
        
done:
        fclose( stream );  
    } else
    {
        return(0);
    }
    return(count);
}

void randomize()
{
    srand( (unsigned)GetTickCount() );
}

int random( long i_max)
{
    return (rand()%i_max);
}



float frandom(float f_max)
{
    return frandom_range(0, f_max);
}

float frandom_range(float min, float max)
{ 
    return ((float)rand() / RAND_MAX) * (max-min) + min;
}

//special hack so this file will compile with the Flexporter plugin SDK
#ifndef _NOLOGMSG
//Random number + a number, starts at 0
int random(long i_max, long i_min)
{
    if (i_max == 0) return 0; //bad info
    return (rand()%i_max)+i_min;
}
#endif

//Random number between a defined range
int random_range(long i_small, long i_large)
{
    if (i_large == 0) return 0;
    if (i_large == i_small) return i_small;
    i_large++; //inclusive of large
   	return (   (((int)(rand()-1) * (i_large-i_small)) / RAND_MAX))  + i_small;
    
    //note, without the -1 from rand() it's possible to get a # 1 higher then
    //the max range.  
}

//legacy...


bool in_range(int i_num, int i_low, int i_high)
{
    if (i_num > i_high) return false;
    if (i_num < i_low) return false;
    return true;
}

bool in_range_float(float i_num, float i_low, float i_high)
{
    if (i_num > i_high) return false;
    if (i_num < i_low) return false;
    return true;
}

 bool in_range_long(long i_num, long i_low, long i_high)
{
    if (i_num > i_high) return false;
    if (i_num < i_low) return false;
    return true;
}

TCHAR * get_path_from_string(const TCHAR * st_in)
{
    static TCHAR st_full[256];
    _tcscpy(st_full, st_in);
    char c_cur = 0;
    
    for (int k=_tcslen(st_full);st_full[k] != '\\'; k--)
    {
        c_cur = k;
    }
    if (c_cur == 0) return st_full; //actually no path was found
    
    st_full[--c_cur] = 0;
    return st_full;
}


//changes/adds a file extension. Do not send a period, it will add it. 	 
void change_file_extension(char *st_filename, char st_extension[100])
{
    ///first find out if there is already a file extension in it
    char *p_index = strchr(st_filename, '.');
    
    if (!p_index)
    {
        //there is no period, let's just add our file extension.
        strcat(st_filename, ".");
        strcat(st_filename, st_extension);
        return; //all done, that was easy
    }
    
    //add it in a slightly different way.
    char st_temp[255];
    sprintf(st_temp, ".%s", st_extension);
    strcpy(p_index, st_temp); //copy this straight to the pointer location we got earlier
}

//changes/adds a file extension. Send ".mp3" to it. 	 
bool file_extension_is(const TCHAR *st_filename, TCHAR * st_extension)
{
    if (!st_filename) return false;
    if (st_filename[0] == 0) return false;
    ///first find out if there is already a file extension in it
    const TCHAR *p_index = _tcschr(st_filename, _T('.'));
    
    if (!p_index)
    {
        return false; //no period at all!
    }
    
    if (_tcsicmp(p_index, st_extension) == 0)
    {
        //match made
        return true;
    }
    return false;
    
}

//this will return a string without the path.  if there was no path it will
//return what it started with.  Returns false if that happens.
bool get_filename_only(TCHAR *p_st_fname_out, const TCHAR * p_st_in)
{
    //walk backwards until we hit a
    for (int i = _tcslen(p_st_in); i >= 0; i--)
    {
        if (p_st_in[i] == '\\')
        {
            //we found the spot that marks where the filename starts
            _tcscpy(p_st_fname_out, &p_st_in[i+1]);
            return true;
        }
        
    }
    
    //failed to find any path, return it as it was
    _tcscpy(p_st_fname_out, p_st_in);
    return false;
}



//should template this.  
bool number_is_close(int i_orig, int i_target, int i_range)
{
    if (i_orig > i_target)
    {
        if (i_orig <= i_target + i_range)
        {
            //close
            return true;
        }
        return false;
    } else
    {
        //less
        if (i_orig >= i_target - i_range)
        {
            return true;
        }
        return false;
    }
    
}



#ifdef _UNICODE
    
 
void switch_to_current_dir()
{
    TCHAR dir_final[MAX_PATH];
    getdir(dir_final);
    //switch to dir run.exe is in
    _wchdir(dir_final);
}


void getdir(TCHAR final[])
{
    //converted to non CString version that spits back path + filename seperately.
    //Using	GetModuleFileName instead of ParamStr, works with Win2000/nt.
    TCHAR dir[MAX_PATH];
    TCHAR path[MAX_PATH];
    GetModuleFileName(NULL, path, MAX_PATH);
    char c_cur = 0;
    
    for (int k=wcslen(path);path[k] != '\\'; k--)
    {
        c_cur = k;
    }
    wcscpy(dir, L"");
    //copy file name
    wcsncat((TCHAR*)&dir, &path[c_cur], wcslen(path)-c_cur);
    path[c_cur] = 0; //truncate
    wcscpy(final, path);
}




void ProcessError(HWND hw, int result, const TCHAR filename[], const TCHAR url[])
{
    if (result > 32)
    { 
        //no error here.
        return;
    }
    
    if (result == ERROR_FILE_NOT_FOUND)
    {
        TCHAR st_text[512];
        wsprintf(st_text, L"Could not find the file %s in the current dir.\n\nPoint your browser to %s instead.\n",
            filename,url);
        MessageBox(hw,st_text, L"Error", MB_OK);
        return;
    }
    
    if (result == SE_ERR_NOASSOC)
    {
        TCHAR st_text[512];
        wsprintf(st_text, L"Your system doesn't know how to handle .URL files.  (pre  Win98?)\n\nPoint your browser to %s instead.\n",
            url);
        MessageBox(hw, st_text, L"Error", MB_OK);
        return;
    }
    
    
      TCHAR st_text[512];
      wsprintf(st_text, L"Your system had an unknown error opening the link.\n\nPoint your browser to %s instead.\n",
        url);
    MessageBox(hw, st_text, L"Error", MB_OK);
}


 //use this function like this:
/*
char st_wildcard[256];
sprintf(st_wildcard, "*.dat");

  char st_temp[255];																		  
	 while (get_files_with_wildard("data\\", st_wildcard, (char*)&st_temp))
     {
     //do something with st_temp, it holds a filename now
     //this will happen for each filename
     }
*/


bool get_files_with_wildard ( TCHAR st_path[], TCHAR st_search_pattern[], TCHAR *st_return)
{
    
    
    
    static HANDLE		dir;
    static WIN32_FIND_DATA	fd;
    static bool b_active = false;
    
    
    TCHAR		*dename;
    
   	//get current dir and save it
    static TCHAR st_old_dir[255];
    
    if (!b_active)
    {
        //first time
        getdir(st_old_dir);
        //first move to the dir
        if (_wchdir(st_path) != 0)
        {
#ifndef _NOLOGMSG
            LogMsg(_T("Error switching to dir %s, can't get files with wildcard."),st_path);
#endif
            b_active = false;
            return 0;
        }
        
        dir = FindFirstFile( st_search_pattern, &fd );
        if( dir == INVALID_HANDLE_VALUE)
        {
            //	log_msg( "Could not open current directory\n" );
            //move back to original dir
            _wchdir(st_old_dir);
            b_active = false;
            return(0);
        }
        
        //do the very first search...    
        b_active = true;
        
    } else
    {
again:
    if( !FindNextFile( dir, &fd ) )
    {
        //done
        FindClose( dir );
        //log_msg("All done finding files.");
        _wchdir(st_old_dir);
        b_active = false;
        return false;
    }
    }
    
    
    if( !(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ) 
    {
        dename = fd.cFileName;
        wcscpy(st_return, dename);
        return true; //keep searching
    }
    
    
    
    //this is not a file that matches the specs, go to the next one..
    goto again;
    
}





#else
   
//NON unicode only, have not converted this yet...

void ProcessError(HWND hw, int result, char filename[255], char url[255])
{
    if (result > 32)
    { 
        //no error here.
        return;
    }
    
    if (result == ERROR_FILE_NOT_FOUND)
    {
        char st_text[255];
        sprintf(st_text, "Could not find the file %s in the current dir.\n\nPoint your browser to %s instead.\n",
            filename,url);
        MessageBox(hw,st_text, "Error", MB_OK);
        return;
    }
    
    if (result == SE_ERR_NOASSOC)
    {
        char st_text[255];
        sprintf(st_text, "Your system doesn't know how to handle .URL files.  (pre  Win98?)\n\nPoint your browser to %s instead.\n",
            url);
        MessageBox(hw, st_text, "Error", MB_OK);
        return;
    }
    
    
    char st_text[255];
    sprintf(st_text, "Your system had an unknown error opening the link.\n\nPoint your browser to %s instead.\n",
        url);
    MessageBox(hw, st_text, "Error", MB_OK);
}


//delete files by wildcard.  Returns number of files deleted

int delete_wildcard ( char st_path[255], char st_search_pattern[255])
{
    
    HANDLE		dir;
    WIN32_FIND_DATA	fd;
    unsigned long	cnt;
    
    char		*dename;
    
   	//get current dir and save it
    char st_old_dir[255];

        if (!_getcwd(st_old_dir, 255))
        {
            LogError("Error getting working directory.");
        }
  
        //first move to the dir
    if (chdir(st_path) != 0)
    {
#ifndef _NOLOGMSG
        LogMsg("Error switching to dir %s, aborting wildcard command.",st_path);
#endif
        return 0;
    }
    
    cnt = 0;
    dir = FindFirstFile( st_search_pattern, &fd );
    if( dir == NULL )
    {
#ifndef _NOLOGMSG
        LogMsg( "Could not open current directory\n" );
#endif
        //move back to original dir
        chdir(st_old_dir);
        return(0);
    }
    
    
    while( 1 ) 
    {
        if( !(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ) 
        {
            cnt++;
            dename = fd.cFileName;
            unlink(dename);				
        }
        if( !FindNextFile( dir, &fd ) ) {
            break;
        }
    }
    
    FindClose( dir );
    //LogMsg("All done finding files.");
    chdir(st_old_dir);
    
    return(cnt);
}

 

void show( HWND g_hWnd, char title[], LPSTR fmt, ... )
{
    char    buff[1024];
    va_list  va;
    va_start(va, fmt);
    //
    // format message with header and show via pop up window
    //
    lstrcpy( buff, "" );
    wvsprintf( &buff[lstrlen(buff)], fmt, va );
    lstrcat( buff, "\r\n" );
    MessageBox(g_hWnd, buff, title, MB_ICONINFORMATION | MB_SETFOREGROUND);
} 


//get full path.  Send something 256 or bigger or may crash.

void all_get_full_path(char *path)
{
    
    GetModuleFileName(NULL, path, 255);
    
    //truncate it before the .exe name starts
    for (int i = strlen(path); i > 0; i--)
    {
        if (path[i] == '\\')
        {
            //this is it
            path[i] = 0;
            return;
        }
    }
    
}


    void getdir(char *dir, char *final)
{
    //converted to non CString version that spits back path + filename seperately.
    //Using	GetModuleFileName instead of ParamStr, works with Win2000/nt.
    char path[255];
    GetModuleFileName(NULL, path, 255);
    char c_cur = 0;
    
    for (int k=strlen(path);path[k] != '\\'; k--)
    {
        c_cur = k;
    }
    strcpy(dir, "");
    //copy file name
    strncat(dir, &path[c_cur], strlen(path)-c_cur);
    path[c_cur] = 0; //truncate
    strcpy(final, path);
    
} 


void switch_to_my_dir()
{
    
    char dir_temp[256], dir_final[256];
    getdir(dir_temp, dir_final);
    //switch to dir run.exe is in
    chdir(dir_final);
}		 


 bool open_file(HWND hWnd, char st_file[])
{
 #ifndef _NOSHELL32
     if (!exist(st_file))
    {
        //file doesn't exist
        char st_temp[500];
        sprintf(st_temp, "Sorry, the file %s does not exist yet.",st_file);
        MessageBox(hWnd, st_temp,
            "Unable to open file", MB_ICONSTOP);
        return false;
        
    }
    int result = (int)ShellExecute(NULL,"open",st_file, NULL,NULL, SW_SHOWDEFAULT  );
    
    //	Msg("Result is %d.",result);
    if ( (result < 32) && (result != 2))
    {
        //big fat error.
        MessageBox(hWnd, "Windows doesn't know how to display this kind of file.  You need to associate it or install a viewer.",
            st_file, MB_ICONSTOP);
        
        return false;
    }
    
    return true;

#else

 LogError("You must remove _NOSHELL32 to build with this in the all.cpp lib.");
  return false;
#endif
}

 
//st_return much be large enough to hold the return FILE.
//just call this over and over until it returns null.

bool get_files_with_wildard ( char st_path[], char st_search_pattern[], char *st_return)
{
    static HANDLE		dir;
    static WIN32_FIND_DATA	fd;
    static bool b_active = false;
    char		*dename;
    
   	//get current dir and save it
    static char st_old_dir[MAX_PATH];
    
    if (!b_active)
    {
        //first time
        
        if (!_getcwd(st_old_dir, MAX_PATH))
        {
            LogError("Error getting working directory.");
        }
        
        //first move to the dir
        if (chdir(st_path) != 0)
        {
#ifndef _NOLOGMSG
            LogMsg("Error switching to dir %s, can't get files with wildcard.",st_path);
#endif
            b_active = false;
            return 0;
        }
        
        dir = FindFirstFile( st_search_pattern, &fd );
        if( dir == INVALID_HANDLE_VALUE)
        {
            //	LogMsg( "Could not open current directory\n" );
            //move back to original dir
            chdir(st_old_dir);
            b_active = false;
            return(0);
        }
        
        //do the very first search...    
        b_active = true;
        
    } else
    {
again:
    if( !FindNextFile( dir, &fd ) )
    {
        //done
        FindClose( dir );
        //LogMsg("All done finding files.");
        chdir(st_old_dir);
        b_active = false;
        return false;
    }
    }
    
  //  if( !(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ) 
    {
        if (compare(fd.cFileName, ".")) goto again; //it's a reserved dir name
        if (compare(fd.cFileName, "..")) goto again; //it's a reserved dir name
        dename = fd.cFileName;
        strcpy(st_return, dename);
        return true; //keep searching
    }
    
    
    //this is not a file that matches the specs, go to the next one..
    goto again;
}
       

void switch_to_current_dir()
{
    char dir_final[256];
    getdir(dir_final);
    //switch to dir run.exe is in
    chdir(dir_final);
}

void getdir(char final[])
{
    //converted to non CString version that spits back path + filename separately.
    //Using	GetModuleFileName instead of ParamStr, works with Win2000/nt.
    char dir[MAX_PATH];
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);
    char c_cur = 0;
    
    for (int k=strlen(path);path[k] != '\\'; k--)
    {
        c_cur = k;
    }
    strcpy(dir, "");
    //copy file name
    strncat((char*)&dir, &path[c_cur], strlen(path)-c_cur);
    path[c_cur] = 0; //truncate
    strcpy(final, path);
}	


TCHAR * GetExeDir()
{
    static TCHAR c_st_dir[256] = "";

    if (c_st_dir[0] == 0)
    {
        //first time, init it
        getdir(c_st_dir);
    }
   return (TCHAR*)&c_st_dir;
}



#endif //end non-unicode