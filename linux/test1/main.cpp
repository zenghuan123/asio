#define UNW_LOCAL_ONLY
#include <libunwind.h>
#include <stdio.h>
#include<algorithm>
#include<vector>
#include<iostream>
#include<thread>
#include<chrono>
#include"test.h"
// Call this function to get a backtrace.
void backtrace() {
    unw_context_t context;
    unw_getcontext(&context);

    unw_cursor_t cursor;
    unw_init_local(&cursor, &context);

    int frameNr = 0;
    while (unw_step(&cursor)) {
        ++frameNr;
        unw_word_t ip = 0;
        unw_get_reg(&cursor, UNW_REG_IP, &ip);

        unw_word_t sp = 0;
        unw_get_reg(&cursor, UNW_REG_SP, &sp);

        char symbol[256] = {"<unknown>"};
        unw_word_t offset = 0;
        unw_get_proc_name(&cursor, symbol, sizeof(symbol), &offset);

        fprintf(stderr, "#%-2d 0x%016" PRIxPTR " sp=0x%016" PRIxPTR " %s + 0x%" PRIxPTR "\n", frameNr,
                static_cast<uintptr_t>(ip), static_cast<uintptr_t>(sp), symbol, static_cast<uintptr_t>(offset));
    }
}


#include <link.h>
#include <stdlib.h>
#include <stdio.h>

int iterate_phdr_callback(struct dl_phdr_info*info,size_t size,void*data)
{
	std::cout<<" phdr call back "<<info->dlpi_phnum<<std::endl;
	for(int i=0;i<info->dlpi_phnum;i++)
	{
		const ElfW(Phdr)& phdr = info->dlpi_phdr[i];
		if(phdr.p_type==PT_LOAD)
		{
			void * addr =(void*)(info->dlpi_addr + phdr.p_vaddr);
			Dl_info dlinfo;
			dladdr(addr,&dlinfo);
			std::cout<<" "<<info->dlpi_name<< " "<<dlinfo.dli_fname<<" ";
			if(dlinfo.dli_sname)
			{
				std::cout<<dlinfo.dli_sname<<" "<<std::endl;
			}else
			{
				std::cout<<std::endl;
			}
		}
	}
}

void dl_iterate_phdr_test()
{
char dl[12];
dl_iterate_phdr(iterate_phdr_callback,dl);
}



void * data[10];
void foo() {
    backtrace(); // <-------- backtrace here!
}

void bar() {
  foo();
}

void a()
{
	int *i = new int[4];
	std::cout<<"a run"<<std::endl;
}



#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


#include<signal.h>
int pipe_fd = 0;
void notify_avatar_finish()
{
	write(pipe_fd,"a",1);
}
void notify_all_finish()
{
	write(pipe_fd,"f",1);
}

void test(const char*s)
{	
	signal(SIGPIPE,SIG_IGN)
	std::cout<< s<<std::endl;
	std::cout<<"create"<<s<< std::endl;
	pipe_fd = open(s,O_WRONLY);
	std::cout<<"create finish"<<std::endl;
	while (true)
	{
		/* code */
		notify_avatar_finish();
		notify_all_finish();
		std::cout<<"write"<<std::endl;
	}
}




void b1()
{
	std::cout<<" b "<<std::endl;
	for(int i=0;i<10;i++)
	{
		std::cout<<i<<std::endl;
	}
}

void test2()
{
	std::vector<int> t= {9,8,7,6,5,4,3,2,1}; //{1,2,3,4,5,6,7,8,9};
	std::cout<<"123"<<std::endl;
    auto c = [](const int i,const int j){return i>j;};
    int low=std::lower_bound(t.begin(),t.end(),3,c)-t.begin();
    int upp=std::upper_bound(t.begin(),t.end(),3,c)-t.begin();
    std::cout<<low<<std::endl;
    std::cout<<upp<<std::endl;
	while (1)
	{
		a();
		std::this_thread::sleep_for(std::chrono::seconds(2));
	}
	
}

int main(int argc, char **argv) {
	//b1();
	try{
		test(argv[1]);
	}
	catch (std::exception& e)
	{
		std::cout<<"  exception "<<e.what()<<std::endl;
	
	}
	

  //	bar();
  	//dl_iterate_phdr_test();

  return 0;
}