#pragma once
#include <string>
#include <map>
#include "huffmanTree.h"
using namespace std;

//�ַ��ṹ��
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
	unsigned char _character;//�ַ�
	long long _chCount;//���ִ���
	string _strCode;//����
};

//�ļ�������
class FileHandler {
private:
	//�ļ�������ȡ�����ֽ�/�ַ��������ڽ�ѹʱ���жϣ�������Ϊ�ֽ���̫��
	long long totalNum = 0;
	long long _encodeNum = 0;

	HuffmanTreeNode<CharInfo>* root;
	//�洢�ַ��Ͷ�Ӧ����
	map<char, string> codemap;
	map<string, char> decodemap;
	CharInfo info[256];
	string suffix;

public:
	void compress(string& FilePath);
	void uncompress(string& FilePath);
private:
	//ɨ���ļ�,��ȡ������Ϣ
	void _scan(ifstream& fin);
	//����Huffman��
	void buildHuffmanTree();
	//��������
	void buildCode(HuffmanTreeNode<CharInfo>* root,string code);
	//��ȡ�ļ���
	string getFileName(string& path, string& suffix);
	string getSuffix(string& FilePath);
	//����ѹ���ļ�
	void buildFile(ofstream& fout, string& path);

	void writeFileHeaderInfo(ofstream& fout,string& path);
	void analysisFileHeaderInfo(ifstream& fin);
	void uncompressData(ifstream& fin, ofstream& fout);
}; 
