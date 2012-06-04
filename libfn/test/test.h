#ifndef _TEST_H_
#define _TEST_H_

#include <boost/test/unit_test.hpp>

struct test_rng {
	test_rng(int r) : _r(r) {
	}
	
	virtual ~test_rng() {
	}
	
	virtual int operator()(int m) {
		return std::min(_r,m-1);
	}
	
	void reset(int r) {
		_r = r;
	}
	
	int _r;
};

#endif
