#include<iostream>
#include <iostream>
#include <fstream>
#include "date_time.h"
#include "smart_ptr.h"
#include "best_forward.h"
#include "student.pb.h"
using namespace std;
void PromptForAddress(tutorial::Persion *persion) {
	cout << "Enter persion name:" << endl;
	string name;
	cin >> name;
	persion->set_name(name);

	int age;
	cin >> age;
	persion->set_age(age);
}

int test() {
	//GOOGLE_PROTOBUF_VERIFY_VERSION;
	string file_name = "1.txt";
	tutorial::AddressBook address_book;
	{
		fstream input(file_name, ios::in | ios::binary);
		if (!input) {
			cout << file_name << ": File not found. Creating a new file." << endl;
		}
		else if (!address_book.ParseFromIstream(&input)) {
			cerr << "Filed to parse address book." << endl;
			return -1;
		}
	}

	// Add an address
	PromptForAddress(address_book.add_persion());
	{
		fstream output(file_name, ios::out | ios::trunc | ios::binary);
		if (!address_book.SerializeToOstream(&output)) {
			cerr << "Failed to write address book." << endl;
			return -1;
		}
	}

	return 0;
}

int main(int argc, char* argv[])
{
	//pool::test();
	//best_forward::test();
	//test();
	return 0;
}

