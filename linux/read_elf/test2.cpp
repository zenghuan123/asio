#include"test.h"
#include<iostream>
int i=11;
typedef struct{
	int a;
} new_name;
void test2()
{
	new_name c;
	c.a =1212;
	std::cout<<" test2 "<<"  "<<i<< std::endl;
}