/* cartesian.h
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
#ifndef _EA_LSYS_PYTHON_H_
#define _EA_LSYS_PYTHON_H_

#include <fstream>

namespace ealib {
    namespace lsys {

        /*! Simple 2D coordinate system that writes Python plotting commands to
         a file.
         */
        struct python2 {
            //! Constructor.
            python2(const std::string& filename) {
                _out.open(filename.c_str());
                _out << "import pylab as pl" << std::endl;
                _out << "import matplotlib as mp" << std::endl << std::endl;
            }
            
            //! Destructor.
            ~python2() {
                _out << "pl.show()" << std::endl;
                _out.close();
            }
            
            //! Add a line to this coordinate system.
            template <typename Point>
            void line(const Point& p1, const Point& p2, const std::string c="red") {
                _out << "pl.plot([" << p1(0) << "," << p2(0) << "], [" << p1(1) << "," << p2(1) << "], c=\"" << c << "\")" << std::endl;
            }
            
            //! Add a line to this coordinate system.
            template <typename Point>
            void gline(const Point& p1, const Point& p2, const std::string c="red") {
                _out << "pl.plot([" << bg::get<0>(p1) << "," << bg::get<0>(p2) << "], ["
                << bg::get<1>(p1) << "," << bg::get<1>(p2) << "], c=\"" << c << "\")" << std::endl;
            }
            
            //! Add a point to this coordinate system.
            template <typename Point>
            void point(const Point& p, const std::string c="red") {
                _out << "pl.plot([" << p(0) << "], [" << p(1) << "], 'o', markersize=3, c=\"" << c << "\")" << std::endl;
            }
            
            //! Add a point to this coordinate system.
            template <typename Point>
            void gpoint(const Point& p, const std::string c="red") {
                _out << "pl.plot([" << bg::get<0>(p) << "], [" << bg::get<1>(p) << "], 'o', markersize=3, c=\"" << c << "\")" << std::endl;
            }
            
            std::ofstream _out;
        };
        
    } // lsys
} // ealib

#endif

