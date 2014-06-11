/* emscript_bound.cpp
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

 EMCC_FAST_COMPILER=0 ../emscripten/1.16.0/emcc --bind -I../libea/include \
 -I/usr/local/include emscript_bound.cpp -o logic9.js \
 -DLIBEA_CHECKPOINT_OFF -DBOOST_PARAMETER_MAX_ARITY=7
 
 The resulting logic9.js file can be loaded like so:
 
 <!doctype html>
 <html>
 <script src="processing.min.js"></script>
 <script src="logic9.js"></script>
 <script type="text/processing" data-processing-target="processing-canvas">
 void setup() {
 size(200, 200);
 background(100);
 stroke(255);
 ellipse(50, 50, 25, 25);
 ea = new Module.EvolutionaryAlgorithm();
 println('hello web!');
 println(ea.size());
 }
 </script>
 <canvas id="processing-canvas"> </canvas>
 </html>
 
 */
#include <boost/algorithm/string/trim_all.hpp>
#include <boost/algorithm/string/split.hpp>
#include <sstream>
#include <fstream>
#include <vector>
#include <string>

#include <emscripten/bind.h>
using namespace emscripten;

#include "logic9.h"

template <typename EA>
class emscript_interface {
public:

    //! Constructor.
    emscript_interface() {
    }
    
    //! Returns the number of individuals in the EA.
    std::size_t size() const { return _ea.size(); }
    
    //! Put metadata in the underlying EA.
    void put(const std::string& k, const std::string& v) {
        ealib::put(k, v, _ea.md());
    }
    
    //! Initialize the underlying EA.
    void initialize() {
        _ea.lifecycle().prepare_new(_ea);
    }
    
    //! Run the underlying EA.
    void run() {
        _ea.lifecycle().advance_all(_ea);
    }
    
protected:
    EA _ea; //!< Underlying EA.
};

//! Interface typedef.
typedef emscript_interface<ea_type> ea_interface;

EMSCRIPTEN_BINDINGS(bound_ea_interface) {
    class_<ea_interface>("EvolutionaryAlgorithm")
    .constructor< >()
    .function("initialize", &ea_interface::initialize)
    .function("size", &ea_interface::size)
    .function("put", &ea_interface::put)
    .function("run", &ea_interface::run);
}
