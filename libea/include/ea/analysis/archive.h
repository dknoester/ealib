/* archive.h
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

#ifndef _EA_ANALYSIS_ARCHIVE_H_
#define _EA_ANALYSIS_ARCHIVE_H_

#include <algorithm>
#include <boost/filesystem.hpp>

#include <ea/analysis.h>
#include <ea/analysis/dominant.h>
#include <ea/archive.h>
#include <ea/comparators.h>
#include <ea/exceptions.h>

namespace ealib {
    namespace analysis {
        
        /*! Analysis tool to append a dominant to a population archive.
         
         The dominant is drawn from the loaded checkpoint, while the archive
         is set via ANALYSIS_INPUT and ANALYSIS_OUTPUT.
         */
        LIBEA_ANALYSIS_TOOL(archive_dominant) {
            const std::string& input = get<ANALYSIS_INPUT>(ea);
            const std::string& output = get<ANALYSIS_OUTPUT>(ea);
            
            typename EA::population_type archive;
            if(boost::filesystem::exists(input)) {
                archive::load(input, archive, ea);
            }
            
            archive.insert(archive.end(), ea.copy_individual(*dominant(ea)));
            archive::save(output, archive, ea);
        }
        
        /*! Trim an archive down to the top ANALYSIS_N best-ranked individuals.
         */
        LIBEA_ANALYSIS_TOOL(trim_archive) {
            const std::string& input = get<ANALYSIS_INPUT>(ea);
            const std::string& output = get<ANALYSIS_OUTPUT>(ea);
            
			if(!boost::filesystem::exists(input)) {
				throw bad_argument_exception("could not open file: " + input);
			}
			
			typename EA::population_type archive;
            archive::load(input, archive, ea);

			int nerase = archive.size() - get<ANALYSIS_N>(ea);
			
			if(nerase > 0) {
				std::sort(archive.begin(), archive.end(), comparators::fitness<EA>(ea));
				archive.erase(archive.begin(), archive.begin() + nerase);
                archive::save(output, archive, ea);
			}
        }
        
    } // analysis
} // ea

#endif
