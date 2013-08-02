/* lifecycle.h
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
#ifndef _EA_LIFECYCLE_H_
#define _EA_LIFECYCLE_H_

#include <boost/regex.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <iostream>
#include <fstream>
#include <sstream>

#include <ea/algorithm.h>
#include <ea/meta_data.h>


namespace ealib {
    
    /*! At the conceptual level, here are the states and actions associated with 
     an EA's lifecycle.  Where appropriate, these states correspond to methods
     defined below.
     
     There are two main paths to get to the "ready-to-run" state, a new EA and an
     EA loaded from a checkpoint.  You could handle these pieces yourself, or
     use the handy "prepare_*" methods defined below.  (Use the prepare methods.)
     
     After calling prepare_*, use advance_epoch to run the EA for a specified number
     of updates.
     
     object construction
     |
     v
     configuration (cannot depend on meta-data)
     |     \
     |      v
     |      meta-data assignment
     |          |
     v          |
     load       |
     |          |
     override   |
     meta-data? |
     |          |
     v          v
     initialization (final resource preparation, event attachment, etc.)
     |          |
     |    initial_population
     |        |
     v        v
     begin epoch
     |
     v
     update <-
     |  \___ /
     |
     end epoch
     |
     v
     save
     */
    namespace lifecycle {
        
        //! Load an EA from the given input stream.
        template <typename EA>
        void load_checkpoint(std::istream& in, EA& ea) {
            boost::archive::xml_iarchive ia(in);
            ia >> BOOST_SERIALIZATION_NVP(ea);
        }
        
        //! Load an EA from the given checkpoint file.
        template <typename EA>
        void load_checkpoint(const std::string& filename, EA& ea) {
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
                load_checkpoint(f,ea);
            } else {
                load_checkpoint(ifs,ea);
            }
            std::cerr << "done." << std::endl;
        }
        
        //! Save an EA to the given output stream.
        template <typename EA>
        void save_checkpoint(std::ostream& out, EA& ea) {
            boost::archive::xml_oarchive oa(out);
            oa << BOOST_SERIALIZATION_NVP(ea);
        }

        //! Save an EA to the given checkpoint file.
        template <typename EA>
        void save_checkpoint(const std::string& filename, EA& ea) {
            std::ofstream ofs(filename.c_str());
            if(!ofs.good()) {
                throw file_io_exception("could not open " + filename + " for writing.");
            }
            save_checkpoint(ofs,ea);
        }
        
        //! Save an EA to a generated checkpoint file.
        template <typename EA>
        void save_checkpoint(EA& ea) {
            std::ostringstream filename;
            filename << get<CHECKPOINT_PREFIX>(ea) << "-" << ea.current_update() << ".xml";
            save_checkpoint(filename.str(), ea);
        }
        
        /*! Convenience method to fast-forward a newly constructed EA to a ready-to-run
         state.
         */
        template <typename EA>
        void prepare_new(EA& ea) {
            ea.configure();
            ea.initialize();
            ea.initial_population();
        }
        
        /*! Convenience method to fast-forward a newly constructed EA to a ready-to-run
         state, given meta-data.
         */
        template <typename EA>
        void prepare_new(EA& ea, meta_data& md) {
            ea.configure();
            ea.md() += md;
            ea.initialize();
            ea.initial_population();
        }
        
        /*! Convenience method to fast-forward a newly constructed EA to a ready-to-run
         state using a checkpoint.
         */
        template <typename Checkpoint, typename EA>
        void prepare_checkpoint(Checkpoint& cp, EA& ea) {
            ea.configure();
            load_checkpoint(cp, ea);
            ea.initialize();
        }
        
        /*! Convenience method to fast-forward a newly constructed EA to a ready-to-run
         state using a checkpoint, given meta-data.
         */
        template <typename Checkpoint, typename EA>
        void prepare_checkpoint(Checkpoint& cp, EA& ea, meta_data& md) {
            ea.configure();
            load_checkpoint(cp, ea);
            ea.md() += md;
            ea.initialize();
        }
        
        /*! Advance the EA by one epoch of n updates.
         */
        template <typename EA>
        void advance_epoch(int n, EA& ea) {
            ea.begin_epoch();
            for( ; n>0; --n) {
                ea.update();
            }
            ea.end_epoch();
        }
        
    } // lifecycle
} // ealib

#endif
