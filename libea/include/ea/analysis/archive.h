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

#include <ea/analysis.h>
#include <ea/analysis/dominant.h>
#include <ea/analysis/find_by_name.h>
#include <ea/archive.h>
#include <ea/comparators.h>
#include <ea/exceptions.h>
#include <ea/metadata.h>

namespace ealib {
	
	LIBEA_MD_DECL(ARCHIVE_INPUT, "ea.archive.input", std::string);
	LIBEA_MD_DECL(ARCHIVE_OUTPUT, "ea.archive.output", std::string);
	
    namespace analysis {
				
		/*! Copy a named individual from one archive to another.
		 
		 If the input archive does not exist, an exception is thrown.  If the 
		 output archive does not exist, it is created, otherwise the individual
		 is added to the output archive.
		 */
		LIBEA_ANALYSIS_TOOL(copy_individual) {
			// load the input archive:
			typename EA::population_type input;
			archive::load(get<ARCHIVE_INPUT>(ea), input, ea);
			ea.population().swap(input);
			
			typename EA::iterator ind = find_by_name(get<IND_UNIQUE_NAME>(ea), ea);
			
			if(ind == ea.end()) {
				throw bad_argument_exception("Could not find individual with name " + get<IND_UNIQUE_NAME>(ea));
			}
			
			// load the output archive, it it exists:
			typename EA::population_type output;
			archive::load_if(get<ARCHIVE_OUTPUT>(ea), output, ea);
			
			output.insert(output.end(), ea.copy_individual(*ind));
			archive::save(get<ARCHIVE_OUTPUT>(ea), output, ea);
		}
		
		/*! Archive a dominant individual from a checkpoint into an output archive.
		 
		 If the output archive does not exist, it is created.  If it does exist,
		 the dominant individual is added to it.
		 */
		LIBEA_ANALYSIS_TOOL(archive_dominant) {
			// load the output archive, it it exists:
			typename EA::population_type output;
			archive::load_if(get<ARCHIVE_OUTPUT>(ea), output, ea);
			
			// copy the dominant:
			typename EA::iterator ind = dominant(ea);
			output.insert(output.end(), ea.copy_individual(*ind));

			// save the output archive:
			archive::save(get<ARCHIVE_OUTPUT>(ea), output, ea);
		}
		
		
		/*! Remove a named individual from an archive.
		 */
		LIBEA_ANALYSIS_TOOL(remove_individual) {
			typename EA::population_type input;
			archive::load(get<ARCHIVE_INPUT>(ea), input, ea);
			ea.population().swap(input);
			
			typename EA::iterator i=find_by_name(get<IND_UNIQUE_NAME>(ea), ea);
			ea.erase(i);
			
			archive::save(get<ARCHIVE_INPUT>(ea), ea.population(), ea);
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
