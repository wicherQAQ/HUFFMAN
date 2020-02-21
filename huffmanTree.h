#pragma once
#include <queue>
#include <vector>
using namespace std;
template <typename T>
struct HuffmanTreeNode
{
	typedef HuffmanTreeNode<T> Node;
	T _weight;
	Node* _left;
	Node* _right;
	Node* _parent;
	HuffmanTreeNode(const T& weight)
		:_weight(weight)
		, _left(NULL)
		, _right(NULL)
		, _parent(NULL)
	{}
};

//优先队列比较规则结构体priority_queue
template <typename T>
struct great
{
	bool operator()(const T& left, const T& right)
	{
		return left->_weight > right->_weight;
	}
};


template <typename T>
class HuffmanTree
{
public:
	typedef HuffmanTreeNode<T> Node;

	HuffmanTree() :root(NULL) {}

	~HuffmanTree() {
		destory(root);
	}

	Node* GetRoot()
	{
		return root;
	}
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
		//printf("%d", treeNodePQ.top()->_weight._chCount);

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

	//递归销毁Huffman树
	void destory(Node* root){
		if (root == NULL)
			return;
		destory(root->_left);
		destory(root->_right);
		delete root;
	}

private:
	Node* root;
};

