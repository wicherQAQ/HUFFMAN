#include "huffman.h"
#include <fstream>
#include <iostream>
#include <strstream>
using namespace std;

void FileHandler::compress(string& FilePath) {
	ifstream fin(FilePath, ios::binary);
	if (!fin.is_open()) {
		cout << "Error��File is not exist" << endl;
		return;
	}
	_scan(fin);
	buildHuffmanTree();//����huffman���ͱ���
	string _suffix(".Huff");
	ofstream fout(getFileName(FilePath,_suffix), ios::binary);
	if (!fout.is_open()) {
		cout << "Error��File can not be writed" << endl;
		fin.close();
		return;
	}
	buildFile(fout,FilePath);//д���ļ���huffman�������Ϣ
	fin.close();
	fout.close();
}

//ͳ��Ƶ��
void FileHandler::_scan(ifstream& fin) {
	unsigned char buffer[1024];
	//����ַ�
	for (int i = 0; i < 256; ++i)
	{
		info[i]._character = i;
	}
	size_t n;
	do {
		fin.read((char*)buffer, 1024);
		n = fin.gcount();
		totalNum += n;//�ļ����ֽ���
		for (size_t i = 0; i < n; i++) {
			info[buffer[i]]._chCount++;
		}
	} while (n > 0);
}

//������������
void FileHandler::buildHuffmanTree() {
	HuffmanTree<CharInfo> tree(info);
	buildCode(tree.GetRoot(), "");
}

//ǰ�����huffman���������ɱ���,���ڵ���Ϣ���뵽������
void FileHandler::buildCode(HuffmanTreeNode<CharInfo>* root, string code) {
	if (root == NULL)return;
	//Ҷ�ӽڵ�
	if (root->_left == NULL && root->_right == NULL) {
		root->_weight._strCode = code;
		//��ÿ���ַ��Ͷ�Ӧ�ı������codemap
		codemap.insert(make_pair(root->_weight._character, root->_weight._strCode));
		//printf("0x%X === %lld�� === ����Ϊ %s\n", root->_weight._character, root->_weight._chCount, root->_weight._strCode.c_str());
	}
	buildCode(root->_left, code + '0');
	buildCode(root->_right, code + '1');
}

//д���ļ���׺�ͱ�����Ϣ
void FileHandler::writeFileHeaderInfo(ofstream& fout, string& path) {
	//���ֽ���
	fout.write((char*)&totalNum, 8);
	//д���ļ���׺
	string suffix = getSuffix(path);
	//�ļ���׺���ֽ���
	short suffixLen = suffix.length();
	fout.write((char*)&suffixLen, sizeof(short));
	//�ļ���׺
	fout.write(suffix.c_str(), suffixLen);
	_encodeNum += suffixLen;
	//д��������
	short count = codemap.size();
	fout.write((char*)&count, sizeof(short));
	_encodeNum += 12;
	//д���ַ��Ͷ�ӦƵ��
	map<char, string>::iterator it;
	long long _count = 0;
	unsigned char code;
	for (it = codemap.begin();it != codemap.end();it++) {
		//д���ַ�
		code = it->first;
		fout.put(code);
		//����Ƶ�Σ�long long 8byte -256������Ļ��ܹ�ռ2kbyte
		_count = info[code]._chCount;
		fout.write((char*)&_count, 8);
		_encodeNum += 9;
	}
}

//д��ѹ������
void FileHandler::buildFile(ofstream& fout, string& path) {
	printf("��ʼд���ļ���Ϣ\n");
	//д���ļ�huffman��Ϣ
	writeFileHeaderInfo(fout, path);
	printf("��ʼ�ռ���д�����\n");

	ifstream fin(path, ios::binary);
	if (!fin.is_open()) {
		cout << "Error��File is not exist" << endl;
		return;
	}

	//length������ܳ�
	int length=0;
	int i, j, gcount = 0;//ÿ�ζ�ȡ���ֽ���
	unsigned char buff[256];//�ֽ���������
	unsigned char out_c, tmp_c;
	string code, out_string;//ת����ı���;����������
	map<char, string>::iterator it;

	do {

		//������ת��Ϊ�������ַ���
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
						printf("error!�����ַ��޶�Ӧ���룡\n");
						exit(1);//�Ƴ�����
					}
				}
			}
		}

		if (length > 0)
		{
			out_string.clear();
			if (length > 8) {
				//��huffman��01�����Զ�������д�뵽����ļ�
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
				code = code.substr(i, length - i);//iλ��ʼ������Ϊlength-i
				length = code.length();
			}

			if (fin.eof() && length < 8) {
				//С�ڵ���8
				char n = 8 - length;//��0����
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
	printf("�ļ���%lld���ֽ�\n", totalNum);
	printf("ѹ����Ϊ%lld���ֽ�\n", _encodeNum);
	float rate = _encodeNum*1.0/totalNum*100;
	printf("ѹ����Ϊ%.2f\%%", rate);
	fin.close();
}

void FileHandler::uncompress(string& FilePath) {
	ifstream fin(FilePath, ios::binary);
	if (!fin.is_open()) {
		cout << "Error��File is not exist" << endl;
		return;
	}
	//����ͷ�ļ���Ϣ
	analysisFileHeaderInfo(fin);
	ofstream fout(getFileName(FilePath, suffix), ios::binary);
	if (!fout.is_open()) {
		cout << "Error��File can not be writed" << endl;
		fin.close();
		return;
	}
	uncompressData(fin,fout);
	fin.close();
	fout.close();
}

//�����ļ�ͷ��huffman����
void FileHandler::analysisFileHeaderInfo(ifstream& fin) {
	char _suffix[10], code[256];
	short count, suffixLen;//������
	unsigned char c;//�ַ�
	long long _count = 0;//Ƶ��

	fin.read((char*)&totalNum, 8);
	fin.read((char*)&suffixLen, 2);
	fin.read(_suffix, suffixLen);
	_suffix[suffixLen] = '\0';
	suffix = _suffix;//��ȡ�ļ�����׺

	fin.read((char*)&count, sizeof(short));
	printf("�ܱ�����%d\n", count);

	for (int i = 0; i < 256; ++i)
		info[i]._character = i;

	for (int i = 0;i < count;i++) {
		fin.read((char*)&c,1);
		fin.read((char*)&_count, 8);
		info[c]._character = c;
		info[c]._chCount = _count;
	}
	printf("��ɱ����ռ�");
}

//��������
void FileHandler::uncompressData(ifstream& fin, ofstream& fout) {
	unsigned char in_char;
	long long writen_len = 0;//��¼��ȡ���ļ�����
	HuffmanTree<CharInfo> tree(info);
	HuffmanTreeNode<CharInfo>* temp = tree.GetRoot();
	while (1) {
		fin.read((char*)&in_char, 1);
		for (int i = 0; i < 8; i++)
		{
			if (in_char & 128)//128 =binary=>1000 0000 ,�������ȡin_char���λ��ȡֵ
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

//��ȡ�ļ���
string FileHandler::getFileName(string& FilePath,string &suffix)
{
	size_t begin = 0;
	size_t end = FilePath.find_last_of(".");
	return FilePath.substr(begin, end)+ suffix;
}

//��ȡ�ļ���׺
string FileHandler::getSuffix(string& FilePath) {
	size_t end = FilePath.length();
	size_t begin = FilePath.find_last_of(".");
	return FilePath.substr(begin, end);
}