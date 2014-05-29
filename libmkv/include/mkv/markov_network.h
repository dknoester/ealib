/* mkv/markov_network.h
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
#ifndef _EA_MKV_MARKOV_NETWORK_H_
#define _EA_MKV_MARKOV_NETWORK_H_

#include <boost/algorithm/string/predicate.hpp>
#include <boost/shared_ptr.hpp>
#include <functional>
#include <ea/rng.h>
#include <ea/functional.h>
#include <mkv/gates.h>

namespace mkv {
    
    /*! Markov Network.
     
     The default update function is a binary or.
     
     \param StateType state variable type
     \param UpdateFunction updates state variables
     \param RandomNumberGenerator
     */
    template <typename StateType=int
    , typename UpdateFunction=ealib::binary_or<StateType>
    , typename InputFunction=ealib::non_zero<StateType>
    , typename RandomNumberGenerator=ealib::default_rng_type
    > class markov_network {
    public:
        typedef StateType state_type; //!< State variable type.
        typedef boost::numeric::ublas::vector<state_type> state_vector_type; //!< State vector type.
        typedef state_type* iterator; //!< Type for iterators over state variables.
        typedef mkv::abstract_gate<RandomNumberGenerator> abstract_gate_type; //!< Type of abstract gate held by this network.
        typedef boost::shared_ptr<abstract_gate_type> abstract_gate_ptr; //!< Abstract gate pointer type.
        typedef std::vector<abstract_gate_ptr> gate_vector_type; //!< Type for a vector of abstract gates.
        typedef UpdateFunction update_function_type; //!< Binary function that updates state variables.
        typedef InputFunction input_function_type; //!< Unary function that calculates the value of an input.
        typedef RandomNumberGenerator rng_type; //!< Random number generator type.
        
        //! Constructor.
        markov_network(std::size_t nin, std::size_t nout, std::size_t nhid, unsigned int seed=0) : _rng(seed) {
            resize(nin, nout, nhid);
        }
        
        //! Constructor.
        markov_network(std::size_t nin, std::size_t nout, std::size_t nhid, const rng_type& rng) : _rng(rng) {
            resize(nin, nout, nhid);
        }
                
        //! Copy constructor.
        markov_network(const markov_network& that) {
            resize(that._nin, that._nout, that._nhid);
            _rng = that._rng;
            for(typename gate_vector_type::const_iterator i=that._gates.begin(); i!=that._gates.end(); ++i) {
                abstract_gate_ptr p((*i)->clone());
                _gates.push_back(p);
            }
        }
        
        //! Assignment operator.
        markov_network& operator=(const markov_network& that) {
            if(this != &that) {
                resize(that._nin, that._nout, that._nhid);
                _rng = that._rng;
                for(typename gate_vector_type::const_iterator i=that._gates.begin(); i!=that._gates.end(); ++i) {
                    abstract_gate_ptr p((*i)->clone());
                    _gates.push_back(p);
                }
            }
        }
        
        //! Resize this network.
        void resize(std::size_t nin, std::size_t nout, std::size_t nhid) {
            _nin = nin;
            _nout = nout;
            _nhid = nhid;
            _T.resize(_nin+_nout+_nhid);
            _Tplus1.resize(_nin+_nout+_nhid);
            clear();
        }
        
        //! Clears this network (resets all state variables).
        void clear() {
            std::fill(_T.begin(), _T.end(), state_type());
            std::fill(_Tplus1.begin(), _Tplus1.end(), state_type());
            for(typename gate_vector_type::iterator i=_gates.begin(); i!=_gates.end(); ++i) {
                (*i)->clear();
            }
        }
        
        //! Disables adaptation of gate logic.
        void disable_adaptation() {
            for(typename gate_vector_type::iterator i=_gates.begin(); i!=_gates.end(); ++i) {
                (*i)->disable_adaptation();
            }
        }
        
        //! Reset this network's rng.
        void reset(unsigned int seed) {
            _rng.reset(seed);
        }
        
        //! Retrieve the size of this network, in number of gates.
        std::size_t ngates() const { return _gates.size(); }
        
        //! Retrieve a vector of this network's gates.
        gate_vector_type& gates() { return _gates; }
        
        //! Retrieve gate i.
        abstract_gate_type& operator[](std::size_t i) { return *_gates[i]; }
        
        //! Retrieve gate i (const-qualified).
        const abstract_gate_type& operator[](std::size_t i) const { return *_gates[i]; }
        
        //! Retrieve the number of state variables in this network.
        std::size_t nstates() const { return _T.size(); }
        
        //! Retrieve the number of inputs to this network.
        std::size_t ninputs() const { return _nin; }
        
        //! Retrieve the number of outputs from this network.
        std::size_t noutputs() const { return _nout; }
        
        //! Retrieve the number of hiddenn state variables in this network.
        std::size_t nhidden() const { return _nhid; }
        
        //! Retrieve state variable i.
        int& operator()(std::size_t i) { return _T(i); }
        
        //! Retrieve state variable i (const-qualified).
        const int& operator()(std::size_t i) const { return _T(i); }
        
        //! Retrieve input state variable i.
        int& input(std::size_t i) { return _T[i]; }
        
        //! Retrieve output state variable i (const-qualified).
        const int& input(std::size_t i) const { return _T[i]; }
        
        //! Retrieve output state variable i.
        int& output(std::size_t i) { return _T[_nin+i]; }
        
        //! Retrieve output state variable i (const-qualified).
        const int& output(std::size_t i) const { return _T[_nin+i]; }
        
        //! Retrieve hidden state variable i.
        int& hidden(std::size_t i) { return _T[_nin+_nout+i]; }
        
        //! Retrieve hidden state variable i (const-qualified).
        const int& hidden(std::size_t i) const { return _T[_nin+_nout+i]; }
        
        //! Retrieve an iterator to the beginning of the inputs.
        iterator begin_input() { return &_T[0]; }
        
        //! Retrieve an iterator to the end of the inputs.
        iterator end_input() { return &_T[0] + _nin; }
        
        //! Retrieve an iterator to the beginning of the outputs.
        iterator begin_output() { return &_T[0] + _nin; }
        
        //! Retrieve an iterator to the end of the outputs.
        iterator end_output() { return &_T[0] + _nin + _nout; }
        
        //! Retrieve an iterator to the beginning of the hidden states.
        iterator begin_hidden() { return &_T[0] + _nin + _nout; }
        
        //! Retrieve an iterator to the end of the hidden states.
        iterator end_hidden() { return &_T[0] + _nin + _nout + _nhid; }
        
        /*! Zero-copy update.
         
         This function calculates the input to each gate from its state variables,
         runs the gate on that input, and then sends that output to other state
         variables.
         
         \param f is any type that supports operator[] (e.g., RA iterator or sequence).
         */
        template <typename RandomAccess>
        void update(RandomAccess f, std::size_t n=1) {
            for( ; n>0; --n) {
                for(typename gate_vector_type::iterator i=_gates.begin(); i!=_gates.end(); ++i) {
                    // calculate the input to this gate
                    mkv::index_vector_type& inputs=(*i)->inputs;
                    state_type x=0;
                    for(std::size_t j=0; j<inputs.size(); ++j) {
                        std::size_t k=inputs[j];
                        if(k<_nin) {
                            x = _uf(x, (_if(f[k]) << j));
                        } else {
                            x = _uf(x, (_if(_T(k)) << j));
                        }
                    }
                    
                    // calculate the output:
                    state_type y = (**i)(x,_rng);
                    
                    // set the output from this gate:
                    mkv::index_vector_type& outputs=(*i)->outputs;
                    for(std::size_t j=0; j<outputs.size(); ++j) {
                        state_type& t = _Tplus1(outputs[j]);
                        t = _uf(t, ((y>>j) & 0x01));
                    }
                }
            }
            std::swap(_T,_Tplus1);
            std::fill(_Tplus1.begin(), _Tplus1.end(), state_type()); // have to clear tplus1 (OR logic on output)
        }
        
        //! Update this Markov network n times, assuming all inputs have been set.
        void update(std::size_t n=1) {
            update(_T.begin(), n);
        }
        
    protected:
        update_function_type _uf; //<! Update functor.
        input_function_type _if; //<! Input functor.
        rng_type _rng; //<! Random number generator.
        std::size_t _nin, _nout, _nhid; //!< Number of inputs, outputs, and hidden state variables.
        gate_vector_type _gates; //!< Vector of gates.
        state_vector_type _T; //!< State vector for time t.
        state_vector_type _Tplus1; //!< State vector for time t+1.
    };
    
} // mkv

#endif
