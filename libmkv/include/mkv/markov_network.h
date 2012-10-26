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

#include <boost/variant.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>
#include <vector>

#include <ea/algorithm.h>
#include <ea/rng.h>
#include <mkv/state_vector_machine.h>


namespace mkv {
    
    namespace detail {
        typedef std::vector<std::size_t> index_list_type; //!< Type for a list of indices.
        
        /*! Logic gate.
         */
        struct logic_gate {
            //! Constructor.
            template <typename ForwardIterator>
            logic_gate(const index_list_type& ins, const index_list_type& outs, ForwardIterator f)
            : inputs(ins), outputs(outs), M(1<<inputs.size()) {
                for(std::size_t i=0; i<static_cast<std::size_t>(1<<inputs.size()); ++i, ++f) {
                    M[i] = *f;
                }
            }
            
            index_list_type inputs; //!< Input indices to this node.
            index_list_type outputs; //!< Output indices from this node.
            index_list_type M; //!< Truth table.
        };
        
        /*! Markov gate.
         */
        struct markov_gate {
            typedef boost::numeric::ublas::matrix<double> matrix_type; //!< Probability table type.
            typedef boost::numeric::ublas::matrix_column<matrix_type> column_type; //!< Column type.
            typedef boost::numeric::ublas::matrix_row<matrix_type> row_type; //!< Row type.
            
            //! Constructor.
            template <typename ForwardIterator>
            markov_gate(const index_list_type& ins, const index_list_type& outs, ForwardIterator f)
            : inputs(ins), outputs(outs), M(1<<inputs.size(), 1<<outputs.size()) {
                for(std::size_t i=0; i<M.size1(); ++i) {
                    row_type row(M, i);
                    f = ea::algorithm::normalize(f, f+M.size2(), row.begin(), 1.0);
                }
            }
            
            index_list_type inputs; //!< Input indices to this node.
            index_list_type outputs; //!< Output indices from this node.
            matrix_type M; //!< Probability table.
        };
    } // detail
    
    
    class markov_network {
    public:
        typedef int state_type; //!< Type for states.
        typedef state_vector_machine<state_type> svm_type; //!< State vector machine type.
        typedef boost::variant<detail::logic_gate, detail::markov_gate> variant_gate_type; //!< Variant gate type.
        typedef std::vector<variant_gate_type> gate_list_type; //!< List of gates.
        typedef ea::default_rng_type rng_type; //!< Random number generator type.

        //! Constructs a Markov network with a copy of the given random number generator.
        markov_network(std::size_t nin, std::size_t nout, std::size_t nhid, const rng_type& rng)
        : _nin(nin), _nout(nout), _nhid(nhid), _svm(_nout+_nhid), _rng(rng) {
        }
        
        //! Constructs a Markov network with a copy of the given random number generator.
        markov_network(std::size_t nin, std::size_t nout, std::size_t nhid, unsigned int seed=0)
        : _nin(nin), _nout(nout), _nhid(nhid), _svm(_nout+_nhid), _rng(seed) {
        }
        
        //! Retrieve this network's underlying random number generator.
        rng_type& rng() { return _rng; }
        
        //! Retrieve the number of state variables in this network.
        std::size_t nstates() const { return _nin + _nout + _nhid; }
        
        //! Retrieve the number of input state variables in this network.
        std::size_t ninput_states() const { return _nin; }
        
        //! Retrieve the number of output state variables in this network.
        std::size_t noutput_states() const { return _nout; }
        
        //! Retrieve the number of hidden state variables in this network.
        std::size_t nhidden_states() const { return _nhid; }
        
        //! Retrieve the size of this network, in number of gates.
        std::size_t size() const { return _gates.size(); }
        
        //! Retrieve an iterator to the beginning of this network's gate list.
        gate_list_type::iterator begin() { return _gates.begin(); }
        
        //! Retrieve an iterator to the end of this network's gate list.
        gate_list_type::iterator end() { return _gates.end(); }
        
        //! Retrieve gate i.
        variant_gate_type& operator[](std::size_t i) { return _gates[i]; }
        
