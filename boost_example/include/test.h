#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <boost/assign.hpp>

#include<iostream>
#include <iostream>
#include <fstream>
#include <set>
#include<string>
#include "boost/operators.hpp"
namespace test
{	
	using namespace std;
	using namespace boost::assign;
	void assign_demo()
	{
		std::vector<int> v = list_of(11).repeat(3, 2);
		v += 1, 2, 3, 4, 5, 6, 7, 8, 9;
		std::set<string> s;
		s += "c", "cpp", "lua", "swift";

		std::map<int, int>in;
		in += make_pair(1, 2), make_pair(2, 3);

		unordered_map<int, string> m = { {1,"f123"},{2,"adf"},{3,"erfdf"} };
	}


}

namespace test2 {
	class point:boost::less_than_comparable<point, boost::equality_comparable<point, boost::addable<point, boost::subtractable<point>>>>
		//
	{
	public:
		point(int x, int y, int z) :x(x), y(y), z(z)
		{

		}
		friend bool test2::point::operator<(const point&l, const point& r)
		{
			std::cout << (l.x*l.x + l.y*l.y + l.z*l.z) << ":" << (r.x*r.x + r.y*r.y + r.z*r.z) << std::endl;
			bool b =  (l.x*l.x + l.y*l.y + l.z*l.z) < (r.x*r.x + r.y*r.y + r.z*r.z);
			if (b)
			{
				std::cout << "true" << std::endl;
			}
			else {
				std::cout << "false" << std::endl;
			}
			return b;
		}
		friend bool test2::point::operator==(const point&l, const point&r)
		{
			return l.x == r.x && l.y == r.y&& l.z == r.z;
		}

		friend test2::point& operator+=(const point& add, const point& add1)
			//friend point& operator+=(const point& add)???
		{
			point r = { 0,0,0 };
			r.x = add.x +add1.x;
			r.y = add.y + add1.y;
			r.z = add.z + add1.z;
			return r;
		}

		friend test2::point& operator-=(const point& add, const point& add1)
		{
			point r = { 0,0,0 };
			r.x = add.x - add1.x;
			r.y = add.y - add1.y;
			r.z = add.z - add1.z;
			return r;
		}

		int x;
		int y;
		int z;
	};
	void operator_demo()
	{
		point l = { 1,2,3 };
		point r = { 2,3,1 };
		std::cout << ((l.x*l.x + l.y*l.y + l.z*l.z) < (r.x*r.x + r.y*r.y + r.z*r.z));
		
		std::cout << (l < r) << std::endl;
		std::cout << (l > r) << std::endl;
		std::cout << (l >= r) << std::endl;
		std::cout << (l == r) << std::endl;
		point re = l + r;
		
	}
}