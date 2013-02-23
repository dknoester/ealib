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
#include <boost/tuple/tuple.hpp>
#include <vector>
#include <deque>

#include <ea/algorithm.h>
#include <ea/rng.h>
#include <mkv/state_vector_machine.h>


namespace mkv {
    
    class markov_network;
    
    namespace detail {
        
        typedef std::vector<std::size_t> index_list_type; //!< Type for a list of indices.
        typedef std::vector<double> weight_vector_type; //!< Type for feedback weights vector.
        
        /*! Abstract gate.
         */
        struct abstract_gate {
            //! Constructor.
            abstract_gate(const index_list_type& ins, const index_list_type& outs)
            : inputs(ins), outputs(outs) {
            }
            
            //! Destructor.
            virtual ~abstract_gate() {
            }
            
            index_list_type inputs; //!< Input indices to this node.
            index_list_type outputs; //!< Output indices from this node.
        };
        
        
        /*! Logic gate.
         */
        struct logic_gate : abstract_gate {
            //! Constructor.
            template <typename ForwardIterator>
            logic_gate(const index_list_type& ins, const index_list_type& outs, ForwardIterator f)
            : abstract_gate(ins, outs), M(1<<inputs.size()) {
                for(std::size_t i=0; i<static_cast<std::size_t>(1<<inputs.size()); ++i, ++f) {
                    M[i] = *f;
                }
            }
            
            //! Destructor.
            virtual ~logic_gate() {
            }
            
            index_list_type M; //!< Truth table.
        };
        
        
        /*! Markov gate.
         */
        struct markov_gate : abstract_gate {
            typedef boost::numeric::ublas::matrix<double> matrix_type; //!< Probability table type.
            typedef boost::numeric::ublas::matrix_column<matrix_type> column_type; //!< Column type.
            typedef boost::numeric::ublas::matrix_row<matrix_type> row_type; //!< Row type.
            
            //! Constructor.
            template <typename ForwardIterator>
            markov_gate(const index_list_type& ins, const index_list_type& outs, ForwardIterator f)
            : abstract_gate(ins, outs), M(1<<inputs.size(), 1<<outputs.size()) {
                for(std::size_t i=0; i<M.size1(); ++i) {
                    row_type row(M, i);
                    f = ea::algorithm::normalize(f, f+M.size2(), row.begin(), 1.0);
                }
            }
            
            //! Destructor.
            virtual ~markov_gate() {
            }
            
            matrix_type M; //!< Probability table.
        };
        
        
        /*! Adaptive Markov gate.
         */
        struct adaptive_gate : abstract_gate {
            typedef boost::numeric::ublas::matrix<double> matrix_type; //!< Probability table type.
            typedef boost::numeric::ublas::matrix_column<matrix_type> column_type; //!< Column type.
            typedef boost::numeric::ublas::matrix_row<matrix_type> row_type; //!< Row type.
            typedef std::deque<std::pair<std::size_t, std::size_t> > history_type;//!< Type for tracking history of decisions.
            
            //! Constructor.
            template <typename ForwardIterator>
            adaptive_gate(std::size_t hn,
                          std::size_t posf, weight_vector_type poswv,
                          std::size_t negf, weight_vector_type negwv,
                          index_list_type ins, index_list_type outs, ForwardIterator f)
            : abstract_gate(ins, outs), h(hn), p(posf), P(poswv), n(negf), N(negwv), M(1<<inputs.size(), 1<<outputs.size()) {
                for(std::size_t i=0; i<M.size1(); ++i) {
                    row_type row(M, i);
                    f = ea::algorithm::normalize(f, f+M.size2(), row.begin(), 1.0);
                }
            }
            
            //! Destructor.
            virtual ~adaptive_gate() {
            }
            
            //! Scale the probability of output (i,j) by s.
            void scale(std::size_t i, std::size_t j, double s) {
                M(i,j) *= 1.0 + s;
                row_type row(M, i);
                ea::algorithm::normalize(row.begin(), row.end(), row.begin(), 1.0);
            }
            
