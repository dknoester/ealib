/* phenotype.h
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
#ifndef _EA_PHENOTYPE_H_
#define _EA_PHENOTYPE_H_

namespace ealib {

    // **** Warning: this is not yet well-developed.

    /* The idea here is that some kinds of EAs require that an individual's genotype
     be converted to another form prior to fitness evaluation.  This is usually
     referred to as the "encoding type."
     
     Here we define three different encoding types: direct, indirect, and generative:
     */
    
    //! Indicates that the individual's genotype directly encodes the phenotype.
    struct directS { };
    
    /*! Indicates that the individaul's genotype indirectly encodes the phenotype
     (i.e., must be translated or generated prior to fitness evaluation).
     */
    struct indirectS { };
    
    /* These difference encoding types are used when make_phenotype() is called
     on an individual.  Note that the default is directS.
     
     To be clear about definitions:
     
     Genotype: That which makes up the genetic component of an individual; inherited.
     In EALib, genotypes == representation.
     
     Phenotype: That component of an individual that is evaluated by the fitness
     function; can be the genotype (direct), an object that was translated
     from the genotype (indirect), or even an object that was generated
     (generative).
     */
    
    namespace attr {

        //! Default phenotype attribute.
        template <typename EA>
        struct phenotype_attribute {
            
            //! Constructor.
            phenotype_attribute() {
            }
            
            //! Returns true if a phenotype is present.
            bool has_phenotype() {
                return _p != 0;
            }
            
            //! Retrieves a pointer reference to the phenotype.
            typename EA::configuration_type::phenotype_ptr& phenotype() {
                return _p;
            }

            /*! Pointer to the phenotype.
             
             In the case of a direct encoding, phenotype_ptr should be a plain-old
             pointer.  In **all** other cases, it should be an auto-deleting pointer
             like boost::shared_ptr< >.
             */
            typename EA::configuration_type::phenotype_ptr _p;
        };
        
    } // attr

    namespace detail {
        
        //! Direct encoding; returns a pointer to the individual's representation (its genotype).
        template <typename EA>
        typename EA::configuration_type::phenotype_ptr make_phenotype(typename EA::individual_type& ind, directS, EA& ea) {
            return &ind.repr();
        }
        
        //! Indirect encoding; returns a pointer to a phenotype that has been translated from the genotype.
        template <typename EA>
        typename EA::configuration_type::phenotype_ptr make_phenotype(typename EA::individual_type& ind, indirectS, EA& ea) {
            return ea.configuration().make_phenotype(ind,ea);
        }
        
        //! Indirect encoding; returns a pointer to a stochastic phenotype that has been translated from the genotype.
        template <typename EA>
        typename EA::configuration_type::phenotype_ptr make_phenotype(typename EA::individual_type& ind, indirectS,
                                                                      typename EA::rng_type& rng, EA& ea) {
            return ea.configuration().make_phenotype(ind,rng,ea);
        }

    } // detail
    
    //! Phenotype attribute accessor; lazily decodes a genotype into a phenotype.
    template <typename EA>
    typename EA::configuration_type::phenotype& phenotype(typename EA::individual_type& ind, EA& ea) {
        BOOST_CONCEPT_ASSERT((EvolutionaryAlgorithmConcept<EA>));

        if(!ind.attr().has_phenotype()) {
            typename EA::configuration_type::phenotype_ptr p=detail::make_phenotype(ind, typename EA::encoding_type(), ea);
            ind.attr().phenotype() = p;
        }

        return *ind.attr().phenotype();
    }
    
    //! Phenotype attribute accessor; lazily decodes a genotype into a stochastic phenotype.
    template <typename EA>
    typename EA::configuration_type::phenotype& phenotype(typename EA::individual_type& ind,
                                                          typename EA::rng_type& rng, EA& ea) {
        BOOST_CONCEPT_ASSERT((EvolutionaryAlgorithmConcept<EA>));
        
        if(!ind.attr().has_phenotype()) {
            typename EA::configuration_type::phenotype_ptr p=detail::make_phenotype(ind, typename EA::encoding_type(), rng, ea);
            ind.attr().phenotype() = p;
        }
        
        return *ind.attr().phenotype();
    }

} // ealib

#endif
