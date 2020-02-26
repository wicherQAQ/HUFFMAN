#pragma once
#include <string>
#include <map>
#include "huffmanTree.h"
using namespace std;

//字符结构体
struct CharInfo {
	CharInfo()
		:_character(0)
		, _chCount(0)
		, _strCode()
	{}
	friend bool operator>(const CharInfo& left, const CharInfo& right)
	{
		return left._chCount > right._chCount;
	}
	friend CharInfo operator+(const CharInfo& left, const CharInfo& right)
	{
		CharInfo tmp;
		tmp._chCount = left._chCount + right._chCount;
		return tmp;
	}
	unsigned char _character;//字符
	long long _chCount;//出现次数
	string _strCode;//编码
};

//文件处理器
class FileHandler {
private:
	//文件中所读取的总字节/字符数，用于解压时的判断（可能因为字节数太大）
	long long totalNum = 0;
	long long _encodeNum = 0;

	HuffmanTreeNode<CharInfo>* root;
	//存储字符和对应编码
	map<char, string> codemap;
	map<string, char> decodemap;
	CharInfo info[256];
	string suffix;

public:
	void compress(string& FilePath);
	void uncompress(string& FilePath);
private:
	//扫描文件,获取编码信息
	void _scan(ifstream& fin);
	//构建Huffman树
	void buildHuffmanTree();
	//构建编码
	void buildCode(HuffmanTreeNode<CharInfo>* root,string code);
	//获取文件名
	string getFileName(string& path, string& suffix);
	string getSuffix(string& FilePath);
	//构建压缩文件
	void buildFile(ofstream& fout, string& path);

	void writeFileHeaderInfo(ofstream& fout,string& path);
	void analysisFileHeaderInfo(ifstream& fin);
	void uncompressData(ifstream& fin, ofstream& fout);
}; 
