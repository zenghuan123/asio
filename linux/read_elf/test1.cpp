#include"test.h"
#include<iostream>

int  i=22;
typedef struct{
	char a;
} new_name;
void test1()
{ 	new_name c;
	c.a ='a';
	std::cout<<" test1 "<<c.a<<" "<<i<<std::endl;
}