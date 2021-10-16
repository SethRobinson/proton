#include "PlatformPrecomp.h"
#include "misc_util.h"
#include "util/MiscUtils.h"
void strchar(char *string, char ch)
/* This acts in the same way as strcat except it combines a string and
a single character, updating the null at the end. */
{
	int last;
	last=strlen(string);
	string[last]=ch;
	string[last+1]=0;
}


void dink_decompress( unsigned char *in, char * destBuf )
{

	const int stackSize = 2*1024;
	unsigned char stack[stackSize], pair[128][2];
	int c, top = 0;
	memset(stack, 0, stackSize);
	memset(pair, 0, 128*2);

	int outputSize = 0;

	c = *in; in++;

	if (c > 127)
	{
		//read optional pair count and pair table
		int readCount = (c-128)*2;
		memcpy(&pair,in, readCount );
		in += readCount;
	}
	else
	{
		if (c == '\r') c = '\n';
		if (c == 9) c = ' ';

		strchar(destBuf,c);
	}
	//    putc(c,out);

	for (;;)
	{

		/* Pop byte from stack or read byte from file */
		if (top)
			c = stack[--top];
		else
		{
			if ((c = *in) == 0) break;
			in++;
		}
		
		/* Push pair on stack or output byte to file */
		if (c > 127)
		{
			if (top >= stackSize )
			{
#ifdef _DEBUG
				LogMsg("Malformed .d file, can't read it.  Would overwrite random memory on the old Dink versions.");
				LogMsg("Decompressed to %d bytes", outputSize);
#endif
				destBuf[outputSize] = 0;
				return;
			}
			stack[top++] = pair[c-128][1];
			stack[top++] = pair[c-128][0];
		}
		else
		{
			if (c == '\r') c = '\n';
			if (c == 9) c = ' ';

			strchar(destBuf,c);//     putc(c,out);
			outputSize++;
			}
	}

	destBuf[outputSize] = 0;
#ifdef _DEBUG
//LogMsg("Decompressed to %d bytes", outputSize);
#endif
}

//this legacy code is very stupid, but I'll leave it for now, it works, whatever
void decompress_nocomp (byte *in, char destBuf[])
{
	const int stackSize = 2*1024;
	unsigned char stack[stackSize], pair[128][2];
	
	memset(stack, 0, stackSize);
	int c, top = 0;

	int outputSize = 0;

	c = *in; in++;

	if (0)
	{
		//read optional pair count and pair table
		int readCount = (c-128)*2;
		memcpy(&pair,in, readCount );
		in += readCount;
	}
	else
	{
		if (c == '\r') c = '\n';
		if (c == 9) c = ' ';

		strchar(destBuf,c);
	}
	//    putc(c,out);

	for (;;)
	{

		/* Pop byte from stack or read byte from file */
		if (top)
			c = stack[--top];
		else
		{
			if ((c = *in) == 0) break;
			in++;
		}

		/* Push pair on stack or output byte to file */
		if (0)
		{
			if (top >= stackSize )
			{
#ifdef _DEBUG		
				LogMsg("Malformed .c file, can't read it.  Would overwrite random memory on the old Dink versions.");
				destBuf[outputSize] = 0;
#endif
				return;
			}
			stack[top++] = pair[c-128][1];
			stack[top++] = pair[c-128][0];
		}
		else
		{
			if (c == '\r') c = '\n';
			if (c == 9) c = ' ';

			strchar(destBuf,c);//     putc(c,out);
			outputSize++;
		}
	}

	destBuf[outputSize] = 0;
#ifdef _DEBUG
	//LogMsg("Decompressed to %d bytes", outputSize);
#endif
}


/*


void decompress (FILE *in, char destBuf[])
{
const int stackSize = 512*1024;
unsigned char stack[stackSize], pair[128][2];
int c, top = 0;

int outputSize = 0;


if ((c = getc(in)) > 127)
fread(pair,2,c-128,in);
else
{
	if (c == '\r') c = '\n';
	if (c == 9) c = ' ';

	strchar(destBuf,c);
}
//    putc(c,out);

for (;;) {


	if (top)
		c = stack[--top];
	else if ((c = getc(in)) == EOF)
		break;

	
	if (c > 127)
	{
		if (top >= stackSize )
		{
			LogMsg("Malformed .d file, can't read it.  Would overwrite random memory on the old Dink versions.");
			destBuf[outputSize] = 0;
			return;
		}
		stack[top++] = pair[c-128][1];
		stack[top++] = pair[c-128][0];
	}
	else
	{
		if (c == '\r') c = '\n';
		if (c == 9) c = ' ';

		strchar(destBuf,c);//     putc(c,out);
		outputSize++;
	}
}

destBuf[outputSize] = 0;
#ifdef _DEBUG
//LogMsg("Decompressed to %d bytes", outputSize);
#endif
}


void decompress_nocomp (FILE *in, char destBuf[])
{
	//let's do it, only this time decompile OUR style

	unsigned char stack[16], pair[128][2];
	short c, top = 0;

	if ((c = getc(in)) > 255)
		fread(pair,2,c-128,in);
	else
	{
		if (c == '\r') c = '\n';
		if (c == 9) c = ' ';

		strchar(destBuf,c);
	}
	//    putc(c,out);

	for (;;) {

	if (top)
			c = stack[--top];
		else if ((c = getc(in)) == EOF)
			break;

		if (c > 255) {
			stack[top++] = pair[c-128][1];
			stack[top++] = pair[c-128][0];
		}
		else
		{
			if (c == '\r') c = '\n';
			if (c == 9) c = ' ';

			strchar(destBuf,c);//     putc(c,out);
		}
	}
}

*/

void replace(const char *this1, char *that, char *line)
{

	char hold[500];
	char thisup[512],lineup[512];
	int u,i;
	int checker;

start:

	strcpy(hold,"");
	strcpy(lineup,line);
	strcpy(thisup,this1);
	ToUpperCase(lineup);
	ToUpperCase(thisup);
	if (strstr(lineup,thisup) == NULL) return;
	checker = -1;
	strcpy(hold,"");
	for (u = 0; u < strlen(line); u++)
	{
		if (checker > -1)
		{
			if (toupper(line[u]) == toupper(this1[checker]))
			{
				if (checker+1 == strlen(this1))
				{
doit:
					u = u - strlen(this1);
					u++;
					for (i = 0; i < u; i++) hold[i] = line[i];
					for (i = 0; i < strlen(that); i++) hold[(u)+i]=that[i];
					hold[strlen(that)+u] = 0;
					for (i = 0; i < (strlen(line)-u)-strlen(this1); i++)
					{
						hold[(u+strlen(that))+i] = line[(u+strlen(this1))+i];
					}
					hold[(strlen(line)-strlen(this1))+strlen(that)] = 0;
					strcpy(line,hold);
					goto start;
				}
				checker++;
			} else { checker = -1;    }
		}
		if( checker == -1)
		{
			if (toupper(line[u]) == toupper(this1[0]))
			{
				checker = 1;
				if (strlen(this1) == 1) goto doit;
			}
		}
	}
}

bool separate_string (const char str[255], int num, char liney, char *return1) 
{
	int l;
	int k;
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

/*
	char s[2];
	s[0] = 9; //tab
	s[1] = 0; //null
	replace(s,"",return1); //Take the /n off it.
*/
	return(false);

done:

	if (l < num)  strcpy(return1,"");

	replace("\n","",return1); //Take the /n off it.

	//Msg("Took %s and turned it to %s.",str, return1);
	return(true);
}
