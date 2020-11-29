#include <memory>
#include<iostream>
namespace smart_ptr
{
	class A 
	{
	public:
		A(int size)
		{
			std::cout << "construct" << std::endl;
		}
		static void * operator new(size_t size)
		{
			std::cout << "new" << size<< std::endl;
			return ::operator new(size);
		}
		static void operator delete(void* ptr)
		{
			std::cout << "delete" << std::endl;
			return ::operator delete(ptr);
		}
		~A()
		{
			std::cout << "destroy" << std::endl;
		}
	};
	void end(int i)
	{
		std::cout << "end " << i << std::endl;
	}
	void test()
	{
		std::unique_ptr<int> ptr(new int{ 1 });
		std::cout << *ptr << std::endl;
		std::shared_ptr<A> s_p = std::make_shared<A>(4);
		
	}
}