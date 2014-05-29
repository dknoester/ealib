/* dominant.h
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

#ifndef _EA_ANALYSIS_DOMINANT_H_
#define _EA_ANALYSIS_DOMINANT_H_

#include <algorithm>
#include <boost/filesystem.hpp>

#include <ea/analysis.h>
#include <ea/archive.h>
#include <ea/comparators.h>

namespace ealib {
    namespace analysis {
        
        //! Returns an iterator to the dominant (most fit) individual in ea.
        template <typename EA>
        typename EA::iterator dominant(EA& ea) {
            return std::max_element(ea.begin(), ea.end(), comparators::fitness<EA>(ea));
        }
        
        /*! Analysis tool to append a dominant to a population archive.
         
         The dominant is drawn from the loaded checkpoint, while the archive
         is set via ANALYSIS_INPUT and ANALYSIS_OUTPUT.
         */
        LIBEA_ANALYSIS_TOOL(archive_dominant) {
            const std::string& input = get<ANALYSIS_INPUT>(ea);
            const std::string& output = get<ANALYSIS_OUTPUT>(ea);
            
            EA archive;
            if(boost::filesystem::exists(input)) {
                load_archive(input, archive);
            }
            
            archive.insert(archive.end(), *dominant(ea));
            save_archive(output, archive);
        }
        
    } // analysis
} // ea

#endif
