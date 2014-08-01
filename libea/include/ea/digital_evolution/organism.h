/* digital_evolution/organism.h 
 * 
 * This file is part of EALib.
 * 
 * Copyright 2014 David B. Knoester, Heather J. Goldsby.
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

#ifndef _EA_ORGANISM_H_
#define _EA_ORGANISM_H_

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/deque.hpp>
#include <boost/serialization/map.hpp>
#include <fstream>
#include <deque>
#include <vector>
#include <map>

#include <ea/digital_evolution/environment.h>
#include <ea/digital_evolution/hardware.h>
#include <ea/digital_evolution/schedulers.h>
#include <ea/metadata.h>

namespace ealib {
    
	/*! Digital organism.

	 */
    template
    < typename Traits
    > class organism {
	public:
        typedef hardware hardware_type;
        typedef hardware_type::genome_type genome_type;
        typedef hardware_type::mutation_operator_type mutation_operator_type;
        typedef Traits traits_type;
        typedef metadata md_type;
		typedef std::map<std::string, double> phenotype_type;
        typedef int io_type;
        typedef std::deque<io_type> iobuffer_type;
        
		//! Constructor.
		organism() : _priority(1.0), _alive(true) {
		}
        
		//! Constructor that builds an organism from a representation.
		organism(const genome_type& r) 
        : _hw(r), _priority(1.0), _alive(true) {
		}
        
        //! Copy constructor.
        organism(const organism& that) {
            _hw = that._hw;
            _priority = that._priority;
            _position = that._position;
            _alive = that._alive;
            _inputs = that._inputs;
            _outputs = that._outputs;
            _phenotype = that._phenotype;
            _md = that._md;
        }
        
        //! Assignment operator.
        organism& operator=(const organism& that) {
            if(this != &that) {
                _hw = that._hw;
                _priority = that._priority;
                _position = that._position;
                _alive = that._alive;
                _inputs = that._inputs;
                _outputs = that._outputs;
                _phenotype = that._phenotype;
                _md = that._md;
            }
            return *this;
        }

        //! Returns true if hardware(s) are equivalent.
        bool operator==(const organism& that) const {
            return (_hw == that._hw)
            && (_priority == that._priority)
            && (_position == that._position)
            && (_alive == that._alive)
            && (_inputs == that._inputs)
            && (_outputs == that._outputs)
            && (_phenotype == that._phenotype)
            && (_md == that._md);
        }
        
        //! Returns this organism's hardware.
        hardware_type& hw() { return _hw; }
        
        //! Returns this organism's hardware (const-qualified).
        const hardware_type& hw() const { return _hw; }

		//! Returns this organism's representation.
		genome_type& repr() { return _hw.repr(); }
        
		//! Returns this organism's representation (const-qualified).
		const genome_type& repr() const { return _hw.repr(); }

        //! Returns this organism's genome.
        genome_type& genome() { return _hw.repr(); }
        
        //! Returns this organism's priority.
		priority_type& priority() { return _priority; }
		
		//! Returns this organism's priority (const-qualified).
		const priority_type& priority() const { return _priority; }

        //! Returns this organism's position.
        position_type& position() { return _position; }

        //! Returns this organism's position (const-qualified).
        const position_type& position() const { return _position; }
        
        //! Returns true if this organism is alive, false otherwise.
        bool& alive() { return _alive; }
        
        //! Returns this organism's inputs.
        iobuffer_type& inputs() { return _inputs; }

        //! Returns this organism's outputs.
        iobuffer_type& outputs() { return _outputs; }

        //! Returns this organism's phenotype.
        phenotype_type& phenotype() { return _phenotype; }
        
        //! Returns this organism's meta data.
        metadata& md() { return _md; }
        
        //! Returns this organism's meta data (const-qualified).
        const metadata& md() const { return _md; }
        
        //! Returns this individual's traits.
        traits_type& traits() { return _traits; }
        
        //! Returns this individual's traits (const-qualified).
        const traits_type& traits() const { return _traits; }
        
        //! Execute this organism for n cycles.
        template <typename EA>
        inline void execute(std::size_t n, typename EA::individual_ptr_type p, EA& ea) {
            _hw.execute(n, p, ea);
        }

	protected:
        hardware_type _hw; //!< This organism's virtual hardware.
		priority_type _priority; //!< Used by the scheduler to prioritize this organism.
        position_type _position; //!< This organism's position in the environment.
        bool _alive; //!< Is this organism currently alive?
        iobuffer_type _inputs; //!< This organism's inputs.
        iobuffer_type _outputs; //!< This organism's outputs.
        phenotype_type _phenotype; //!< This organism's phenotype.
        metadata _md; //!< This organism's meta data.
        traits_type _traits; //!< This organism's traits.

	private:
		friend class boost::serialization::access;
		template<class Archive>
        void serialize(Archive & ar, const unsigned int version) {
            ar & boost::serialization::make_nvp("hardware", _hw);
            ar & boost::serialization::make_nvp("priority", _priority);
            ar & boost::serialization::make_nvp("position", _position);
            ar & boost::serialization::make_nvp("alive", _alive);
            ar & boost::serialization::make_nvp("inputs", _inputs);
            ar & boost::serialization::make_nvp("outputs", _outputs);
            ar & boost::serialization::make_nvp("phenotype", _phenotype);
            ar & boost::serialization::make_nvp("metadata", _md);
		}
	};
    
} // ealib

#endif
