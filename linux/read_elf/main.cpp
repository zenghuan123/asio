#include<iostream>
#include<fstream>
#include<string>
#include<cstring>
#include<elf.h>
#include<link.h>
#include"test.h"
#include <chrono>
#include<unordered_set>
using namespace std;
std::string random_str()
{
	std::string s;
	for (int i = 0; i < 10; i++)
	{
		s = s + (char)('a' + std::rand()%26);
	}
	return s;
}
//int __attribute__((weak))i;
int main(int argc, char*argv[])
{
	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	std::unordered_set <std::string > f1(1000 * 10000);
	for (int i = 0; i < 1000 * 10000; i++)
	{
		f1.insert(random_str());
	}
	std::unordered_set<std::string>f2(1100 * 10000);
	now = std::chrono::system_clock::now();
	for (auto st : f1)
	{
		f2.insert(st);
	}
	std::chrono::duration<double> elapsed_seconds = std::chrono::system_clock::now() - now;
	std::cout << elapsed_seconds.count() << std::endl;

	test1();
	test2();

	std::ifstream f;
	f.open("/home/zenghuan/linux/test2/part.o",ios::in);
	unsigned char e_ident[EI_NIDENT];
	f.read((char*)e_ident,EI_NIDENT);
	for (int i = 0; i < 16; i++)
		printf("  %d  ", int(e_ident[i]));
	printf("\n");
	if(strncmp((char*)e_ident,ELFMAG,SELFMAG)!=0)
	{
		std::cout<<"is not elf"<<endl;
		return 1;
	}
	std::cout<<"is elf"<<endl;
	if(e_ident[EI_CLASS]==ELFCLASS32)
	{
		cout<<" systeam is 32"<<endl;
	}else if(e_ident[EI_CLASS]==ELFCLASS64)
	{
		cout<<"system is 64"<<endl;
	}else
	{
		cout<<" error systeam "<<e_ident[EI_CLASS] <<endl;
		for(int i=0;i<16;i++)
			printf("  %d  ",int(e_ident[i]));
		return 1;
	}

	return 0;
}