            //! Reinforce the recent behavior of this gate.
            void reinforce() {
                for(std::size_t i=0; (i<P.size()) && (i<H.size()); ++i) {
                    scale(H[i].first, H[i].second, P[i]);
                }
            }
            
            //! Inhibit the recent behavior of this gate.
            void inhibit() {
                for(std::size_t i=0; (i<N.size()) && (i<H.size()); ++i) {
                    scale(H[i].first, H[i].second, N[i]);
                }
            }
            
            std::size_t h; //!< Size of history to keep.
            history_type H; //!< History of decisions made by this node.
            std::size_t p; //!< Index of positive feedback state.
            weight_vector_type P; //!< Positive feedback weight vector.
            std::size_t n; //!< Index of negative feedback state.
            weight_vector_type N; //!< Negative feedback weight vector.
            matrix_type M; //!< Probability table.
        };

    } // detail
    
    typedef boost::variant<detail::logic_gate, detail::markov_gate, detail::adaptive_gate> variant_gate_type; //!< Variant gate type.
    
    
    /*! Markov Network that contains gates, a state vector machine, and an underlying geometry of
     inputs, outputs, and hidden states.
     */
    class markov_network : public std::vector<variant_gate_type> {
    public:
        typedef boost::tuple<std::size_t, std::size_t, std::size_t> desc_type; //!< Descriptor for Markov Networks.
        enum { IN, OUT, HID }; //!< Indices into the desc_type for number of inputs, outputs, and hidden states.
        typedef int state_type; //!< Type for states.
        typedef state_vector_machine<state_type> svm_type; //!< State vector machine type.
        typedef std::vector<variant_gate_type> base_type; //!< List of gates.
        typedef ea::default_rng_type rng_type; //!< Random number generator type.
        
        //! Constructs a Markov network with a copy of the given random number generator.
        markov_network(std::size_t nin, std::size_t nout, std::size_t nhid, const rng_type& rng)
        : _desc(nin, nout, nhid), _svm(nout+nhid), _rng(rng), _threshold(0) {
        }
        
        //! Constructs a Markov network with the given seed.
        markov_network(std::size_t nin, std::size_t nout, std::size_t nhid, unsigned int seed=0)
        : _desc(nin, nout, nhid), _svm(nout+nhid), _rng(seed), _threshold(0) {
        }
        
        //! Constructs a Markov network with the given seed.
        markov_network(const desc_type& desc, unsigned int seed=0)
        : _desc(desc), _svm(desc.get<OUT>()+desc.get<HID>()), _rng(seed), _threshold(0) {
        }
        
        //! Constructs a Markov network with a copy of the given random number generator.
        markov_network(const desc_type& desc, const rng_type& rng)
        : _desc(desc), _svm(desc.get<OUT>()+desc.get<HID>()), _rng(rng), _threshold(0) {
        }
        
        //! Retrieve this network's underlying random number generator.
        rng_type& rng() { return _rng; }
        
        //! Retrieve the number of input state variables in this network.
        std::size_t ninput_states() const { return _desc.get<IN>(); }
        
        //! Retrieve the number of output state variables in this network.
        std::size_t noutput_states() const { return _desc.get<OUT>(); }
        
        //! Retrieve the number of hidden state variables in this network.
        std::size_t nhidden_states() const { return _desc.get<HID>(); }
        
        //! Retrieve the number of state variables in this network.
        std::size_t nstates() const {
            return ninput_states() + noutput_states() + nhidden_states();
        }
        
        //! Retrieve the size of this network, in number of gates.
        std::size_t ngates() const { return size(); }
        
        //! Clear the network.
        void clear() { _svm.clear(); }
        
        //! Rotate t and t-1 state vectors.
        void rotate() { _svm.rotate(); }
        
        //! Retrieve an iterator to the beginning of the svm outputs at time t.
        svm_type::iterator begin_output() { return _svm.t().begin(); }
        
