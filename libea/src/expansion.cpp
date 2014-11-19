/* expansion.cpp
 *
 * This file is part of EALib.
 *
 * Copyright 2014 David B. Knoester.
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
#include <wordexp.h>
#include <sstream>

#include <ea/expansion.h>
#include <ea/exceptions.h>

/*! Returns an expansion (i.e., with environment variables replaced by their
 values) of the given string.
 */
std::string ealib::expansion(const std::string& s) {
	wordexp_t p;
	if(wordexp(s.c_str(), &p, 0)) {
		wordfree(&p);
		throw bad_argument_exception("error in expansion: " + s);
	}
	std::ostringstream v;
	for(std::size_t i=0; i<p.we_wordc; ++i) {
		v << p.we_wordv[i];
	}
	wordfree(&p);
	return v.str();
}
