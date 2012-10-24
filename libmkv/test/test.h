/* test.h
 * 
 * This file is part of EALib.
 * 
 * Copyright 2012 David B. Knoester.
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
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
