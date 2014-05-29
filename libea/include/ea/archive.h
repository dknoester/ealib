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
    
    //! Loads an archived population from input stream is into ea.
    template <typename EA>
    void load_archive(std::istream& in, EA& ea) {
        boost::archive::xml_iarchive ia(in);
        ia >> boost::serialization::make_nvp("population", ea.population());
    }
    
    //! Loads an archived population from file filename into ea.
    template <typename EA>
    void load_archive(const std::string& filename, EA& ea) {
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
            load_archive(f,ea);
        } else {
            load_archive(ifs,ea);
        }
        std::cerr << "done." << std::endl;
    }
    
    //! Save a population archive from ea to output stream os.
    template <typename EA>
    void save_archive(std::ostream& os, EA& ea) {
        boost::archive::xml_oarchive oa(os);
        oa << boost::serialization::make_nvp("population", ea.population());
    }
    
    //! Save a population archive from ea to file filename.
    template <typename EA>
    void save_archive(const std::string& filename, EA& ea) {
        std::ofstream ofs(filename.c_str());
        if(!ofs.good()) {
            throw file_io_exception("could not open " + filename + " for writing.");
        }
        save_archive(ofs,ea);
    }
    
} // ea

#endif
