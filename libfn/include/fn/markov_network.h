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
#ifndef _FN_MARKOV_NETWORK_H_
#define _FN_MARKOV_NETWORK_H_

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>
#include <boost/shared_ptr.hpp>
#include <deque>
#include <vector>

#include <ea/algorithm.h>
#include <ea/rng.h>
#include <ea/meta_data.h>
#include <fn/svm.h>
#include <fn/update.h>

// meta-data
LIBEA_MD_DECL(PROB_GATE_ALLOW_0, "fn.markov_network.node.allow_zero", bool);
LIBEA_MD_DECL(NODE_INPUT_LIMIT, "fn.markov_network.node.input.limit", int);
LIBEA_MD_DECL(NODE_INPUT_FLOOR, "fn.markov_network.node.input.floor", int);
LIBEA_MD_DECL(NODE_OUTPUT_LIMIT, "fn.markov_network.node.output.limit", int);
LIBEA_MD_DECL(NODE_OUTPUT_FLOOR, "fn.markov_network.node.output.floor", int);
LIBEA_MD_DECL(NODE_HISTORY_LIMIT, "fn.markov_network.node.history.limit", int);
LIBEA_MD_DECL(NODE_HISTORY_FLOOR, "fn.markov_network.node.history.floor", int);


namespace fn {
    
    // pre-decs
    namespace detail { struct abstract_markov_node; }
    class markov_network;
    
    //! Base class for markov network instruments.
    class mkv_instrument {
    public:
        mkv_instrument() { }
        virtual ~mkv_instrument() { }
        virtual void top_half(markov_network& mkv) { }
        virtual void bottom_half(markov_network& mkv) { }
        virtual void clear() { }
        virtual std::ostream& operator<<(std::ostream& out) { return out; }
    };
    
    /*! Markov network.
     
     SVM layout: | INPUTS... | OUTPUTS... | HIDDEN STATES... |
     */
    class markov_network {
    public:
        typedef int state_type; //!< Type for states.
        typedef svm<state_type> svm_type; //!< State vector machine type.
        typedef boost::shared_ptr<detail::abstract_markov_node> nodeptr_type; //!< Pointer type for markov nodes.
        typedef std::vector<nodeptr_type> nodelist_type; //!< Type for a list of markov nodes.
        typedef ea::default_rng_type rng_type; //!< Random number generator type.
        typedef ea::meta_data md_type; //!< Meta-data type.
        
        //! Constructor.
        markov_network(std::size_t nin, std::size_t nout, std::size_t nhid, unsigned int seed=42)
        : _nin(nin), _nout(nout), _nhid(nhid), _svm(_nin+_nout+_nhid), _rng(seed) {
        }
        
        //! Append a node to this network.
        void append(nodeptr_type node) {
            _nodes.push_back(node);
        }
        
        //! Reset the random number generator used by this network.
        void reset(unsigned int seed) { _rng.reset(seed); }
        
        //! Retrieve this network's underlying random number generator.
        rng_type& rng() { return _rng; }
        
        //! Accessor for meta-data.
        md_type& md() { return _md; }
        
        //! Rotate this network's states.
        void rotate() { _svm.rotate(); }
        
        //! Called immediately before network nodes are updated.
        void top_half() { }
        
        //! Called immediately after network nodes are updated.
        void bottom_half() { }
        
        //! Retrieve the size of the state vector machine, in number of states.
        std::size_t svm_size() const { return _svm.size(); }
        
        //! Retrieve the size of this network, in number of nodes.
        std::size_t size() const { return _nodes.size(); }

        //! Retrieve the input size for this network.
        std::size_t input_size() const { return _nin; }
        
        //! Retrieve the output size for this network.
        std::size_t output_size() const { return _nout; }
        
        //! Retrieve the backing state vector machine.
        svm_type& svm() { return _svm; }
        
        //! Retrieve an iterator to the beginning of the inputs in the tminus1 state vector.
        svm_type::state_vector_type::iterator tminus1_inputs() { return _svm.tminus1().begin(); }
        
