#include <iostream>
#include <string>
#include "huffman.h"
using namespace std;

string string2charArr(string& _str) {
	int size = _str.size(), i, j;
	unsigned char tmp_c, out_c;
	string out_str;
	char* out;

	if (size <= 8) {
		out_c = 0;
		for (i = 0;i < size;i++) {
			if ('0' == _str[i])
				tmp_c = 0;
			else
				tmp_c = 1;
			out_c += tmp_c << (size-1 - i);
		}
		out_str += out_c;
		return out_str;
	}
	else {
		//一个循环写一个字节
		for (i = 0; i + 7 < size; i += 8)
		{
			out_c = 0;
			for (j = 0; j < 8; j++)
			{
				if ('0' == _str[i + j])
					tmp_c = 0;
				else
					tmp_c = 1;
				out_c += tmp_c << (7 - j);
			}
			out_str += out_c;
		}
		return out_str;
	}
}

int main() {
	FileHandler fh;
	string path = "d:/hexo.txt";
	fh.compress(path);
	
	


	return 0;
}