        //! Clear the network.
        void clear() { _svm.clear(); }
        
        //! Rotate t and t-1 state vectors.
        void rotate() { _svm.rotate(); }
        
        //! Retrieve an iterator to the beginning of the svm outputs at time t.
        svm_type::state_vector_type::iterator output_begin() { return _svm.t().begin(); }
        
        //! Retrieve an iterator to the end of the svm outputs at time t.
        svm_type::state_vector_type::iterator output_end() { return _svm.t().begin()+_nout; }
        
        /*! Retrieve the value of input i.  Markov networks treat any state variable
         as input, so we need to check to see if the requested input comes from
         the range of inputs, or if it's an internal state variable.
         */
        template <typename RandomAccessIterator>
        state_type input(RandomAccessIterator f, std::size_t i) {
            if(i < _nin) {
                return f[i];
            } else {
                return _svm.state_tminus1(i-_nin);
            }
        }
        
        /*! Update output i with value v.  In this version, we disallow writing
         to the input space.
         */
        void output(std::size_t i, const state_type& v) {
            if(i >= _nin) {
                _svm.state_t(i-_nin) |= v;
            }
        }

        //! Append a gate to this Markov network.
        void append(const variant_gate_type& v) {
            _gates.push_back(v);
        }

    protected:
        std::size_t _nin; //!< Number of input states.
        std::size_t _nout; //!< Number of output states.
        std::size_t _nhid; //!< Number of hidden states.
        svm_type _svm; //!< State vector machine for the hidden & output states.
        
        gate_list_type _gates; //!< List of gates in this markov network.
        rng_type _rng; //<! Random number generator.
    };
    
    
    namespace detail {
        /*! Visitor, used to trigger updates on different gate types.
         */
        template <typename RandomAccessIterator>
        class gate_update_visitor : public boost::static_visitor< > {
        public:
            gate_update_visitor(markov_network& net, RandomAccessIterator f)
            : _net(net), _f(f) {
            }
            
            /*! Retrieve the input to this node from the Markov network's state machine at time t-1.
             The first input is the low-order bit of the lookup table.
             */
            int get_input(const index_list_type& inputs) const {
                int v=0;
                for(std::size_t i=0; i<inputs.size(); ++i) {
                    v |= (_net.input(_f, inputs[i]) & 0x01) << i;
                }
                return v;
            }
            
            /*! Set the output from this node to the Markov network's state machine at time t.
             The first output is the low-order bit of the lookup table.
             */
            void set_output(int v, const index_list_type& outputs) const {
                for(std::size_t i=0; i<outputs.size(); ++i) {
                    _net.output(outputs[i], (v >> i) & 0x01);
                }
            }
            
            //! Update a logic gate.
            void operator()(logic_gate& g) const {
                set_output(g.M[get_input(g.inputs)], g.outputs);
            }
            
            //! Update a Markov gate.
            void operator()(markov_gate& g) const {
                markov_gate::row_type row(g.M, get_input(g.inputs));
                double p = _net.rng().uniform_real(0.0,1.0);
                
                for(int i=0; i<static_cast<int>(g.M.size2()); ++i) {
                    if(p <= row[i]) {
                        set_output(i, g.outputs);
                        return;
                    }
                    p -= row[i];
                }
                
                // if we get here, there was a floating point precision problem.
                // default to the final column:
                set_output(static_cast<int>(g.M.size2()-1), g.outputs);
            }
            
        protected:
            markov_network& _net;
            RandomAccessIterator _f;
        };
    } // detail
    
    
    /*! Update a Markov network n times, with the inputs given by f and writing outputs to o.
     */
    template <typename RandomAccessIterator, typename OutputIterator>
    void update(markov_network& net, std::size_t n, RandomAccessIterator f, OutputIterator o) {
        detail::gate_update_visitor<RandomAccessIterator> visitor(net, f);
        
        for( ; n>0; --n) {
            net.rotate();
            std::for_each(net.begin(), net.end(), boost::apply_visitor(visitor));
        }
        
        std::copy(net.output_begin(), net.output_end(), o);
    }
    
} // mkv

#endif
