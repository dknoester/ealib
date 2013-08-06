/* main.cpp
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
#include <boost/program_options.hpp>
#include <iostream>

#include <ea/exceptions.h>
#include <ea/cmdline_interface.h>

//! Called to gather properties (meta-data) into an options_description.
void gather_options(boost::program_options::options_description& opt_desc);


/*! Common main function for most evolutionary algorithms supported by EALib.
 
 This is more of a convenience than anything else - EALib is a header-only library,
 and is most likely to be used within existing code (i.e., something that already has
 a main() and configuration framework).  As such, it's reasonable to de-couple the
 runner from the EA.
 */
int main(int argc, char* argv[]) {
	namespace po = boost::program_options;
	using namespace std;

	try {
        ealib::registrar::instance()->exec(argc, argv);
    } catch(const po::unknown_option& ex) {
        cerr << "Unknown option: " << ex.get_option_name() << endl;
        return -1;
    } catch(const po::error& ex) {
        cerr << "Program option error: " << ex.what() << endl;
        return -1;
	} catch(const ealib::ealib_exception& ex) {
		cerr << "Caught exception: " << ex.msg << endl;
		return -1;
	}
	
	return 0;
}