        //! Retrieve an iterator to the beginning of the outputs in the t state vector.
        svm_type::state_vector_type::iterator t_outputs() { return _svm.t().begin()+_nin; }
        
        //! Retrieve a begin iterator to the nodelist.
        nodelist_type::iterator begin() { return _nodes.begin(); }
        
        //! Retrieve an end iterator to the nodelist.
        nodelist_type::iterator end() { return _nodes.end(); }
        
        //! Retrieve a begin iterator to the nodelist (const-qualified).
        nodelist_type::const_iterator begin() const { return _nodes.begin(); }
        
        //! Retrieve an end iterator to the nodelist (const-qualified).
        nodelist_type::const_iterator end() const { return _nodes.end(); }
        
    protected:
        std::size_t _nin; //!< Number of input states.
        std::size_t _nout; //!< Number of output states.
        std::size_t _nhid; //!< Number of hidden states.
        svm_type _svm; //!< SVM backing this markov network.
        nodelist_type _nodes; //!< List of nodes in this markov network.
        rng_type _rng; //<! Random number generator.
        md_type _md; //!< Meta-data type.
    };
    
    
    namespace detail {
        typedef std::vector<std::size_t> index_list_type; //!< Type for a list of indices.
        typedef boost::numeric::ublas::matrix<int> matrix_type;
        typedef boost::numeric::ublas::matrix_column<matrix_type> column_type;
        typedef boost::numeric::ublas::matrix_row<matrix_type> row_type;
        typedef std::vector<double> weight_vector_type; //!< Type for feedback weights vector.

        //! Abstract base class for nodes in a Markov network.
        struct abstract_markov_node {
            abstract_markov_node(index_list_type inputs, index_list_type outputs)
            : _in(inputs), _out(outputs) {
            }
            
            //! Destructor.
            virtual ~abstract_markov_node() { }
            
            //! Update the Markov network from this node.
            virtual void update(markov_network& mkv) = 0;
            
            //! Retrieve the input to this node from the Markov network's state machine at time t-1.
            int get_input(markov_network& mkv) {
                int x=0;
                for(std::size_t i=0; i<_in.size(); ++i) {
                    x |= (mkv.svm().state_tminus1(_in[i]) & 0x01) << (_in.size()-1-i);
                }
                return x;
            }
            
            //! Set the output from this node to the Markov network's state machine at time t.
            void set_output(int x, markov_network& mkv) {
                for(std::size_t i=0; i<_out.size(); ++i) {
                    mkv.svm().state_t(_out[i]) |= (x >> (_out.size()-1-i)) & 0x01;
                }
            }
            
            index_list_type _in; //!< Input state indices to this node.
            index_list_type _out; //!< Output state indices from this node.
        };
        
        
        /*! Probabilistic Markov network node.
         */
        struct probabilistic_mkv_node : public abstract_markov_node {
            //! Constructor.
            template <typename ForwardIterator>
            probabilistic_mkv_node(index_list_type inputs, index_list_type outputs, ForwardIterator f, bool allow_zero) 
            : abstract_markov_node(inputs,outputs), _table(1<<inputs.size(), (1<<outputs.size())+1) {
                for(std::size_t i=0; i<_table.size1(); ++i) {
                    int sum=0;
                    for(std::size_t j=0; j<(_table.size2()-1); ++j, ++f) {
                        _table(i,j) = *f;
                        if((!allow_zero) && (_table(i,j)==0)) {
                            ++_table(i,j);
                        }
                        sum += _table(i,j);
                    }
                    _table(i,_table.size2()-1) = sum;
                }
            }
            
            //! Update the Markov network from this probabilistic node.
            void update(markov_network& mkv) {
                row_type row(_table, get_input(mkv));
                int rnum = mkv.rng()(*row.rbegin()+1);
                int col=0;
                while(rnum > row[col]) {
                    rnum -= row[col];
                    ++col;
                }
                set_output(col, mkv);
            }
            
            matrix_type _table; //!< Probability table.
        };
        
