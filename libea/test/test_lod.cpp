/* test_minimal.cpp
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

#include "test.h"
#include <ea/analysis.h>
#include <ea/cmdline_interface.h>
#include <ea/line_of_descent.h>

LIBEA_ANALYSIS_TOOL(test_population_lod_tool) {
    using namespace ealib;
    
    line_of_descent<EA> lod = lod_load(get<ANALYSIS_INPUT>(ea), ea);
    
    for(typename line_of_descent<EA>::iterator i=lod.begin(); i!=lod.end(); ++i) {
        std::cout << get<IND_NAME>(*i) << std::endl;
    }
}

template <typename EA>
class cli : public cmdline_interface<EA> {
public:
    virtual void gather_tools() {
        add_tool<test_population_lod_tool>(this);
    }
    
    virtual void gather_events(EA& ea) {
        add_event<lod_event>(ea);
    };
};

BOOST_AUTO_TEST_CASE(test_lod) {
    cli<all_ones_lod_ea> ea;
    
}
