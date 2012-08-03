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
        //! Type of individual held by this metapopulation.
        typedef EA individual_type;
        //! Individual pointer type.
        typedef boost::shared_ptr<individual_type> individual_ptr_type;
        //! Type of population container.
        typedef std::vector<individual_ptr_type> population_type;
        //! Meta-population initializer type.
        typedef Initializer initializer_type;
        //! Event handler.
        typedef EventHandler<meta_population> event_handler_type;
        //! Meta-data type.
        typedef MetaData md_type;
        //! Random number generator type.
        typedef RandomNumberGenerator rng_type;        
        //! Iterator for embedded EAs.
        typedef boost::indirect_iterator<typename population_type::iterator> iterator;
        //! Const iterator for embedded EAs.
        typedef boost::indirect_iterator<typename ea_container_type::const_iterator> const_iterator;
        //! Iterator for embedded EAs.
        typedef boost::indirect_iterator<typename ea_container_type::reverse_iterator> reverse_iterator;
        //! Const iterator for embedded EAs.
       typedef boost::indirect_iterator<typename ea_container_type::const_reverse_iterator> const_reverse_iterator;

        typedef boost::indirect_iterator<typename population_type::const_iterator> const_iterator;
        
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
            return _population.size();
        }

        //! Return the population.
        population_type& population() {
            return _population;
        }
        
        //! Return the n'th embedded EAs.
        individual_type& operator[](std::size_t n) {
            return *_population[n];
        }
        
        //! Returns a begin iterator to the embedded EAs.
        iterator begin() {
            return iterator(_population.begin());
        }
        
        //! Returns an end iterator to the embedded EAs.
        iterator end() {
            return iterator(_population.end());
        }

        //! Returns a begin iterator to the embedded EAs (const-qualified).
        const_iterator begin() const {
            return const_iterator(_population.begin());
        }
        
        //! Returns an end iterator to the embedded EAs (const-qualified).
        const_iterator end() const {
            return const_iterator(_population.end());
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
                individual_ptr_type p(new individual_type());
                p->md() = md();
                p->rng().reset(rng()(std::numeric_limits<int>::max()));
                p->initialize();
                _population.push_back(p);
            }
        }        
        
        //! Generates the initial population.
        void generate_initial_population() {
            for(iterator i=begin(); i!=end(); ++i) {
                i->generate_initial_population();
            }
        }
        
        //! Reset all populations.
        void reset() {
            for(iterator i=begin(); i!=end(); ++i) {
                i->reset();
            }
        }
        
        //! Advance the epoch of this EA by n updates.
        void advance_epoch(std::size_t n) {
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

        //! Perform any needed preselection.
        void preselect(population_type& src) {
        }

    protected:
        unsigned long _update; //!< Meta-population update.
        rng_type _rng; //!< Random number generator.
        meta_data _md; //!< Meta-data for the meta-population.
        event_handler_type _events; //!< Event handler.        
        population_type _population; //!< List of EAs in this meta-population.

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
                individual_ptr_type p(new individual_type());
                ar & boost::serialization::make_nvp("subpopulation", *p);
                _population.push_back(p);
            }
		}
		BOOST_SERIALIZATION_SPLIT_MEMBER();
    };

}

#endif
