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

#ifndef _EA_ARCHIVE_H_
#define _EA_ARCHIVE_H_

#include <boost/regex.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <iostream>
#include <fstream>
#include <sstream>

namespace ealib {
    namespace archive {
        /* Archives are defined as serialized populations.  They are not
         entire EAs, as with a checkpoint, because we might want different EAs
         to load up the same archive.  The goal is that, as long as the 
         individuals are the same type, an archive should be "cross-platform."
         
         (The EA parameter gets passed in to all of these functions for its
         types.)
         */
        
        //! Loads an archived population from input stream is into pop.
        template <typename EA>
        void load(std::istream& in, typename EA::population_type& pop, EA& ea) {
            boost::archive::xml_iarchive ia(in);
            ia >> boost::serialization::make_nvp("population", pop);
        }
        
        //! Loads an archived population from file filename into pop.
        template <typename EA>
        void load(const std::string& filename, typename EA::population_type& pop, EA& ea) {
            std::ifstream ifs(filename.c_str());
            if(!ifs.good()) {
                throw file_io_exception("could not open " + filename + " for reading.");
            }
            std::cerr << "loading " << filename << "... ";
            
            // is this a gzipped file?  test by checking file extension...
            static const boost::regex e(".*\\.gz$");
            if(boost::regex_match(filename, e)) {
                namespace bio = boost::iostreams;
                bio::filtering_stream<bio::input> f;
                f.push(bio::gzip_decompressor());
                f.push(ifs);
                load(f, pop, ea);
            } else {
                load(ifs, pop, ea);
            }
            std::cerr << "done." << std::endl;
        }
        
        //! Save a population archive from pop to output stream os.
        template <typename EA>
        void save(std::ostream& os, typename EA::population_type& pop, EA& ea) {
            boost::archive::xml_oarchive oa(os);
            oa << boost::serialization::make_nvp("population", pop);
        }
        
        //! Save a population archive from pop to file filename.
        template <typename EA>
        void save(const std::string& filename, typename EA::population_type& pop, EA& ea) {
            std::ofstream ofs(filename.c_str());
            if(!ofs.good()) {
                throw file_io_exception("could not open " + filename + " for writing.");
            }
            save(ofs, pop, ea);
        }
        
    } // archive
} // ea

#endif
