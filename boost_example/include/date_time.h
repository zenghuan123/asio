#include<boost/timer.hpp>
#include<boost/progress.hpp>
#include<boost/date_time/gregorian/gregorian.hpp>
#include "boost/date_time/posix_time/posix_time.hpp"

#include <iostream>

namespace date_time {
	void test()
	{
		boost::timer t;
		std::cout << t.elapsed_max() << std::endl;
		std::cout << t.elapsed_min() << std::endl;
		std::cout << t.elapsed() << std::endl;
		boost::progress_timer pt;

		std::cout << boost::gregorian::day_clock::universal_day() << std::endl;

		boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
		std::cout << boost::posix_time::to_iso_extended_string(now) << std::endl;
		
	}

}

