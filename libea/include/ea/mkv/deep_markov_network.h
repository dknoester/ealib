/* markov_network.h
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
#ifndef _EA_MKV_DEEP_MARKOV_NETWORK_H_
#define _EA_MKV_DEEP_MARKOV_NETWORK_H_

#include <boost/shared_ptr.hpp>
#include <vector>

#include <ea/configuration.h>
#include <ea/cmdline_interface.h>
#include <ea/representations/circular_genome.h>
#include <ea/mutation.h>
#include <ea/rng.h>
#include <ea/mkv/markov_network.h>

namespace ealib {
    
    /*! Deep Markov Network class, which provides a layered / hiererachical structure
     of Markov Networks, a la Deep Learning.
     */
    template <typename StateType=int, typename UpdateFunction=binary_or<StateType>, typename RandomNumberGenerator=default_rng_type>
    class deep_markov_network {
    public:
        typedef StateType state_type; //!< Type for state variables.
        typedef UpdateFunction update_function_type; //!< Binary function that updates state variables.
        typedef RandomNumberGenerator rng_type; //!< Random number generator type.
        typedef markov_network<state_type,update_function_type,rng_type> markov_network_type; //!< Type for Markov network layer.
        typedef boost::shared_ptr<markov_network_type> markov_network_ptr; //!< Type for a network pointer.
        typedef std::vector<markov_network_ptr> markov_network_vector_type; //!< Type for list of network layers.
        
        //! Constructor.
        template <typename ForwardIterator>
        deep_markov_network(ForwardIterator f, ForwardIterator l, unsigned int seed=0) : _rng(seed) {
            resize(f,l);
        }
        
        //! Constructor.
        template <typename ForwardIterator>
        deep_markov_network(ForwardIterator f, ForwardIterator l, const rng_type& rng) : _rng(rng) {
            resize(f,l);
        }
        
        /*! Resize this network.
         
         \param f, l are a range of desc_types, which are 3-tuples describing nin,
         nout, and nhid for each layer of the deep network.
         */
        template <typename ForwardIterator>
        void resize(ForwardIterator f, ForwardIterator l) {
            _layers.clear();
            for( ; f!=l; ++f) {
                markov_network_ptr p(new markov_network_type(*f, _rng.seed()));
                _layers.push_back(p);
            }
        }
        
        //! Clears this network (resets all state variables).
        void clear() {
            for(typename markov_network_vector_type::iterator i=_layers.begin(); i!=_layers.end(); ++i) {
                (*i)->clear();
            }
        }
        
        //! Reset this network's rng.
        void reset(unsigned int seed) {
            _rng.reset(seed);
            for(typename markov_network_vector_type::iterator i=_layers.begin(); i!=_layers.end(); ++i) {
                (*i)->reset(_rng.seed());
            }
        }
        
        //! Retrieve the number of state variables in this network.
        std::size_t nlayers() const { return _layers.size(); }

        //! Retrieve the total number of gates in this network.
        std::size_t ngates() const {
            std::size_t c=0;
            for(typename markov_network_vector_type::const_iterator i=_layers.begin(); i!=_layers.end(); ++i) {
                c += (*i)->ngates();
            }
            return c;
        }

        //! Retrieve layer i.
        markov_network_type& operator[](std::size_t i) { return *_layers[i]; }
        
        //! Retrieve layer i.
        const markov_network_type& operator[](std::size_t i) const { return *_layers[i]; }
        
        //! Retrieve state i,j.
        int& operator()(std::size_t i, std::size_t j) { return (*_layers[i])(j); }
        
        //! Retrieve state i,j (const-qualified).
        const int& operator()(std::size_t i, std::size_t j) const { return (*_layers[i])(j); }
        
        //! Retrieve an iterator to the beginning of the output of the last layer.
        typename markov_network_type::iterator begin_output() { return _layers.back()->begin_output(); }
        
        //! Retrieve an iterator to the end of the outputs of the last layer.
        typename markov_network_type::iterator end_output() { return _layers.back()->end_output(); }
        
        //! Update each layer of this deep Markov network, assuming all inputs have been set.
        void update(std::size_t n=1) {
            for(typename markov_network_vector_type::iterator i=_layers.begin(); i!=_layers.end(); ++i) {
                (*i)->update(n);
            }
        }
        
        /*! Zero-copy update.
         
         \param f points to a list of RandomAccess, one for each layer.
         */
        template <typename ForwardIterator>
        void update(ForwardIterator f, std::size_t n=1) {
            for(typename markov_network_vector_type::iterator i=_layers.begin(); i!=_layers.end(); ++i, ++f) {
                (*i)->update(*f,n);
            }
        }
        
        /*! Zero-copy cascading update.
         */
        template <typename RandomAccess>
        void cascade_update(RandomAccess f, std::size_t n=1) {
            markov_network_ptr last=_layers[0];
            last->update(f,n);
            
            for(std::size_t i=1; i<_layers.size(); ++i) {
                _layers[i]->update(last->begin_output(), n);
                last = _layers[i];
            }
        }
        
        /*! Cascading update, assumes that input to layer zero has been set.
         */
        void cascade_update(std::size_t n=1) {
            cascade_update(_layers[0].begin_input(), n);
        }
        
    private:
        markov_network_vector_type _layers; //!< Vector of Markov network layers.
        rng_type _rng; //<! Random number generator.
    };
    

    namespace mkv {
        
        /*! Configuration object for EAs that use Markov Networks.
         */
        template <typename EA>
        struct deep_configuration : public abstract_configuration<EA> {
            typedef indirectS encoding_type;
            typedef deep_markov_network< > phenotype;
            typedef boost::shared_ptr<phenotype> phenotype_ptr;
            typedef std::vector<desc_type> desc_vector_type;
            
            //! Translate an individual's representation into a Markov Network.
            virtual phenotype_ptr make_phenotype(typename EA::individual_type& ind,
                                                 typename EA::rng_type& rng, EA& ea) {
                phenotype_ptr p(new phenotype(desc.begin(), desc.end(), rng.seed()));
                translate_genome(ind.repr(), start, translator, *p);
                return p;
            }
            
            //! Called as the first step of an EA's lifecycle.
            virtual void configure(EA& ea) {
            }
            
            //! Called to generate the initial EA population.
            virtual void initial_population(EA& ea) {
                generate_ancestors(mkv::ancestor(), get<POPULATION_SIZE>(ea), ea);
            }
            
            //! Called as the final step of EA initialization.
            virtual void initialize(EA& ea) {
                for(std::size_t i=0; i<get<MKV_LAYERS_N>(ea); ++i) {
                    desc.push_back(desc_type(get<MKV_INPUT_N>(ea), get<MKV_OUTPUT_N>(ea), get<MKV_HIDDEN_N>(ea)));
                }
            }
            
            //! Disable a gate type.
            void disable(gate_type g) {
                translator.disable(g);
            }
            
            desc_vector_type desc; //!< Description for the deep Markov network (# in, out, & hidden).
            start_codon start; //!< Start codon detector.
            deep_genome_translator translator; //!< Genome translator.
        };
        
    } // mkv
} // ealib

#endif
