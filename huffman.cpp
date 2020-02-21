#include "huffman.h"
#include <fstream>
#include <iostream>
#include <strstream>
using namespace std;

void FileHandler::compress(string& FilePath) {
	ifstream fin(FilePath, ios::binary);
	if (!fin.is_open()) {
		cout << "Error！File is not exist" << endl;
		return;
	}
	_scan(fin);
	buildHuffmanTree();//构建huffman树和编码
	ofstream fout(getFileName(FilePath), ios::binary);
	if (!fout.is_open()) {
		cout << "Error！File can not be writed" << endl;
		fin.close();
		return;
	}
	buildFile(fout,FilePath);//写入文件的huffman编码等信息
	fin.close();
	fout.close();
}

//统计频次
void FileHandler::_scan(ifstream& fin) {
	unsigned char buffer[1024];
	//填充字符
	for (int i = 0; i < 256; ++i)
	{
		info[i]._character = i;
	}

	size_t n;
	do {
		fin.read((char*)buffer, 1024);
		n = fin.gcount();
		totalNum += n;
		for (size_t i = 0; i < n; i++) {
			info[buffer[i]]._chCount++;
		}
	} while (n > 0);
}

//构建哈夫曼树
void FileHandler::buildHuffmanTree() {
	HuffmanTree<CharInfo> tree(info);
	buildCode(tree.GetRoot(),"");
}

//前序遍历huffman树，并生成编码,将节点信息存入到对象中
void FileHandler::buildCode(HuffmanTreeNode<CharInfo>* root, string code) {
	if (root == NULL)return;
	//叶子节点
	if (root->_left == NULL && root->_right == NULL) {
		root->_weight._strCode = code;
		//将每个字符和对应的编码存入codemap
		codemap.insert(make_pair(root->_weight._character, root->_weight._strCode));
	}
	buildCode(root->_left,code+'0');
	buildCode(root->_right,code+'1');
}

//获取文件名
string FileHandler::getFileName(string& FilePath)
{
	size_t begin = 0;
	size_t end = FilePath.find_last_of(".");
	return FilePath.substr(begin, end)+".Huff";
}


string lltoString(long long t)
{
	std::string result;
	std::strstream ss;
	ss << t;
	ss >> result;
	return result;
}

//const char* string2charArr(string& _str) {
//	int size = _str.size(),i,j;
//	unsigned char tmp_c, out_c;
//	string out_str;
//	
//	if (size<=8) {
//		out_c = 0;
//		for (i = 0;i < size;i++) {
//			if ('0' == _str[i])
//				tmp_c = 0;
//			else
//				tmp_c = 1;
//			out_c += tmp_c << (7 - i);
//		}
//		out_str += out_c;
//		return out_str.c_str();
//	}
//	else {
//		//一个循环写一个字节
//		for (i = 0; i + 7 < size; i += 8)
//		{
//			out_c = 0;
//			for (j = 0; j < 8; j++)
//			{
//				if ('0' == _str[i + j])
//					tmp_c = 0;
//				else
//					tmp_c = 1;
//				out_c += tmp_c << (7 - j);
//			}
//			out_str += out_c;
//		}
//		return out_str.c_str();
//	}
//}

//写入文件构建的huffman编码
void FileHandler::writeFileHeaderInfo(ofstream& fout) {
	//写入总编码数
	short count = codemap.size();
	fout.write((char*)&count,sizeof(short));
	//写入字符对应编码
	map<char, string>::iterator it;
	for (it = codemap.begin();it != codemap.end();it++) {
		//写入字符
		fout.put(it->first);
		//写入编码长度
		short codeLen = it->second.length();
		fout.write((char*)&codeLen, sizeof(short));
		//写入编码
		//如果直接写入字符串
		fout.write(it->second.c_str(), it->second.length());
	}
}

