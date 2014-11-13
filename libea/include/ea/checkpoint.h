/* checkpoint.h
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
#ifndef _EA_CHECKPOINT_H_
#define _EA_CHECKPOINT_H_

#include <ea/metadata.h>

namespace ealib {
	
	LIBEA_MD_DECL(CHECKPOINT_OFF, "ea.run.checkpoint_off", int);
	LIBEA_MD_DECL(CHECKPOINT_NAME, "ea.run.checkpoint_name", std::string);
	
} // ealib

/* The mess below is to support a version of ealib that doesn't link against
 boost::serialization.  For example, the emscripten version of ealib doesn't
 currently support serialization (indeed, it won't compile even if we include
 the serialization headers).
 
 So, in the event that serialization needs to be removed, simply define
 LIBEA_CHECKPOINT_OFF, and the checkpoint stubs will still be there (thus things
 will compile), but you won't need to link against boost::serialization.
 */
#ifdef LIBEA_CHECKPOINT_OFF

namespace ealib {
	namespace checkpoint {
		template <typename EA> void load(std::istream& in, EA& ea) { }
		template <typename EA> void load(const std::string& filename, EA& ea) { }
        template <typename EA> void load(const std::string& filename, const metadata& md, EA& ea) { }
		template <typename EA> void save(std::ostream& out, EA& ea) { }
		template <typename EA> void save(const std::string& filename, EA& ea) { }
		template <typename EA> void save(EA& ea) { }
	} // checkpoint
} // ealib

#else

#include <boost/regex.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <iostream>
#include <fstream>
#include <sstream>

#include <ea/exceptions.h>

namespace ealib {
	namespace checkpoint {
		
		//! Load an EA from the given input stream.
		template <typename EA>
		void load(std::istream& in, EA& ea, const metadata& md=metadata()) {
			boost::archive::xml_iarchive ia(in);
			ia >> BOOST_SERIALIZATION_NVP(ea);
			ea.initialize(md);
		}
		
		//! Load an EA from the given checkpoint file.
		template <typename EA>
		void load(const std::string& filename, EA& ea, const metadata& md=metadata()) {
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
				load(f, ea, md);
			} else {
				load(ifs, ea, md);
			}
			std::cerr << "done." << std::endl;
		}
		
		//! Save an EA to the given output stream.
		template <typename EA>
		void save(std::ostream& out, EA& ea) {
			boost::archive::xml_oarchive oa(out);
			oa << BOOST_SERIALIZATION_NVP(ea);
		}
		
		//! Save an EA to the given checkpoint file.
		template <typename EA>
		void save(const std::string& filename, EA& ea) {
			std::ofstream ofs(filename.c_str());
			if(!ofs.good()) {
				throw file_io_exception("could not open " + filename + " for writing.");
			}
			save(ofs,ea);
		}
		
		//! Save an EA to a generated checkpoint file.
		template <typename EA>
		void save(EA& ea) {
            if(!get<CHECKPOINT_OFF>(ea,0)) {
                std::string fname;
                if(exists<CHECKPOINT_NAME>(ea)) {
                    fname = get<CHECKPOINT_NAME>(ea);
                } else {
                    std::ostringstream filename;
                    filename << "checkpoint-" << ea.current_update() << ".xml";
                    fname = filename.str();
                }
                save(fname, ea);
            }
		}
		
	} // checkpoint
} // ealib

#endif
#endif