        //! Retrieve an iterator to the end of the svm outputs at time t.
        svm_type::iterator end_output() { return _svm.t().begin()+_desc.get<OUT>(); }
        
        //! Retrieve the threshold for inputs to be considered a "1".
        int& threshold() { return _threshold; }
        
        /*! Retrieve the value of input i.  Markov networks treat any state variable
         as input, so we need to check to see if the requested input comes from
         the range of inputs, or if it's an internal state variable.
         */
        template <typename RandomAccessIterator>
        state_type input(RandomAccessIterator f, std::size_t i) {
            if(i < _desc.get<IN>()) {
                return (f[i] > _threshold);
            } else {
                return _svm.state_tminus1(i-_desc.get<IN>());
            }
        }
        
        /*! Update output i with value v.  In this version, we disallow writing
         to the input space.
         */
        void output(std::size_t i, const state_type& v) {
            if(i >= _desc.get<IN>()) {
                _svm.state_t(i-_desc.get<IN>()) |= v;
            }
        }
        
    protected:
        desc_type _desc; //!< Geometry descriptor for this Markov Network.
        svm_type _svm; //!< State vector machine for the hidden & output states.
        rng_type _rng; //<! Random number generator.
        state_type _threshold; //!< Threshold value above which an input is considered a "1".
    };
    
    
    namespace detail {

        /*! Visitor, used to trigger updates on different gate types.
         */
        template <typename RandomAccessIterator>
        class markov_network_update_visitor : public boost::static_visitor< > {
        public:
            markov_network_update_visitor(markov_network& net, RandomAccessIterator f)
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
                // output:
                std::size_t i = get_input(g.inputs);
                markov_gate::row_type row(g.M, i);
                double p = _net.rng().uniform_real(0.0,1.0);
                
                for(int j=0; j<static_cast<int>(g.M.size2()); ++j) {
                    if(p <= row[j]) {
                        set_output(j, g.outputs);
                        return;
                    }
                    p -= row[j];
                }
                
                // if we get here, there was a floating point precision problem.
                // default to the final column:
                set_output(static_cast<int>(g.M.size2()-1), g.outputs);
            }
            
            //! Update an adaptive Markov gate.
            void operator()(adaptive_gate& g) const {
                // learn first: if one of the feedback bits is on, it means that
                // the previous behavior of this gate should be reinforced.
                // if we wait to learn until after updating, we'll be reinforcing
                // for the *next* output as well.
                while(g.H.size() > g.h) { // prune history
                    g.H.pop_front();
                }
                if(_net.input(_f, g.p)) { // reinforce
                    g.reinforce();
                }
                if(_net.input(_f, g.n)) { // inhibit
                    g.inhibit();
                }
                
                // now handle the next output:
                std::size_t i = get_input(g.inputs);
                adaptive_gate::row_type row(g.M, i);
                double p = _net.rng().uniform_real(0.0,1.0);
                
                for(int j=0; j<static_cast<int>(g.M.size2()); ++j) {
                    if(p <= row[j]) {
                        set_output(j, g.outputs);
                        g.H.push_back(std::make_pair(i,j));
                        return;
                    }
                    p -= row[j];
                }
                
                // if we get here, there was a floating point precision problem.
                // default to the final column:
                set_output(static_cast<int>(g.M.size2()-1), g.outputs);
                g.H.push_back(std::make_pair(i,static_cast<int>(g.M.size2()-1)));
            }
            
        protected:
            markov_network& _net;
            RandomAccessIterator _f;
        };

    } // detail
    
    /*! Update a Markov Network n times with inputs given by f.
     */
    template <typename RandomAccessIterator>
    void update(markov_network& net, std::size_t n, RandomAccessIterator f) {
        detail::markov_network_update_visitor<RandomAccessIterator> visitor(net, f);
        for( ; n>0; --n) {
            net.rotate();
            std::for_each(net.begin(), net.end(), boost::apply_visitor(visitor));
        }
    }
    
} // mkv

#endif