//解析文件头的huffman编码
void FileHandler::analysisFileHeaderInfo(ifstream& fin) {
	short count;//编码数
	char c;//字符
	short codeLen;//编码长度
	char code[256];//编码
	codemap.clear();
	if (fin.is_open()) {
		fin.read((char*)&count, sizeof(short));
		printf("总编码数%d\n", count);
		for (int i = 0;i < count;i++) {
			if (i == 192) {
				printf("waiting");
			}
			fin.get(c);
			printf("%d <==> %c===>", i + 1, c);
			fin.read((char*)&codeLen, sizeof(short));
			fin.read(code, codeLen);
			code[codeLen] = '\0';
			string a = code;
			printf("%s\n", code);
			codemap.insert(make_pair(c, a));
		}

		char buff;
		fin.get(buff);//再读一个字节就读到文件结束标志了
		int p = fin.gcount();
		if (fin.eof()) {
			printf("finished");
		}
	}
	fin.close();
}

string charArr2str(char* c, int len) {
	string data;
	unsigned char k = 0x80;
	for (int i = 0;i < len;i++) {
		for (int j = 0; j < 8; j++, k >>= 1) {
			if (c[i] & k) {
				data += '1';
			}
			else {
				data += "0";
			}
		}
		k = 0x80;
	}
	return data;
}



//压缩文件
void FileHandler::buildFile(ofstream &fout,string &path) {
	//写入文件huffman信息
	writeFileHeaderInfo(fout);
	unsigned char c_char;
	ifstream fin(path);
	if (!fin.is_open()) {
		cout << "Error！File is not exist" << endl;
		return;
	}

	//length编码的总长
	int length,i,j;
	char in_char;
	unsigned char out_c, tmp_c;
	string code,out_string;
	map<char, string>::iterator it;
	bool batchflag = false;//批处理标志位
	
	do {
		//每次读取一个字节，并找到对应编码将编码不断追加直到大于1024
		fin.get(in_char);

		if (fin.eof()) {
			length = code.length();
			if (length >0) {
				if (length >= 8) {
					out_string.clear();
					//将huffman的01编码以二进制流写入到输出文件
					for (i = 0; i + 7 < length; i += 8)
					{
						out_c = 0;
						for (j = 0; j < 8; j++)
						{
							if ('0' == code[i + j])
								tmp_c = 0;
							else
								tmp_c = 1;
							out_c += tmp_c << (7 - j);
						}
						out_string += out_c;
					}
					fout.write(out_string.c_str(), out_string.length());
					code = code.substr(i, length - i);//i位开始，长度为length-i
					length = code.length();
				}

				if (length!=0) {
					short n = 8 - length;//补0个数
					char* zero = new char[n+1];
					zero[n] = '\0';
					fill(zero, zero + n, '0');
					string s(zero);
					code.append(zero);
					out_c = 0;
					for (j = 0; j < 8; j++)
					{
						if ('0' == code[j])
							tmp_c = 0;
						else
							tmp_c = 1;
						out_c += tmp_c << (7 - j);
					}
					fout.put(out_c);
					fout.write((char*)&n,sizeof(short));
				}
			}
			break;
		}

		it = codemap.find(in_char);
		if (it != codemap.end())
			code += it->second;
		else
		{
			//十六进制数形式输出整数
			//找不到编码退出程序
			printf("Can't find the huffman code of character %X\n", in_char);
			exit(1);//推出程序
		}

		length = code.length();
		
		if (length > 1024)
		{
			out_string.clear();
			//将huffman的01编码以二进制流写入到输出文件
			for (i = 0; i + 7 < length; i += 8)
			{
				// 每八位01转化成一个unsigned char输出
				// 不使用char，如果使用char，在移位操作的时候符号位会影响结果
				// 另外char和unsigned char相互转化二进制位并不变
				out_c = 0;
				for (j = 0; j < 8; j++)
				{
					if ('0' == code[i + j])
						tmp_c = 0;
					else
						tmp_c = 1;
					out_c += tmp_c << (7 - j);
				}
				out_string += out_c;
			}
			fout.write(out_string.c_str(), out_string.length());
			code = code.substr(i, length - i);
		}
	} while (!fin.eof());
	fout.close();
}
