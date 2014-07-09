/* lifecycle.h
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
#ifndef _EA_LIFECYCLE_H_
#define _EA_LIFECYCLE_H_

#ifndef LIBEA_CHECKPOINT_OFF
#include <boost/regex.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#endif
#include <iostream>
#include <fstream>
#include <sstream>

#include <ea/algorithm.h>
#include <ea/metadata.h>


namespace ealib {
    
	LIBEA_MD_DECL(RUN_UPDATES, "ea.run.updates", int);
	LIBEA_MD_DECL(RUN_EPOCHS, "ea.run.epochs", int);
    LIBEA_MD_DECL(CHECKPOINT_OFF, "ea.run.checkpoint_off", int);
	LIBEA_MD_DECL(CHECKPOINT_PREFIX, "ea.run.checkpoint_prefix", std::string);
    
    /*! At the conceptual level, this class defines the states and actions of
     an EA's lifecycle.  Where appropriate, these states correspond to methods
     defined below.
     
     There are two main paths to get to the "ready-to-run" state, a new EA and an
     EA loaded from a checkpoint.  See the prepare_* methods defined below.
     
     After calling prepare_*, use advance_epoch to run the EA for a specified number
     of updates.  By default, a checkpoint is created after every epoch.
     
     object construction
     |          |
     v          |
     load       |
     |          |
     v          v
     meta-data assignment
     |          |
     v          v
     initialize (final resource preparation, event attachment, etc.)
     |          |
     **after_initialization()**
     |          |
     |    initial_population
     |          |
     |          |
     |    **after_initial_population()**
     |          |
     v          v
     begin epoch
     |
     v
     update <-N-- stop? --Y-> end epoch ---> save
          \___ /
     */
    struct default_lifecycle {
        
        /*! Called after EA initialization.
         
         This is a good place to handle programmatic setup tasks.  E.g., adding
         instructions to a digital evolution ISA, loading external data files,
         and the like.
         */
        template <typename EA>
        void after_initialization(EA& ea) {
        }
        
        /*! Called after the initial population has been generated.
         
         */
        template <typename EA>
        void after_initial_population(EA& ea) {
        }
        
        //! Called to reset the state of this population.
        template <typename EA>
        void reset(EA& ea) {
        }
        
        //! Load an EA from the given input stream.
        template <typename EA>
        void load_checkpoint(std::istream& in, EA& ea) {
#ifndef LIBEA_CHECKPOINT_OFF
            boost::archive::xml_iarchive ia(in);
            ia >> BOOST_SERIALIZATION_NVP(ea);
#endif
        }
        
        //! Load an EA from the given checkpoint file.
        template <typename EA>
        void load_checkpoint(const std::string& filename, EA& ea) {
#ifndef LIBEA_CHECKPOINT_OFF
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
#endif
        }
        
        //! Save an EA to the given output stream.
        template <typename EA>
        void save_checkpoint(std::ostream& out, EA& ea) {
#ifndef LIBEA_CHECKPOINT_OFF
            boost::archive::xml_oarchive oa(out);
            oa << BOOST_SERIALIZATION_NVP(ea);
#endif
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
         state, given meta-data.
         */
        template <typename EA>
        void prepare_new(EA& ea, metadata& md) {
            ea.initialize(md);
            generate_initial_population(ea);
        }
        
        /*! Convenience method to fast-forward a newly constructed EA to a ready-to-run
         state using a checkpoint, given meta-data.
         
         Meta-data is assigned after checkpoint load to provide for overriding.
         */
        template <typename Checkpoint, typename EA>
        void prepare_checkpoint(Checkpoint& cp, EA& ea, metadata& md) {
            load_checkpoint(cp, ea);
            ea.initialize(md);
        }
        
        /*! Advance the EA by one epoch.
         */
        template <typename EA>
        void advance_epoch(EA& ea) {
            ea.begin_epoch();
            for(int n=get<RUN_UPDATES>(ea); n>0; --n) {
                ea.update();
                if(ea.stop()) {
                    break;
                }
            }
            ea.end_epoch();
        }
        
        //! Advance the EA by all configured epochs.
        template <typename EA>
        void advance_all(EA& ea) {
			for(int i=0; i<get<RUN_EPOCHS>(ea); ++i) {
                advance_epoch(ea);
                if(!get<CHECKPOINT_OFF>(ea,0)) {
                    std::ostringstream filename;
                    filename << get<CHECKPOINT_PREFIX>(ea) << "-" << ea.current_update() << ".xml";
                    save_checkpoint(filename.str(), ea);
                }
			}
		}
    };
    
} // ealib

#endif
