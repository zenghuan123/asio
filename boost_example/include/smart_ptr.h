#include <memory>
#include<iostream>
#include<boost/pool/pool.hpp>
#include<boost/pool/object_pool.hpp>
namespace smart_ptr
{
	class A 
	{
	public:
		A(int size)
		{
			std::cout << "construct" << std::endl;
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
namespace pool
{
	class Ob
	{
	public:
		char *arr;
		Ob(int i) :arr(new char[i])
		{
			std::cout << "construct" << std::endl;
		}
		~Ob()
		{
			std::cout << "destroy" << std::endl;
		}
	};
	
	boost::object_pool<Ob> bop{};
	void pool_free(Ob*ob)
	{
		bop.destroy(ob);
	}
	std::shared_ptr<Ob> new_ob()
	{	
		auto f = [](Ob*j) {bop.free(j); };
		//std::shared_ptr<Ob> pp(bop.construct(1), f);
		std::shared_ptr<Ob> pp(bop.construct(1), pool_free);
		return pp;
	}
	void test()
	{
		boost::pool<> p1(sizeof(int));
		int*i = static_cast<int*>(p1.malloc());
		assert(p1.is_from(i));
		p1.free(i);
		std::shared_ptr<Ob> ob_ptr = new_ob();	
		std::cout << "count :" << ob_ptr.use_count() << std::endl;
	}
}