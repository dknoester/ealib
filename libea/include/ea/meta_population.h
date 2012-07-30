/* meta_population.h
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
#ifndef _EA_META_POPULATION_H_
#define _EA_META_POPULATION_H_

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/iterator/indirect_iterator.hpp>
#include <limits>

#include <ea/initialization.h>
#include <ea/interface.h>
#include <ea/meta_data.h>
#include <ea/events.h>
#include <ea/rng.h>

namespace ea {
    
    template <typename EA,
    typename Initializer=initialization::all_subpopulations,
    template <typename> class EventHandler=event_handler,
	typename MetaData=meta_data,
	typename RandomNumberGenerator=ea::default_rng_type>
    class meta_population {
    public:
        //! Embedded EA type.
        typedef EA ea_type;
        //! Type of individual held by subpopulations.
        typedef typename ea_type::individual_type individual_type;
        //! Type of population used by subpopulations.
        typedef typename ea_type::population_type population_type;        
        //! Meta-population initializer type.
        typedef Initializer initializer_type;
        //! Event handler.
        typedef EventHandler<meta_population> event_handler_type;
        //! Meta-data type.
        typedef MetaData md_type;
        //! Random number generator type.
        typedef RandomNumberGenerator rng_type;        
        //! Pointer to embedded EA type.
        typedef boost::shared_ptr<ea_type> ea_type_ptr;
        //! Container type for EAs.
        typedef std::vector<ea_type_ptr> ea_container_type;
        //! Iterator for embedded EAs.
        typedef boost::indirect_iterator<typename ea_container_type::iterator> iterator;
        //! Const iterator for embedded EAs.
        typedef boost::indirect_iterator<typename ea_container_type::const_iterator> const_iterator;
        //! Iterator for embedded EAs.
        typedef boost::indirect_iterator<typename ea_container_type::reverse_iterator> reverse_iterator;
        //! Const iterator for embedded EAs.
        typedef boost::indirect_iterator<typename ea_container_type::const_reverse_iterator> const_reverse_iterator;
        
        //! Construct a meta-population EA.
        meta_population() : _update(0) {
        }
        
        //! Accessor for the random number generator.
        rng_type& rng() { return _rng; }

        //! Accessor for this EA's meta-data.
        md_type& md() { return _md; }

        //! Returns the event handler.
        event_handler_type& events() { return _events; }

        //! Return the number of embedded EAs.
        std::size_t size() const {
            return _eas.size();
        }
        
        //! Return the n'th embedded EAs.
        ea_type& operator[](std::size_t n) {
            return *_eas[n];
        }
        
        //! Returns a begin iterator to the embedded EAs.
        iterator begin() {
            return iterator(_eas.begin());
        }
        
        //! Returns an end iterator to the embedded EAs.
        iterator end() {
            return iterator(_eas.end());
        }

        //! Returns a begin iterator to the embedded EAs (const-qualified).
        const_iterator begin() const {
            return const_iterator(_eas.begin());
        }
        
        //! Returns an end iterator to the embedded EAs (const-qualified).
        const_iterator end() const {
            return const_iterator(_eas.end());
        }
        
        //! Returns a reverse begin iterator to the embedded EAs.
        reverse_iterator rbegin() {
            return reverse_iterator(_eas.rbegin());
        }
        
        //! Returns an reverse end iterator to the embedded EAs.
        reverse_iterator rend() {
            return reverse_iterator(_eas.rend());
        }
        
        //! Returns a reverse begin iterator to the embedded EAs (const-qualified).
        const_reverse_iterator rbegin() const {
            return const_reverse_iterator(_eas.rbegin());
        }
        
        //! Returns an reverse end iterator to the embedded EAs (const-qualified).
        const_reverse_iterator rend() const {
            return const_reverse_iterator(_eas.rend());
        }

        //! Initialize all the embedded EAs.
        void initialize() {
            for(unsigned int i=0; i<get<META_POPULATION_SIZE>(*this); ++i) {
                ea_type_ptr p(new ea_type());
                p->md() = md();
                p->rng().reset(rng()(std::numeric_limits<int>::max()));
                p->initialize();
                _eas.push_back(p);
            }
        }        
        
        //! Advance the epoch of this EA by n updates.
        void advance_epoch(std::size_t n) {
            // calculate initial fitness for all of our embedded EAs:
            for(iterator i=begin(); i!=end(); ++i) {
                calculate_fitness(i->population().begin(), i->population().end(), *i);
            }

            // update all the EAs:
            for( ; n>0; --n) {
                update();
            }
            
            // record end-of-epoch stats:
            for(iterator i=begin(); i!=end(); ++i) {
                i->events().record_statistics(*i);
                i->events().end_of_epoch(*i); // don't checkpoint!
            }

            _events.record_statistics(*this);
            _events.end_of_epoch(*this); // checkpoint!
        }
        
        //! Advance this EA by one update.
        void update() {
            _events.record_statistics(*this);
            for(iterator i=begin(); i!=end(); ++i) {
                i->update();
            }
            ++_update;
            _events.end_of_update(*this);
        }        
        
        //! Returns the current update of this EA.
        unsigned long current_update() {
            return _update;
        }

    protected:
        unsigned long _update; //!< Meta-population update.
        rng_type _rng; //!< Random number generator.
        meta_data _md; //!< Meta-data for the meta-population.
        event_handler_type _events; //!< Event handler.        
        ea_container_type _eas; //!< List of EAs in this meta-population.

    private:
        friend class boost::serialization::access;
        
		template<class Archive>
		void save(Archive & ar, const unsigned int version) const {
            ar & boost::serialization::make_nvp("update", _update);
            ar & boost::serialization::make_nvp("rng", _rng);
            ar & boost::serialization::make_nvp("meta_data", _md);

            std::size_t s = size();
            ar & boost::serialization::make_nvp("meta_population_size", s);
            for(const_iterator i=begin(); i!=end(); ++i) {
                ar & boost::serialization::make_nvp("subpopulation", *i);
            }
		}
		
		template<class Archive>
		void load(Archive & ar, const unsigned int version) {
            ar & boost::serialization::make_nvp("update", _update);
            ar & boost::serialization::make_nvp("rng", _rng);
            ar & boost::serialization::make_nvp("meta_data", _md);

            std::size_t s;
            ar & boost::serialization::make_nvp("meta_population_size", s);
            for(std::size_t i=0; i<s; ++i) {
                ea_type_ptr p(new ea_type());
                ar & boost::serialization::make_nvp("subpopulation", *p);
                _eas.push_back(p);
            }
		}
		BOOST_SERIALIZATION_SPLIT_MEMBER();
    };

}

#endif