        /*! Synaptically-learning probabilistic Markov network node.
         */
        struct synprob_mkv_node : public probabilistic_mkv_node {
            typedef std::deque<std::pair<std::size_t, std::size_t> > history_type;//!< Type for tracking history of decisions.
            
            //! Constructor.
            template <typename ForwardIterator>
            synprob_mkv_node(std::size_t hn,
                             std::size_t posf, weight_vector_type poswv, 
                             std::size_t negf, weight_vector_type negwv,
                             index_list_type inputs, index_list_type outputs, ForwardIterator ft, bool allow_zero) 
            : probabilistic_mkv_node(inputs, outputs, ft, allow_zero), _hn(hn), _posf(posf), _poswv(poswv), _negf(negf), _negwv(negwv) {
            }
            
            //! Learn.
            void learn(markov_network& mkv) {
                // learn:
                if(mkv.svm().state_tminus1(_posf)) {
                    // positive feedback
                    for(std::size_t i=0; (i<_poswv.size()) && (i<_history.size()); ++i) {
                        int& cell = _table(_history[i].first, _history[i].second);
                        int last=cell;
                        cell = static_cast<int>(static_cast<double>(last) / _poswv[i]);
                        _table(_history[i].first, _table.size2()-1) += (cell-last);
                    }
                }
                if(mkv.svm().state_tminus1(_negf)) {
                    // negative feedback
                    for(std::size_t i=0; (i<_negwv.size()) && (i<_history.size()); ++i) {
                        int& cell = _table(_history[i].first, _history[i].second);
                        int last=cell;
                        cell = static_cast<int>(static_cast<double>(last) * _negwv[i]);
                        _table(_history[i].first, _table.size2()-1) -= (last-cell);
                    }
                }
            }
            
            //! Update the Markov network from this probabilistic node.
            void update(markov_network& mkv) {
                // perform learning:
                learn(mkv);
                
                // set the output:
                std::size_t row = get_input(mkv);
                row_type r(_table, row);
                int rnum = mkv.rng()(*r.rbegin()+1);
                int col=0;
                while(rnum > r[col]) {
                    rnum -= r[col];
                    ++col;
                }                
                set_output(col, mkv);
                
                // track history:
                _history.push_back(std::make_pair(row,col));
                while(_history.size() > _hn) {
                    _history.pop_front();
                }
            }
            
            std::size_t _hn; //!< Size of history to keep.
            std::size_t _posf; //!< Index of positive feedback state.
            weight_vector_type _poswv; //!< Positive feedback weight vector.
            std::size_t _negf; //!< Index of negative feedback state.
            weight_vector_type _negwv; //!< Negative feedback weight vector.
            history_type _history; //!< History of decisions made by this node.
        };
        
        
        /*! Deterministic Markov network node.
         */
        struct deterministic_mkv_node : public abstract_markov_node {
            //! Constructor.
            template <typename ForwardIterator>
            deterministic_mkv_node(index_list_type inputs, index_list_type outputs, ForwardIterator f) 
            : abstract_markov_node(inputs,outputs), _table(1<<_in.size(), 1) {
                for(std::size_t i=0; i<(1<<_in.size()); ++i, ++f) {
                    _table(i,0) = *f;
                }
            }
            
            //! Update the Markov network from this deterministic node.
            void update(markov_network& mkv) {
                set_output(_table(get_input(mkv),0), mkv);
            }
            
            matrix_type _table; //!< Deterministic table.
        };
        
