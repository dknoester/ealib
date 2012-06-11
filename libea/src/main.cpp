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
#include <fstream>
#include <vector>

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
        // these options are only available on the command line.  when adding options,
        // if they must be available to the EA, don't add them here -- use meta_data
        // instead.
        po::options_description cmdline_only_options("Command-line only options");
        cmdline_only_options.add_options()
        ("help,h", "produce this help message")
        ("config,c", po::value<string>()->default_value("libea_defaults.cfg"), "ealib configuration file")
        ("checkpoint,l", po::value<string>(), "load a checkpoint file")
        ("override", "override checkpoint options")
        ("reset", "reset all fitness values prior to continuing a checkpoint")
        ("analyze", po::value<string>(), "analyze the results of this EA")
        ("with-time", "output the instantaneous and mean wall-clock time per update");
	    
        po::options_description ea_options("Configuration file and command-line options");
        ea::registrar::instance()->gather_options(ea_options);
		
        po::options_description all_options;
        all_options.add(cmdline_only_options).add(ea_options);
        
        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, all_options), vm);
        po::notify(vm);
        
        string cfgfile(vm["config"].as<string>());
        ifstream ifs(cfgfile.c_str());
        if(ifs.good()) {
            po::store(po::parse_config_file(ifs, ea_options), vm);
            ifs.close();
        }
        
        if(vm.count("help")) {
            cout << "Usage: " << argv[0] << " [-c config_file] [-l checkpoint] [--override] [--analyze] [--option_name value...]" << endl;
            cout << all_options << endl;
            return -1;
        }
        
        if(vm.count("analyze")) {
            ea::registrar::instance()->analyze(vm);
        } else if(vm.count("checkpoint")) {
            ea::registrar::instance()->continue_checkpoint(vm);
        } else {
            ea::registrar::instance()->run(vm);
        }
    } catch(const po::unknown_option& ex) {
        cerr << "Unknown option: " << ex.get_option_name() << endl;
        return -1;
    } catch(const po::error& ex) {
        cerr << "Program option error: " << ex.what() << endl;
        return -1;
	} catch(const ea::ealib_exception& ex) {
		cerr << "Caught exception: " << ex.msg << endl;
		return -1;
	}
	
	return 0;
}
