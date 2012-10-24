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
#ifndef _MKV_MARKOV_NETWORK_H_
#define _MKV_MARKOV_NETWORK_H_

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>
#include <boost/shared_ptr.hpp>
#include <deque>
#include <vector>

#include <ea/algorithm.h>
#include <ea/rng.h>
#include <ea/meta_data.h>
#include <mkv/state_vector_machine.h>


namespace mkv {
    
    /*! Updates a Markov network with the given input vector, producing output.
     */
    template <typename MKV, typename StateVector>
    void update(MKV& mkv, const StateVector& input, StateVector& output) {
        if(input.size() != mkv.input_size()) {
            throw std::invalid_argument("input size of network not equal to number of inputs provided");
        }
        if(output.size() != mkv.output_size()) {
            throw std::invalid_argument("output size of network not equal to number of outputs provided");
        }
        
        mkv.rotate();
        std::copy(input.begin(), input.end(), mkv.input_begin());
        
        mkv.top_half();
        for(typename MKV::iterator i=mkv.begin(); i!=mkv.end(); ++i) {
            (*i)->update(mkv);
        }
        mkv.bottom_half();
        
        std::copy(mkv.output_begin(), mkv.output_end(), output.begin());
    }

    template <typename MKV, typename T>
    void update_ptr(MKV& mkv, const T* input, T* output) {
        mkv.rotate();
        std::copy(input, input+mkv.input_size(), mkv.input_begin());
        
        mkv.top_half();
        for(typename MKV::iterator i=mkv.begin(); i!=mkv.end(); ++i) {
            (*i)->update(mkv);
        }
        mkv.bottom_half();
        
        std::copy(mkv.output_begin(), mkv.output_end(), output);
    }

    /*! Update a Markov Network n times with inputs [f,l).
     */
    template <typename MKV, typename StateVector>
    void update_n(std::size_t n, MKV& mkv, const StateVector& input, StateVector& output) {
        for(; n>0; --n) {
            update(mkv, input, output);
        }
    }

    template <typename MKV, typename T>
    void update_n_ptr(std::size_t n, MKV& mkv, const T* input, T* output) {
        for(; n>0; --n) {
            update_ptr(mkv, input, output);
        }
    }

    //! Reinforce all nodes in a (Markov) network.
    template <typename Network>
    void reinforce(Network& net, double lr) {
        for(typename Network::iterator i=net.begin(); i!=net.end(); ++i) {
            (*i)->reinforce(lr);
        }
    }
    
    // pre-decs
    namespace detail { struct abstract_markov_node; }

    class mkv_instrument;
    
    
    /*! Markov network.
     
     SVM layout: | INPUTS... | OUTPUTS... | HIDDEN STATES... |
     */
    class markov_network {
    public:
        typedef int state_type; //!< Type for states.
        typedef state_vector_machine<state_type> svm_type; //!< State vector machine type.
        typedef boost::shared_ptr<detail::abstract_markov_node> nodeptr_type; //!< Pointer type for markov nodes.
        typedef std::vector<nodeptr_type> nodelist_type; //!< Type for a list of markov nodes.
        typedef std::vector<mkv_instrument*> instrument_list_type; //!< Type for a list of instruments.
        typedef nodelist_type::iterator iterator; //!< Iterator type.
        typedef nodelist_type::const_iterator const_iterator; //!< Const iterator type.
        typedef ea::default_rng_type rng_type; //!< Random number generator type.
        typedef ea::meta_data md_type; //!< Meta-data type.
        
        //! Constructs a Markov network.
        markov_network(std::size_t nin, std::size_t nout, std::size_t nhid, unsigned int seed=42)
        : _nin(nin), _nout(nout), _nhid(nhid), _svm(_nin+_nout+_nhid), _rng(seed) {
        }
        
        //! Constructs a Markov network with a copy of the given random number generator.
        markov_network(std::size_t nin, std::size_t nout, std::size_t nhid, const rng_type& rng)
        : _nin(nin), _nout(nout), _nhid(nhid), _svm(_nin+_nout+_nhid), _rng(rng) {
        }
        
        //! Reset the random number generator used by this network.
        void reset(unsigned int seed) { _rng.reset(seed); }
        
        //! Retrieve this network's underlying random number generator.
        rng_type& rng() { return _rng; }
        
        //! Accessor for meta-data.
        md_type& md() { return _md; }
        
        //
        // The following methods are for accessing nodes and size information for
        // this Markov network:
        //
        
        //! Retrieve the size of the state vector machine, in number of states.
        std::size_t svm_size() const { return _svm.size(); }
        
        //! Retrieve the input size for this network.
        std::size_t input_size() const { return _nin; }
        
        //! Retrieve the output size for this network.
        std::size_t output_size() const { return _nout; }
        
        //! Retrieve the number of hidden states for this network.
        std::size_t hidden_size() const { return _nhid; }
        
        //! Append a node to this network.
        void append(nodeptr_type node) {
            _nodes.push_back(node);
        }
        
        //! Add an instrument to this markov network.
        void add_instrument(mkv_instrument* instr) {
            _instr.push_back(instr);
        }
        
        //! Retrieve the size of this network, in number of nodes.
        std::size_t size() const { return _nodes.size(); }
        
        //! Retrieve a begin iterator to the nodelist.
        iterator begin() { return _nodes.begin(); }
        
        //! Retrieve an end iterator to the nodelist.
        iterator end() { return _nodes.end(); }
        
        //! Retrieve a begin iterator to the nodelist (const-qualified).
        const_iterator begin() const { return _nodes.begin(); }
        
        //! Retrieve an end iterator to the nodelist (const-qualified).
        const_iterator end() const { return _nodes.end(); }
        
        //! Retrieve a pointer to node i.
        nodeptr_type operator[](std::size_t i) { return _nodes[i]; }
        
        //! Retrieve the backing state vector machine:
        svm_type& svm() { return _svm; }
        
        //
        // The following methods define the interface for a SOC:
        //
        
        //! Clear the network.
        void clear() { _svm.clear(); }
        
        //! Rotate t and t-1 state vectors.
        void rotate() { _svm.rotate(); }
        
        //! Called immediately before network nodes are updated.
        void top_half();
        
        //! Called immediately after network nodes are updated.
        void bottom_half();
        
        //! Return an iterator to the beginning of the svm inputs at time t-1.
        svm_type::state_vector_type::iterator input_begin() { return _svm.tminus1().begin(); }
        
        //! Return an iterator to the beginning of the svm outputs at time t.
        svm_type::state_vector_type::iterator output_begin() { return _svm.t().begin()+_nin; }
        
        //! Return an iterator to the end of the svm outputs at time t.
        svm_type::state_vector_type::iterator output_end() { return _svm.t().begin()+_nin+_nout; }
        
    protected:
        std::size_t _nin; //!< Number of input states.
        std::size_t _nout; //!< Number of output states.
        std::size_t _nhid; //!< Number of hidden states.
        svm_type _svm; //!< SVM backing this markov network.
        nodelist_type _nodes; //!< List of nodes in this markov network.
        instrument_list_type _instr; //!< List of active instruments.
        rng_type _rng; //<! Random number generator.
        md_type _md; //!< Meta-data type.
    };

} // mkv

#endif