        enum node_type { PROB=42, DET=43, SYNPROB=44 };
    } // detail
    
        
    /*! Build a Markov network from the genome [f,l).
     */
    template <typename Network, typename ForwardIterator>
    void build_markov_network(Network& net, ForwardIterator f, ForwardIterator l) {
        using namespace detail;
        using namespace ea;
        using namespace ea::algorithm;
        
        if(f == l) { return; }
        
        ForwardIterator last=f;
        ++f;
        
        for( ; f!=l; ++f, ++last) {
            int start_codon = *f + *last;
            if(start_codon == 255) {
                switch(*last) {
                    case PROB: { // build a probabilistic node
                        ForwardIterator h=f;
                        ++h;
                        int nin=modnorm(*h++, get<NODE_INPUT_FLOOR>(net), get<NODE_INPUT_LIMIT>(net));
                        int nout=modnorm(*h++, get<NODE_OUTPUT_FLOOR>(net), get<NODE_OUTPUT_LIMIT>(net));
                        index_list_type inputs(h, h+nin);
                        std::transform(inputs.begin(), inputs.end(), inputs.begin(), std::bind2nd(std::modulus<int>(), net.svm_size()));
                        h+=nin;
                        index_list_type outputs(h, h+nout);
                        std::transform(outputs.begin(), outputs.end(), outputs.begin(), std::bind2nd(std::modulus<int>(), net.svm_size()));
                        h+=nout;
                        
                        markov_network::nodeptr_type p(new probabilistic_mkv_node(inputs, outputs, h, get<PROB_GATE_ALLOW_0>(net)));
                        net.append(p);
                        break;
                    }
                    case DET: { // build a deterministic node
                        ForwardIterator h=f;
                        ++h;
                        int nin=modnorm(*h++, get<NODE_INPUT_FLOOR>(net), get<NODE_INPUT_LIMIT>(net));
                        int nout=modnorm(*h++, get<NODE_OUTPUT_FLOOR>(net), get<NODE_OUTPUT_LIMIT>(net));
                        index_list_type inputs(h, h+nin);
                        std::transform(inputs.begin(), inputs.end(), inputs.begin(), std::bind2nd(std::modulus<int>(), net.svm_size()));
                        h+=nin;
                        index_list_type outputs(h, h+nout);
                        std::transform(outputs.begin(), outputs.end(), outputs.begin(), std::bind2nd(std::modulus<int>(), net.svm_size()));
                        h+=nout;
                        
                        markov_network::nodeptr_type p(new deterministic_mkv_node(inputs, outputs, h));
                        net.append(p);
                        break;
                    }
                    case SYNPROB: { // build a synaptically learning probabilistic node
                        ForwardIterator h=f;
                        ++h;
                        int nin=modnorm(*h++, get<NODE_INPUT_FLOOR>(net), get<NODE_INPUT_LIMIT>(net));
                        int nout=modnorm(*h++, get<NODE_OUTPUT_FLOOR>(net), get<NODE_OUTPUT_LIMIT>(net));
                        int nhistory=modnorm(*h++, get<NODE_HISTORY_FLOOR>(net), get<NODE_HISTORY_LIMIT>(net));
                        int posf=*h++ % net.svm_size();
                        int negf=*h++ % net.svm_size();
                        index_list_type inputs(h, h+nin);
                        std::transform(inputs.begin(), inputs.end(), inputs.begin(), std::bind2nd(std::modulus<int>(), net.svm_size()));
                        h+=nin;
                        index_list_type outputs(h, h+nout);
                        std::transform(outputs.begin(), outputs.end(), outputs.begin(), std::bind2nd(std::modulus<int>(), net.svm_size()));
                        h+=nout;                        
                        weight_vector_type poswv(h, h+nhistory);
                        std::transform(poswv.begin(), poswv.end(), poswv.begin(), std::bind2nd(std::divides<double>(), 32767.0));
                        h+=nhistory;
                        weight_vector_type negwv(h, h+nhistory);
                        std::transform(negwv.begin(), negwv.end(), negwv.begin(), std::bind2nd(std::divides<double>(), 32767.0));
                        h+=nhistory;
                        
                        markov_network::nodeptr_type p(new synprob_mkv_node(nhistory,
                                                                            posf, poswv,
                                                                            negf, negwv,
                                                                            inputs, outputs, h, get<PROB_GATE_ALLOW_0>(net)));
                        net.append(p);
                        break;
                    }
                    default: { 
                        break;
                    }
                }
            }
        }
    }
    
} // fn

#endif
