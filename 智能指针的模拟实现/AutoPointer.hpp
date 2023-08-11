#pragma once

/* 简易版智能指针（auto_ptr / unique_ptr / shared_ptr / weak_ptr）的设计与实现 */
/* 
	注意：为了模拟指针的使用，必须重载 * 和 -> 运算符！
	注意：结合调用示例理解
*/
namespace Mptr {
	// 1. auto_ptr 的设计与实现
	/*
		auto_ptr 管理权转移，被拷贝对象的出现悬空问题
	*/
	template<class T>
	class auto_ptr {
	public:
		auto_ptr(T* ptr = nullptr)
			:_ptr(ptr) 
		{

		}
		~auto_ptr() {
			if (_ptr) {
				delete _ptr;
			}
		}

		// 拷贝构造：实现管理权转移
		auto_ptr(auto_ptr<T>& aptr)
			:_ptr(aptr._ptr) 
		{
			aptr._ptr = nullptr;
		}

		// 赋值运算符重载：实现管理权转移
		auto_ptr<T>& operator = (auto_ptr<T>& aptr)
		{
			// 1. 检验是否出现对自身赋值
			if (this == &aptr) {
				return *this;
			}

			// 对新构建的指针置空处理
			if (_ptr) { delete _ptr; }

			_ptr = aptr._ptr;
			aptr._ptr = nullptr;

			return *this;
		}

		// 解引用重载！解引用返回对象的“值”
		T& operator * () {
			return *_ptr;
		}

		// -> 重载：返回指针
		T* operator -> () {
			return _ptr;
		}
	private:
		T* _ptr;
	};


	// 2. unique_ptr 的设计与实现
	/*
		unique_ptr：就是防止拷贝的产生；
		不支持：拷贝构造 和 赋值运算符重载

		C98 模拟实现思路：对拷贝构造函数及赋值运算符重载只声明（防止编译器自动生成），并私有化（不能被外部访问，达到不支持效果）！
		C11 模拟实现思路：使用 关键字 delete 即可
	*/
	template<class T>
	class unique_ptr {
	public:
		// 基本构造函数
		unique_ptr(T* ptr = nullptr)
			:_ptr(ptr)
		{

		}

		// 析构函数
		~unique_ptr() {
			if (_ptr) {
				delete _ptr;
			}
		}

		// 拷贝构造函数
		unique_ptr(unique_ptr<T>& uptr) = delete;
		// 赋值运算符重载
		unique_ptr<T>& operator = (unique_ptr<T>& uptr) = delete;

		// 解引用重载！解引用返回对象的“值”
		T& operator * () {
			return *_ptr;
		}

		// -> 重载：返回指针
		T* operator -> () {
			return _ptr;
		}

	private:
		T* _ptr;
	};


	// 3. shared_ptr 的模拟实现【此处使用类模板的方式模拟实现对段上空间释放】
	// 注：官方库中使用的是类来处理一段内存的释放

	// 制作默认仿函数：默认为处理实例的单体对象，而不是如数组形式的一段对象
	template<class D>
	struct Delete {
		void operator () (D* ptr) {
			delete ptr;
		}
	};

	/*
		注意：共享的计数器使用的是指针！而不是静态成员变量！
		每次对一份资源管理时，需要调用构造函数，在构造函数处初始化计数器
		当前缺陷：多线程中缺乏保护！可能由于线程切换，而中断 ++ / -- 的数据操作！（破环原子性：没有一次性完整执行 ++ / --）
	*/
	template<class T, class D = Delete<T>>		// 注意此处设置了默认的空间释放方式
	class shared_ptr {
	public:
		shared_ptr(T* ptr)
			:_ptr(ptr),
			_count(new int(1))
		{

		}

		// 析构函数：析构条件！
		~shared_ptr() {
			// 空间释放条件：引用计数为：0
			//if (--(*_count) == 0) {
			//	D()(_ptr);
			//	delete _count;
			//}
			// 改进
			_release();
		}

		// 拷贝构造：sp2(sp1)
		shared_ptr(const shared_ptr<T>& sptr)
			:_ptr(sptr._ptr),
			_count(sptr._count)
		{
			++(*_count);	// 引用计数 +1
		}

		// 赋值运算符重载
		shared_ptr<T>& operator = (const shared_ptr<T>& sptr) {
			// 1. 判断是否为自身赋值：sp1 = sp1 / sp2 = sp1, sp1 = sp2 【两种情形】
			//if(this == &sptr)				// 该方式只能判断 sp1 = sp1
			// 改进：
			if (_ptr == sptr._ptr) {
				return *this;
			}

			// 2. 对新构造对象处理 / 减被赋值对象的计数，如果是最后一个对象，要释放资源
			//if (--(*_count) == 0) {
			//	delete _ptr;
			//	delete _count;
			//}
			// 改进：
			_release();

			// 3. 共管新资源，++计数
			_ptr = sptr._ptr;
			_count = sptr._count;
			++(*_count);
			return *this;
		}


		// 解引用重载！解引用返回对象的“值”
		T& operator * () {
			return *_ptr;
		}

		// -> 重载：返回指针
		T* operator -> () {
			return _ptr;
		}

		// 提供方法：给外部获取指针
		T* get() const
		{
			return _ptr;
		}

		int use_count() {
			return *_count;
		}

	private:
		// 释放空间处理
		void _release() {
			// 空间释放条件：引用计数为：0
			if (--(*_count) == 0) {
				D()(_ptr);
				delete _count;
			}
		}
	private:
		T* _ptr;
		int* _count;

	};


	// 4. weak_ptr：模拟核心作用：解决使用 shared_ptr 出现的循环引用问题！
	template<class T>
	class weak_ptr {
	public:
		weak_ptr(T* ptr = nullptr)
			:_ptr(ptr)
		{

		}
		~weak_ptr() {
			delete _ptr;
		}

		// 拷贝构造
		weak_ptr(const shared_ptr<T>& wptr)
			:_ptr(wptr.get())
		{}

		// 赋值运算符重载
		weak_ptr<T>& operator = (const shared_ptr<T>& wptr) 
		{
			_ptr = wptr.get();
			return *this;
		}

		// 解引用重载！解引用返回对象的“值”
		T& operator * () {
			return *_ptr;
		}

		// -> 重载：返回指针
		T* operator -> () {
			return _ptr;
		}
	private:
		T* _ptr;
	};
}