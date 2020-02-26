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
	string _suffix(".Huff");
	ofstream fout(getFileName(FilePath,_suffix), ios::binary);
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
		totalNum += n;//文件总字节数
		for (size_t i = 0; i < n; i++) {
			info[buffer[i]]._chCount++;
		}
	} while (n > 0);
}

//构建哈夫曼树
void FileHandler::buildHuffmanTree() {
	HuffmanTree<CharInfo> tree(info);
	buildCode(tree.GetRoot(), "");
}

//前序遍历huffman树，并生成编码,将节点信息存入到对象中
void FileHandler::buildCode(HuffmanTreeNode<CharInfo>* root, string code) {
	if (root == NULL)return;
	//叶子节点
	if (root->_left == NULL && root->_right == NULL) {
		root->_weight._strCode = code;
		//将每个字符和对应的编码存入codemap
		codemap.insert(make_pair(root->_weight._character, root->_weight._strCode));
		//printf("0x%X === %lld次 === 编码为 %s\n", root->_weight._character, root->_weight._chCount, root->_weight._strCode.c_str());
	}
	buildCode(root->_left, code + '0');
	buildCode(root->_right, code + '1');
}

//写入文件后缀和编码信息
void FileHandler::writeFileHeaderInfo(ofstream& fout, string& path) {
	//总字节数
	fout.write((char*)&totalNum, 8);
	//写入文件后缀
	string suffix = getSuffix(path);
	//文件后缀的字节数
	short suffixLen = suffix.length();
	fout.write((char*)&suffixLen, sizeof(short));
	//文件后缀
	fout.write(suffix.c_str(), suffixLen);
	_encodeNum += suffixLen;
	//写入编码个数
	short count = codemap.size();
	fout.write((char*)&count, sizeof(short));
	_encodeNum += 12;
	//写入字符和对应频次
	map<char, string>::iterator it;
	long long _count = 0;
	unsigned char code;
	for (it = codemap.begin();it != codemap.end();it++) {
		//写入字符
		code = it->first;
		fout.put(code);
		//出现频次，long long 8byte -256个编码的话总共占2kbyte
		_count = info[code]._chCount;
		fout.write((char*)&_count, 8);
		_encodeNum += 9;
	}
}

//写入压缩数据
void FileHandler::buildFile(ofstream& fout, string& path) {
	printf("开始写入文件信息\n");
	//写入文件huffman信息
	writeFileHeaderInfo(fout, path);
	printf("开始收集并写入编码\n");

	ifstream fin(path, ios::binary);
	if (!fin.is_open()) {
		cout << "Error！File is not exist" << endl;
		return;
	}

	//length编码的总长
	int length=0;
	int i, j, gcount = 0;//每次读取的字节数
	unsigned char buff[256];//字节流缓冲区
	unsigned char out_c, tmp_c;
	string code, out_string;//转换后的编码;编码后的数据
	map<char, string>::iterator it;

	do {

		//将数据转换为二进制字符串
		while (length < 1024 && (!fin.eof())) {
			fin.read((char*)buff, 256);
			gcount = fin.gcount();

			if (gcount > 0) {
				for (i = 0;i < gcount;i++) {
					it = codemap.find(buff[i]);
					if (it != codemap.end()) {
						code += it->second;
						length = code.length();
					}
					else {
						printf("Can't find the huffman code of character %X\n", buff[i]);
						printf("error!存在字符无对应编码！\n");
						exit(1);//推出程序
					}
				}
			}
		}

		if (length > 0)
		{
			out_string.clear();
			if (length > 8) {
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
				_encodeNum += out_string.length();
				code = code.substr(i, length - i);//i位开始，长度为length-i
				length = code.length();
			}

			if (fin.eof() && length < 8) {
				//小于等于8
				char n = 8 - length;//补0个数
				char* zero = new char[n + 1];
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
				_encodeNum += 1;
			}
		}
	} while (!fin.eof());
	printf("文件共%lld个字节\n", totalNum);
	printf("压缩后为%lld个字节\n", _encodeNum);
	float rate = _encodeNum*1.0/totalNum*100;
	printf("压缩率为%.2f\%%", rate);
	fin.close();
}

void FileHandler::uncompress(string& FilePath) {
	ifstream fin(FilePath, ios::binary);
	if (!fin.is_open()) {
		cout << "Error！File is not exist" << endl;
		return;
	}
	//解析头文件信息
	analysisFileHeaderInfo(fin);
	ofstream fout(getFileName(FilePath, suffix), ios::binary);
	if (!fout.is_open()) {
		cout << "Error！File can not be writed" << endl;
		fin.close();
		return;
	}
	uncompressData(fin,fout);
	fin.close();
	fout.close();
}

//解析文件头的huffman编码
void FileHandler::analysisFileHeaderInfo(ifstream& fin) {
	char _suffix[10], code[256];
	short count, suffixLen;//编码数
	unsigned char c;//字符
	long long _count = 0;//频数

	fin.read((char*)&totalNum, 8);
	fin.read((char*)&suffixLen, 2);
	fin.read(_suffix, suffixLen);
	_suffix[suffixLen] = '\0';
	suffix = _suffix;//读取文件名后缀

	fin.read((char*)&count, sizeof(short));
	printf("总编码数%d\n", count);

	for (int i = 0; i < 256; ++i)
		info[i]._character = i;

	for (int i = 0;i < count;i++) {
		fin.read((char*)&c,1);
		fin.read((char*)&_count, 8);
		info[c]._character = c;
		info[c]._chCount = _count;
	}
	printf("完成编码收集");
}

//解析数据
void FileHandler::uncompressData(ifstream& fin, ofstream& fout) {
	unsigned char in_char;
	long long writen_len = 0;//记录读取的文件长度
	HuffmanTree<CharInfo> tree(info);
	HuffmanTreeNode<CharInfo>* temp = tree.GetRoot();
	while (1) {
		fin.read((char*)&in_char, 1);
		for (int i = 0; i < 8; i++)
		{
			if (in_char & 128)//128 =binary=>1000 0000 ,与运算获取in_char最高位的取值
				temp = temp->_right;
			else
				temp = temp->_left;

			if (temp->_left == 0 && temp->_right == 0)
			{
				fout.write((char*)&temp->_weight._character, 1);
				++writen_len;
				if (writen_len == totalNum) break;
				temp = tree.GetRoot();
			}
			in_char <<= 1;
		}
		if (writen_len == totalNum) break;
	}
}

//获取文件名
string FileHandler::getFileName(string& FilePath,string &suffix)
{
	size_t begin = 0;
	size_t end = FilePath.find_last_of(".");
	return FilePath.substr(begin, end)+ suffix;
}

//获取文件后缀
string FileHandler::getSuffix(string& FilePath) {
	size_t end = FilePath.length();
	size_t begin = FilePath.find_last_of(".");
	return FilePath.substr(begin, end);
}