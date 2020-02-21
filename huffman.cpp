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
	ofstream fout(getFileName(FilePath), ios::binary);
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
		totalNum += n;
		for (size_t i = 0; i < n; i++) {
			info[buffer[i]]._chCount++;
		}
	} while (n > 0);
}

//������������
void FileHandler::buildHuffmanTree() {
	HuffmanTree<CharInfo> tree(info);
	buildCode(tree.GetRoot(),"");
}

//ǰ�����huffman���������ɱ���,���ڵ���Ϣ���뵽������
void FileHandler::buildCode(HuffmanTreeNode<CharInfo>* root, string code) {
	if (root == NULL)return;
	//Ҷ�ӽڵ�
	if (root->_left == NULL && root->_right == NULL) {
		root->_weight._strCode = code;
		//��ÿ���ַ��Ͷ�Ӧ�ı������codemap
		codemap.insert(make_pair(root->_weight._character, root->_weight._strCode));
	}
	buildCode(root->_left,code+'0');
	buildCode(root->_right,code+'1');
}

//��ȡ�ļ���
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
//		//һ��ѭ��дһ���ֽ�
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

//д���ļ�������huffman����
void FileHandler::writeFileHeaderInfo(ofstream& fout) {
	//д���ܱ�����
	short count = codemap.size();
	fout.write((char*)&count,sizeof(short));
	//д���ַ���Ӧ����
	map<char, string>::iterator it;
	for (it = codemap.begin();it != codemap.end();it++) {
		//д���ַ�
		fout.put(it->first);
		//д����볤��
		short codeLen = it->second.length();
		fout.write((char*)&codeLen, sizeof(short));
		//д�����
		//���ֱ��д���ַ���
		fout.write(it->second.c_str(), it->second.length());
	}
}

//�����ļ�ͷ��huffman����
void FileHandler::analysisFileHeaderInfo(ifstream& fin) {
	short count;//������
	char c;//�ַ�
	short codeLen;//���볤��
	char code[256];//����
	codemap.clear();
	if (fin.is_open()) {
		fin.read((char*)&count, sizeof(short));
		printf("�ܱ�����%d\n", count);
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
		fin.get(buff);//�ٶ�һ���ֽھͶ����ļ�������־��
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



//ѹ���ļ�
void FileHandler::buildFile(ofstream &fout,string &path) {
	//д���ļ�huffman��Ϣ
	writeFileHeaderInfo(fout);
	unsigned char c_char;
	ifstream fin(path);
	if (!fin.is_open()) {
		cout << "Error��File is not exist" << endl;
		return;
	}

	//length������ܳ�
	int length,i,j;
	char in_char;
	unsigned char out_c, tmp_c;
	string code,out_string;
	map<char, string>::iterator it;
	bool batchflag = false;//�������־λ
	
	do {
		//ÿ�ζ�ȡһ���ֽڣ����ҵ���Ӧ���뽫���벻��׷��ֱ������1024
		fin.get(in_char);

		if (fin.eof()) {
			length = code.length();
			if (length >0) {
				if (length >= 8) {
					out_string.clear();
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
					code = code.substr(i, length - i);//iλ��ʼ������Ϊlength-i
					length = code.length();
				}

				if (length!=0) {
					short n = 8 - length;//��0����
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
			//ʮ����������ʽ�������
			//�Ҳ��������˳�����
			printf("Can't find the huffman code of character %X\n", in_char);
			exit(1);//�Ƴ�����
		}

		length = code.length();
		
		if (length > 1024)
		{
			out_string.clear();
			//��huffman��01�����Զ�������д�뵽����ļ�
			for (i = 0; i + 7 < length; i += 8)
			{
				// ÿ��λ01ת����һ��unsigned char���
				// ��ʹ��char�����ʹ��char������λ������ʱ�����λ��Ӱ����
				// ����char��unsigned char�໥ת��������λ������
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
