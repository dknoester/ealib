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
#ifndef _MKV_DEEP_MARKOV_NETWORK_H_
#define _MKV_DEEP_MARKOV_NETWORK_H_

#include <boost/variant.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include <vector>
#include <numeric>
#include <algorithm>

#include <ea/algorithm.h>
#include <ea/rng.h>
#include <mkv/markov_network.h>


namespace mkv {
    namespace bl = boost::lambda;
    
    /*! Deep Markov Network class, which provides a layered / hiererachical structure
     of Markov Networks, a la Deep Learning.
     */
    class deep_markov_network : public std::vector<markov_network> {
    public:
        typedef std::vector<markov_network::desc_type> desc_type; //!< Type for geometry of Deep Markov Network.
        typedef std::vector<markov_network> base_type; //!< Base type container for Deep Markov Networks.
        typedef ealib::default_rng_type rng_type; //!< Random number generator type.

        //! Constructs a Deep Markov network with a given seed.
        deep_markov_network(const desc_type& desc, unsigned int seed=0) : _desc(desc), _rng(seed) {
            for(desc_type::iterator i=_desc.begin(); i!=_desc.end(); ++i) {
                push_back(markov_network(*i, _rng.seed()));
            }
        }
        
        //! Retrieve this network's underlying random number generator.
        rng_type& rng() { return _rng; }
        
        //! Retrieve the number of input state variables in this network.
        std::size_t ninput_states() const {
            return std::accumulate(begin(), end(), 0, bl::_1 += bl::bind(&markov_network::ninput_states, bl::_2));
        }
        
        //! Retrieve the number of output state variables in this network.
        std::size_t noutput_states() const {
            return std::accumulate(begin(), end(), 0, bl::_1 += bl::bind(&markov_network::noutput_states, bl::_2));
        }
        
        //! Retrieve the number of hidden state variables in this network.
        std::size_t nhidden_states() const {
            return std::accumulate(begin(), end(), 0, bl::_1 += bl::bind(&markov_network::nhidden_states, bl::_2));
        }
        
        //! Retrieve the number of state variables in this network.
        std::size_t nstates() const {
            return ninput_states() + noutput_states() + nhidden_states();
        }
        
        //! Retrieve the number of gates in this network.
        std::size_t ngates() const {
            return std::accumulate(begin(), end(), 0, bl::_1 += bl::bind(&markov_network::ngates, bl::_2));
        }
        
        //! Convenience method to access a specific gate.
        variant_gate_type& operator()(std::size_t i, std::size_t j) {
            return at(i).at(j);
        }
        
        //! Clear the network.
        void clear() {
            std::for_each(begin(), end(), bl::bind(&markov_network::clear, bl::_1));
        }
        
        //! Reset the network.
        void reset(int seed) {
            _rng.reset(seed);
            for(iterator i=begin(); i!=end(); ++i) {
                i->reset(_rng.seed());
            }
        }
        
        //! Rotate t and t-1 state vectors.
        void rotate() {
            std::for_each(begin(), end(), bl::bind(&markov_network::rotate, bl::_1));
        }
        
        //! Retrieve an iterator to the beginning of the svm outputs at time t in the last (highest-level) layer.
        markov_network::svm_type::iterator begin_output() { return rbegin()->begin_output(); }
        
        //! Retrieve an iterator to the end of the svm outputs at time t in the last (highest-level) layer.
        markov_network::svm_type::iterator end_output() { return rbegin()->end_output(); }

        //! Update the network n times per layer, with the top-level inputs given by f.
        template <typename RandomAccessIterator>
        void update(std::size_t n, RandomAccessIterator f) {
            if(size() == 0) { return; } // empty network, should warn
            
            for( ; n>0; --n) {
                rotate();
                
                markov_network& l0=operator[](0);
                detail::markov_network_update_visitor<RandomAccessIterator> l0v(l0, f);
                std::for_each(l0.begin(), l0.end(), boost::apply_visitor(l0v));
                
                for(std::size_t i=1; i<size(); ++i) {
                    markov_network& l=operator[](i);
                    
                    // get a visitor to the **previous** layer's outputs:
                    detail::markov_network_update_visitor<markov_network::svm_type::iterator> lv(l, operator[](i-1).begin_output());
                    
                    // and now update this layer:
                    std::for_each(l.begin(), l.end(), boost::apply_visitor(lv));
                }
            }
        }
        
        //! Set whether this Markov network is able to write into its inputs.
        void writable_inputs(bool w) {
            for(iterator i=begin(); i!=end(); ++i) {
                i->writable_inputs(w);
            }
        }

    private:
        desc_type _desc; //!< Description of the geometries of each successive layer of DMKVs.
        rng_type _rng; //<! Random number generator.
    };

    /*! Update a Deep Markov Network n times with inputs given by f.
     */
    template <typename RandomAccessIterator>
    void update(deep_markov_network& net, std::size_t n, RandomAccessIterator f) {
        net.update(n,f);
    }
    
} // mkv

#endif
