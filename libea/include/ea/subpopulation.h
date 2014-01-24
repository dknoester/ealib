/* subpopulation.h
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
#ifndef _EA_SUBPOPULATION_H_
#define _EA_SUBPOPULATION_H_

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/shared_ptr.hpp>

#include <ea/fitness_functions/constant.h>
#include <ea/meta_data.h>
#include <ea/phenotype.h>
#include <ea/traits.h>

namespace ealib {
    
    /*! Subpopulation class for metapopulation EAs.
     
     A subpopulation is an adaptor that makes an EA suitable for use as an
     individual in a metapopulation EA.
     */
	template
    < typename EA
    , typename FitnessFunction=constant
    , typename Phenotype=EA
    , typename Encoding=directS
    , template <typename> class Traits=default_traits
    > class subpopulation {
    public:
        //! Subpopulation EA type.
        typedef EA ea_type;
        //! Representation type (synonym for EA).
        typedef ea_type representation_type;
        //! Fitness function type.
        typedef FitnessFunction fitness_function_type;
        //! Fitness value type for this individual.
        typedef typename FitnessFunction::fitness_type fitness_type;
        //! Phenotype for this individual.
        typedef Phenotype phenotype_type;
        //! Encoding of this individual.
        typedef Encoding encoding_type;
        //! Pointer to this individual.
        typedef boost::shared_ptr<subpopulation> individual_ptr_type;
        //! Traits for this individual.
        typedef Traits<subpopulation> traits_type;
        //! Phenotype pointer type.
        typedef typename traits_type::phenotype_ptr_type phenotype_ptr_type;
        //! Meta-data type.
        typedef typename ea_type::md_type md_type;
        //! Iterator over this EA's population.
        typedef typename ea_type::iterator iterator;
        //! Const iterator over this EA's population.
        typedef typename ea_type::const_iterator const_iterator;
        //! Reverse iterator over this EA's population.
        typedef typename ea_type::reverse_iterator reverse_iterator;
        //! Const reverse iterator over this EA's population.
        typedef typename ea_type::const_reverse_iterator const_reverse_iterator;

        //! Default constructor.
        subpopulation() {
        }
        
        //! Constructor.
        subpopulation(const ea_type& ea) : _ea(ea) {
        }
        
        //! Returns this subpopulation's underlying EA.
        ea_type& ea() { return _ea; }
        
        //! Returns this subpopulation's underlying EA (const-qualified).
        const ea_type& ea() const { return _ea; }
        
        //! Returns this subpopulation's representation (synonym for EA).
		representation_type& repr() { return _ea; }
        
		//! Returns this subpopulation's representation (const-qualified; synonym for EA).
		const representation_type& repr() const { return _ea; }
        
        //! Returns this subpopulation's fitness.
        fitness_type& fitness() { return _fitness; }
        
        //! Returns this subpopulation's fitness (const-qualified).
        const fitness_type& fitness() const { return _fitness; }

        //! Returns this subpopulation's meta-data.
        md_type& md() { return _ea.md(); }
        
        //! Returns this subpopulation's meta-data (const-qualified).
        const md_type& md() const { return _ea.md(); }
        
        //! Returns this subpopulation's traits.
        traits_type& traits() { return _traits; }
        
        //! Returns this individual's traits (const-qualified).
        const traits_type& traits() const { return _traits; }
        
        //! Accessor for the population model object.
        typename ea_type::population_type& population() { return _ea.population(); }
        
        //! Return the number of individuals in this EA.
        std::size_t size() const { return _ea.size(); }

        //! Return the n'th individual in the population.
        typename ea_type::individual_type& operator[](std::size_t n) { return _ea[n]; }
        
        //! Returns a begin iterator to the population.
        iterator begin() { return _ea.begin(); }
        
        //! Returns an end iterator to the population.
        iterator end() { return _ea.end(); }
        
        //! Returns a begin iterator to the population (const-qualified).
        const_iterator begin() const { return _ea.begin(); }
        
        //! Returns an end iterator to the population (const-qualified).
        const_iterator end() const { return _ea.end(); }
        
        //! Returns a reverse begin iterator to the population.
        reverse_iterator rbegin() { return _ea.rbegin(); }
        
        //! Returns a reverse end iterator to the population.
        reverse_iterator rend() { return _ea.rend(); }
        
        //! Returns a reverse begin iterator to the population (const-qualified).
        const_reverse_iterator rbegin() const { return _ea.rbegin(); }
        
        //! Returns a reverse end iterator to the population (const-qualified).
        const_reverse_iterator rend() const { return _ea.rend(); }

        //! Inserts individual x into the population before pos.
        void insert(iterator pos, typename ea_type::individual_ptr_type x) { _ea.insert(pos, x); }
        
        //! Inserts individuals [f,l) into the population before pos.
        template <typename InputIterator>
        void insert(iterator pos, InputIterator f, InputIterator l) { _ea.insert(pos, f, l); }
        
        //! Erases the given individual from the population.
        void erase(iterator i) { _ea.erase(i); }
        
        //! Erases the given range from the population.
        void erase(iterator f, iterator l) { _ea.erase(f, l); }
        
        //! Erases all individuals in this EA.
        void clear() { _ea.clear(); }

    protected:
        ea_type _ea; //!< This subpopulation's EA.
        fitness_type _fitness; //!< This subpopulation's fitness.
        traits_type _traits; //!< This subpopulation's traits.
        
    private:
        //! Serialization.
        friend class boost::serialization::access;
        template <class Archive>
        void serialize(Archive& ar, const unsigned int version) {
            ar & boost::serialization::make_nvp("ea", _ea);
            ar & boost::serialization::make_nvp("fitness", _fitness);
		}
    };

} // ealib

#endif
