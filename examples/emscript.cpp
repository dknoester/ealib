/* emscript.cpp
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

/* This file is meant for compilation by emscripten, like so:
 
 ../emscripten/1.16.0/emcc -I../libea/include -I/usr/local/include \
 emscript.cpp -o logic9.html --embed-file ../etc@/ \
 -DLIBEA_CHECKPOINT_OFF -DBOOST_PARAMETER_MAX_ARITY=7
 
 */
#include <boost/algorithm/string/trim_all.hpp>
#include <boost/algorithm/string/split.hpp>
#include <sstream>
#include <fstream>
#include <vector>
#include <string>

#include <ea/datafiles/runtime.h>
using namespace ealib;

#include "logic9.h"

/*! In emscripten world, it's a bit of a pain to use third-party libraries.
 (At least, until there's a bjam toolchaing for it.)  So, we're going to eliminate
 the dependencies on the Boost libraries.
 */
int main(int argc, char* argv[]) {
    using namespace std;
    using namespace boost;
    cout << "constructing..." << endl;
    ea_type ea;
    cout << "loading config..." << endl;
    ifstream infile("emscript.cfg");//argv[1]);
    if(!infile.is_open()) {
        ostringstream msg;
        msg << "emscript.cpp::main: could not open config file " << argv[1];
        throw file_io_exception(msg.str());
    }

    string line;
    while(getline(infile, line)) {
        // remove leading & trailing whitespace:
        trim_all(line);
        // only consider lines with length > 0:
        if(line.empty()) {
            continue;
        }
        // split all remaining lines into fields, add them to the string matrix:
        vector<string> fields;
        split(fields, line, is_any_of("="));
        cout << fields[0] << "=" << fields[1] << endl;
        // set the meta data:
        ealib::put(fields[0], fields[1], ea.md());
    }

    cout << "initializing..." << endl;
    ea.initialize();
    cout << "adding events..." << endl;
    ealib::add_event<ealib::datafiles::emscript>(ea);
    cout << "generating initial population..." << endl;
    ealib::generate_initial_population(ea);
    cout << "advancing epoch..." << endl;
    ea.lifecycle().advance_all(ea);

    return 0;
}
