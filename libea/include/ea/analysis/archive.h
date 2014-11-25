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
	LIBEA_MD_DECL(ARCHIVE_N, "ea.archive.n", int);
	
    namespace analysis {
		
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
		
		/*! Merge the contents of two archives.
		 
		 If either archive does not exist, an exception is thrown.
		 */
		LIBEA_ANALYSIS_TOOL(merge_archives) {
			// load the input archive:
			typename EA::population_type input;
			archive::load(get<ARCHIVE_INPUT>(ea), input, ea);

			// load the output archive:
			typename EA::population_type output;
			archive::load(get<ARCHIVE_OUTPUT>(ea), output, ea);
			
			output.insert(output.end(), input.begin(), input.end());
			archive::save(get<ARCHIVE_OUTPUT>(ea), output, ea);
		}

		/*! List the individuals in an archive.
		 */
		LIBEA_ANALYSIS_TOOL(list_individuals) {
			// load the input archive:
			typename EA::population_type input;
			archive::load(get<ARCHIVE_INPUT>(ea), input, ea);
			ea.population().swap(input);
			
			std::cout << "name,fitness" << std::endl;
			for(typename EA::iterator i=ea.begin(); i!=ea.end(); ++i) {
				std::cout << get<IND_UNIQUE_NAME>(*i) << "," << ealib::fitness(*i,ea) << std::endl;
			}
		}

		/*! Remove a named individual from an archive.
		 */
		LIBEA_ANALYSIS_TOOL(remove_individual) {
			// load the input archive:
			typename EA::population_type input;
			archive::load(get<ARCHIVE_INPUT>(ea), input, ea);
			ea.population().swap(input);
			
			// find the named individual, and erase it:
			typename EA::iterator ind = find_by_name(get<IND_UNIQUE_NAME>(ea), ea);
			if(ind == ea.end()) {
				throw bad_argument_exception("Could not find individual with name " + get<IND_UNIQUE_NAME>(ea));
			}
			ea.erase(ind);
			
			archive::save(get<ARCHIVE_INPUT>(ea), ea.population(), ea);
		}

		/*! Recalculate fitness for all the individuals in an archive.
		 */
		LIBEA_ANALYSIS_TOOL(recalculate_fitnesses) {
			// load the input archive:
			typename EA::population_type input;
			archive::load(get<ARCHIVE_INPUT>(ea), input, ea);
			ea.population().swap(input);
			
			ealib::recalculate_fitness(ea.begin(), ea.end(), ea);
			archive::save(get<ARCHIVE_INPUT>(ea), ea.population(), ea);
		}
		
		/*! Rename all individuals in an archive.
		 */
		LIBEA_ANALYSIS_TOOL(rename_individuals) {
			// load the input archive:
			typename EA::population_type input;
			archive::load(get<ARCHIVE_INPUT>(ea), input, ea);
			ea.population().swap(input);
			
			for(typename EA::iterator i=ea.begin(); i!=ea.end(); ++i) {
				put<IND_UNIQUE_NAME>(ea.rng().uuid(), *i);
			}
			archive::save(get<ARCHIVE_INPUT>(ea), ea.population(), ea);
		}

	} // analysis
} // ea

#endif
