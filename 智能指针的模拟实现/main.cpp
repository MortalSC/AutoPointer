#include <iostream>
#include <memory>
#include "AutoPointer.hpp"

class A
{
public:
	~A()
	{
		std::cout << "~A()" << std::endl;
	}
	//private:
	int _a1 = 0;
	int _a2 = 0;
};

void test_auto_ptr()
{
	Mptr::auto_ptr<A> ap1(new A);
	ap1->_a1++;
	ap1->_a2++;

	Mptr::auto_ptr<A> ap2(ap1);
	//ap1->_a1++;
	//ap1->_a2++;
	ap2->_a1++;
	ap2->_a2++;

	// 2 2
	std::cout << ap2->_a1 << std::endl;
	std::cout << ap2->_a2 << std::endl;

	Mptr::auto_ptr<A> ap3(new A);
	ap2 = ap3;

	ap2->_a1++;
	ap2->_a2++;

	std::cout << ap2->_a1 << std::endl;
	std::cout << ap2->_a2 << std::endl;
}

// unique_ptr:简单粗暴，不让拷贝 只适用于不需要拷贝一些场景
void test_unique_ptr()
{
	Mptr::unique_ptr<A> up1(new A);
	//Mptr::unique_ptr<A> up2(up1);
	up1->_a1++;
	up1->_a2++;

	Mptr::unique_ptr<A> up3(new A);
	//up1 = up2;
}

// shared_ptr 需要拷贝的场景
void test_shared_ptr1()
{
	Mptr::shared_ptr<A> sp1(new A);
	Mptr::shared_ptr<A> sp2(sp1);
	Mptr::shared_ptr<A> sp3(sp1);

	sp1->_a1++;
	sp1->_a2++;
	std::cout << sp2->_a1 << ":" << sp2->_a2 << std::endl;
	sp2->_a1++;
	sp2->_a2++;
	std::cout << sp1->_a1 << ":" << sp1->_a2 << std::endl;

	Mptr::shared_ptr<A> sp5(new A);
	Mptr::shared_ptr<A> sp6(sp5);

	sp1 = sp5;
	sp2 = sp5;
	sp3 = sp5;

	// 自己给自己赋值
	Mptr::shared_ptr<int> sp4(new int);
	sp4 = sp4;
	sp1 = sp5;
}

struct Node
{
	int _val = 0;
	//Mptr::shared_ptr<Node> _next;
	//Mptr::shared_ptr<Node> _prev;

	Mptr::weak_ptr<Node> _next;
	Mptr::weak_ptr<Node> _prev;

	~Node()
	{
		std::cout << "~Node" << std::endl;
	}
};

// 循环引用 -- weak_ptr不是常规智能指针，没有RAII，不支持直接管理资源
// weak_ptr主要用shared_ptr构造，用来解决shared_ptr循环引用问题
void test_shared_ptr2()
{

	//Mptr::shared_ptr<Node> n1(new Node);
	//Mptr::shared_ptr<Node> n2(new Node);

	//std::cout << n1.use_count() << std::endl;
	//std::cout << n2.use_count() << std::endl;

	//n1->_next = n2;
	//n2->_prev = n1;

	//std::cout << n1.use_count() << std::endl;
	//std::cout << n2.use_count() << std::endl;
}

template<class T>
struct DeleteArray
{
	void operator()(T* ptr)
	{
		std::cout << "delete[]" << ptr << std::endl;
		delete[] ptr;
	}
};

template<class T>
struct Free
{
	void operator()(T* ptr)
	{
		std::cout << "free" << ptr << std::endl;
		free(ptr);
	}
};

// 定制删除器
void test_shared_ptr3()
{
	// 仿函数对象
	/*std::shared_ptr<Node> n1(new Node[5], DeleteArray<Node>());
	std::shared_ptr<Node> n2(new Node);

	std::shared_ptr<int> n3(new int[5], DeleteArray<int>());

	std::shared_ptr<int> n4((int*)malloc(sizeof(12)), Free<int>());*/

	// lambda
	//std::shared_ptr<Node> n1(new Node[5], [](Node* ptr){delete[] ptr; });
	//std::shared_ptr<Node> n2(new Node);

	//std::shared_ptr<int> n3(new int[5], [](int* ptr){delete[] ptr; });

	//std::shared_ptr<int> n4((int*)malloc(sizeof(12)), [](int* ptr){free(ptr); });
	//std::shared_ptr<FILE> n5(fopen("test.txt", "w"), [](FILE* ptr){fclose(ptr); });

	//std::unique_ptr<Node, DeleteArray<Node>> up(new Node[5]);
}

void test_shared_ptr4()
{
	Mptr::shared_ptr<Node, DeleteArray<Node>> n1(new Node[5]);
	Mptr::shared_ptr<Node> n2(new Node);
	Mptr::shared_ptr<int, DeleteArray<int>> n3(new int[5]);
	Mptr::shared_ptr<int, Free<int>> n4((int*)malloc(sizeof(12)));
}

int main() {

	// auto_ptr 测试
	test_auto_ptr();

	// unique_ptr 测试
	test_unique_ptr();

	// shared_ptr 的基本构造测试：需要拷贝的场景
	test_shared_ptr1();

	// 循环引用问题的测试
	test_shared_ptr2();

	// 定制删除器
	test_shared_ptr3();

	// 模拟类模板方式使用一段内存变量的处理
	test_shared_ptr4();
	return 0;
}