#include <iostream>

namespace best_forward
{


	class Test
	{
	public:
		int * a;
		Test():a(new int[16])
		{
			std::cout << "construction call" << std::endl;
		}
		Test(const Test& t)
		{
			std::cout << "copy " << std::endl;
		}
		Test(Test&&t)
		{
			std::cout << "move " << std::endl;
		}
		~Test()
		{
			std::cout << "destroy" << std::endl;
		}
	};
	using namespace std;

	void fun(Test& x) { cout << "call lvalue ref" << endl; }
	void fun(Test&& x) { cout << "call rvalue ref" << endl; }
	void fun(const Test& x) { cout << "call const lvalue ref" << endl; }
	void fun(const Test&& x) { cout << "call const rvalue ref" << endl; }

	template<typename T>
	void PerfectForward(T &&t)
	{
		std::cout << "T is a ref type?: " << std::is_reference<T>::value << std::endl;
		std::cout << "T is a lvalue ref type?: " << std::is_lvalue_reference<T>::value << std::endl;
		std::cout << "T is a rvalue ref type?: " << std::is_rvalue_reference<T>::value << std::endl;
		//t传进来是什么类型，到fun就是什么类型
		fun(forward<T>(t));
		//fun(t);
	}

	Test get()
	{
		Test a;

		return a;
	}

	int test()
	{

		Test a;
		fun(a);
		std::cout << "--------------------" << std::endl;
		Test& ref = a;
		fun(ref);
		std::cout << "--------------------" << std::endl;
		//Test c = get();
		PerfectForward(a);            // call lvalue ref
		std::cout << "--------------------" << std::endl;
		PerfectForward(move(a));      // call rvalue ref
		std::cout << "--------------------" << std::endl;
		const Test b;
		PerfectForward(b);           // call const lvalue ref
		std::cout << "--------------------" << std::endl;
		PerfectForward(move(b));     // call const rvalue ref

		return 0;
	}
}

