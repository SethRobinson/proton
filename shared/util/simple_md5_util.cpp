#include "simple_md5_util.h"
#include "MiscUtils.h"
using namespace std;

std::string GetMD5CheckSumAsString(std::string inputData)
{

	if (inputData.empty())
		return ""; //uhh... how can we get the md5 of a blank string?

	md5_state_t state;
	md5_byte_t digest[16];
	md5_init(&state);
	md5_append(&state, (const md5_byte_t *)&inputData[0], (int)inputData.size());
	md5_finish(&state, digest);

	string tmp;
	tmp.resize(16);
	memcpy(&tmp[0], digest, 16); 	//trust me...

	return HexToString(tmp);
}