/* encoding.h
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
#ifndef _EA_ENCODING_H_
#define _EA_ENCODING_H_

namespace ealib {
    
    // **** Warning: this is not yet well-developed.
    
    //! Indicates that the individual's genotype directly encodes the phenotype.
    struct directS { };
    
    /*! Indicates that the individaul's genotype indirectly encodes the phenotype
     (i.e., must be translated prior to fitness evaluation).
     */
    struct indirectS { };
    
    /*! Indicates that the individual's genotype generates the phenotype (i.e.,
     must be "run" to build the phenotype upon which fitness will be evaluated).
     */
    struct generativeS { };
    
    namespace detail {
        //!
        template <typename EA>
        typename EA::configuration_type::phenotype make_phenotype(typename EA::individual_type& ind, directS, EA& ea) {
            return ind.repr();
        }
        
        //!
        template <typename EA>
        typename EA::configuration_type::phenotype make_phenotype(typename EA::individual_type& ind, indirectS, EA& ea) {
            return ind.repr();
        }

        //!
        template <typename EA>
        typename EA::configuration_type::phenotype make_phenotype(typename EA::individual_type& ind, generativeS, EA& ea) {
            return ind.repr();
        }
    }
    
    /*! Called to decode a representation into a phenotype.
     
     The representation defines the genotype, e.g., a vector of integers.  The 
     encoding defines how that genotype is used during fitness evaluation.  For
     example, the genotype may directly encode the phenotype, as in the classic
     all-ones problem.
     */
    template <typename EA>
    typename EA::configuration_type::phenotype make_phenotype(typename EA::individual_type& ind, EA& ea) {
        return detail::make_phenotype(ind, typename EA::configuration_type::encoding_type(), ea);
    }
    
    
    
} // ealib

#endif
