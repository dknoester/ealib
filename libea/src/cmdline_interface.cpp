/* cmdline_interface.cpp
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
#include <ea/cmdline_interface.h>
#include <ea/exceptions.h>

ealib::registrar* ealib::registrar::_instance=0;

//! Parse command-line (and potentially config file) options.
boost::program_options::variables_map ealib::parse_command_line(int argc, char* argv[]) {
    namespace po = boost::program_options;
    using namespace std;
    
    // these options are only available on the command line.  when adding options,
    // if they must be available to the EA, don't add them here -- use meta_data
    // instead.
    po::options_description cmdline_only_options("Command-line only options");
    cmdline_only_options.add_options()
    ("help,h", "produce this help message")
    ("config,c", po::value<string>()->default_value("ealib.cfg"), "ealib configuration file")
    ("checkpoint,l", po::value<string>(), "load a checkpoint file")
    ("override", "override checkpoint options")
    ("reset", "reset all fitness values prior to continuing a checkpoint")
    ("analyze", po::value<string>(), "analyze the results of this EA")
    ("with-time", "output the instantaneous and mean wall-clock time per update");
    
    po::options_description ea_options("Configuration file and command-line options");
    ealib::registrar::instance()->gather_options(ea_options);
    
    po::options_description all_options;
    all_options.add(cmdline_only_options).add(ea_options);
    
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, all_options), vm);
    po::notify(vm);
    
    string cfgfile(vm["config"].as<string>());
    ifstream ifs(cfgfile.c_str());
    if(ifs.good()) {
        po::parsed_options opt=po::parse_config_file(ifs, ea_options, true);
        vector<string> unrec = po::collect_unrecognized(opt.options, po::exclude_positional);
        if(!unrec.empty()) {
            ostringstream msg;
            msg << "Unrecognized options were found in " << cfgfile << ":" << endl;
            for(std::size_t i=0; i<unrec.size(); ++i) {
                msg << "\t" << unrec[i] << endl;
            }
            msg << "Exiting..." << endl;
            throw ealib::ealib_exception(msg.str());
        }
        po::store(opt, vm);
        ifs.close();
    }
    
    if(vm.count("help")) {
        ostringstream msg;
        msg << "Usage: " << argv[0] << " [-c config_file] [-l checkpoint] [--override] [--analyze] [--option_name value...]" << endl;
        msg << all_options << endl;
        throw ealib::ealib_exception(msg.str());
    }
    
    return vm;
}
