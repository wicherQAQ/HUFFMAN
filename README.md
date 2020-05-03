### 摘要

&emsp;&emsp;受到疫情的影响，今年包括考研的时间线都往后顺延，因此也多了好多准备的时间。为了较好的掌握c++的语法，以及考虑到Huffman算法的易实现性，我便产生了写huffman压缩的想法。待到真正上手写代码的时候，发现其设计的细节特别多，一度也让我有了放弃的念头！好在都被我一一解决，哈哈！还是挺高兴的。

&emsp;&emsp;这个小项目其实说难也不难，但前提是得要熟悉三方面的知识。第一就是最基本的Huffman编码；第二是对数据的移位操作——对二进制数的理解；第三还得熟悉c++（注：或者其他语言）对文件字节流的操作。总的来说思路还是比较清晰的。

### Huffman算法

&emsp;&emsp;Huffman编码法被称为最优编码法，所获得的编码为无重复前缀码，其编码的依据是根据被编码的字符在整个原文中出现的频率，频率越高，代表该字符的编码长度越短，因此通过编码可以减少文件的大小。

#### Huffman树的生成过程

1. 首先统计出各个字符出现的频率作为权值，然后每个字符对应创建一个节点，节点保存字符值、权值、编码。
2. 取出所有节点中权值最小的两个节点（利用堆或者优先队列），将其权值相加后作为新节点的权值，并将新节点作为这两个节点的双亲节点。将该节点重新放入所有节点中。
3. 重复 2 步骤，直到剩下一个节点，则所有节点构成一个Huffman树，该节点为树根 

```c++
	template <class T>
	HuffmanTree(T* info)
	{
		//声明huffman树的优先队列（底层是用堆实现的）
		priority_queue<Node*, vector<Node*>,great<Node*>> treeNodePQ;

		for (int i = 0; i < 256; ++i)
		{
			//invalid表示字符的统计次数为0，因此不包括在Huffman树中
			if (info[i]._chCount != 0)
			{
				Node* tmp = new Node(info[i]);
				treeNodePQ.push(tmp);
			}
		}

		Node* Left;
		Node* Right;
		while (treeNodePQ.size() > 1){
			//获取堆顶两个最小的node
			Left = treeNodePQ.top();
			treeNodePQ.pop();

			Right = treeNodePQ.top();
			treeNodePQ.pop();

			Node* Parent = new Node(Left->_weight + Right->_weight);
			Parent->_left = Left;
			Parent->_right = Right;
			Left->_parent = Parent;
			Right->_parent = Parent;

			treeNodePQ.push(Parent);
		}
		root = treeNodePQ.top();
	}
```

#### 编码过程（Huffman树的遍历）

&emsp;&emsp;编码的生成我是利用递归遍历的方法实现的，当然你也可以用BFS。Huffman的规则是通往左子树的路径为0，右子树的路径为1。下面是代码：

```
//前序遍历huffman树，并生成编码,将节点信息存入到对象中
void FileHandler::buildCode(HuffmanTreeNode<CharInfo>* root, string code) {
	if (root == NULL)return;
	//叶子节点
	if (root->_left == NULL && root->_right == NULL) {
		root->_weight._strCode = code;
		//将每个字符和对应的编码存入codemap
		codemap.insert(make_pair(root->_weight._character, root->_weight._strCode));
	}
	buildCode(root->_left, code + '0');
	buildCode(root->_right, code + '1');
}
```

#### 解码过程：

&emsp;&emsp;从树根开始，根据Huffman编码0向左，1向右终点便是字符所在节点。但是这种解码方式有个前提就是需要根据文件头信息（编码的时候自己写入），重建Huffman树。

```
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
```

### 文件后缀和编码信息

&emsp;&emsp;为了将压缩后的数据解码，我们需要将一些文件的信息，比如文件的大小、文件的后缀名以及编码信息写入压缩文件。我的文件头信息结构如下表：

| 总字节数  | 文件后缀字节数 | 后缀名   | 编码个数 | 字符          | 出现频次  |
| --------- | -------------- | -------- | -------- | ------------- | --------- |
| long long | short          | 后缀长度 | short    | unsigned char | long long |

&emsp;&emsp;总字节数用于后面判断文件结束，但可能因为文件过大，字节数超过long long所能存储的最大数。目前我觉得可以用unsigned long long来存储，或者利用单位换算，降低其数量级（但可能会涉及到精度问题）。由于后缀名具有不确定性，读取时并不确定所占的字节数，所以用2个字节存入后缀名所占字节数。

```
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
```

### 文件数据压缩

&emsp;&emsp;文件压缩的细节点很多，比如关于c++的ifstream这个东西当时就搞的我很头疼。原来我是用fin.get(in_char);来读取的，但后来绝的每次读取一个字节效率实在太低。因此换用缓冲流来接收。还有关于不够八位的补零处理。虽然我补了零但我觉得直接写入应该也没什么问题，因为我是根据处理的字节总数来判断是否读取结束的。

```
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
```

### 压缩率

&emsp;&emsp;关于压缩率，我测试了`txt`文件，`jpg`文件，和`MP3`文件除了`txt`偶尔达到50%，其他均高达90%+，毕竟图片文件和`mp3`文件都是压缩后的产物，压缩率不高也很正常。

### 相关文章

关于二进制的介绍和c++文件操作相关，详见

[揭开二进制与位移运算的神秘面纱](https://wicherqaq.github.io/2020/02/16/%E6%8F%AD%E5%BC%80%E4%BA%8C%E8%BF%9B%E5%88%B6%E4%B8%8E%E4%BD%8D%E7%A7%BB%E8%BF%90%E7%AE%97%E7%9A%84%E7%A5%9E%E7%A7%98%E9%9D%A2%E7%BA%B1/)

[文件操作fstream](https://wicherqaq.github.io/2020/02/09/%E6%96%87%E4%BB%B6%E6%93%8D%E4%BD%9Cfstream/)
