/* mkv/gates.h
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
#ifndef _EA_MKV_GATES_H_
#define _EA_MKV_GATES_H_

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>
#include <vector>
#include <deque>

#include <ea/algorithm.h>
#include <mkv/graph.h>

namespace mkv {
    
    namespace bnu = boost::numeric::ublas;
    
    typedef bnu::vector<std::size_t> index_vector_type; //!< Index vector (logical vector).
    typedef bnu::vector<double> weight_vector_type; //!< Type for feedback weights vector.
    
    typedef bnu::matrix<double> matrix_type; //!< Probability table type.
    typedef bnu::matrix_column<matrix_type> column_type; //!< Column type.
    typedef bnu::matrix_row<matrix_type> row_type; //!< Row type.
    
    
    
    
    
    
    //
    //
    //
    //        //! Parse a Markov gate.
    //        void operator()(markov_gate& g) const {
    //            G[v].gt = vertex_properties::MARKOV;
    //            add_edges(g.inputs, g.outputs);
    //        }
    //
    //        //! Parse an Adaptive Markov gate.
    //        void operator()(adaptive_gate& g) const {
    //            G[v].gt = vertex_properties::ADAPTIVE;
    //            add_edges(g.inputs, g.outputs);
    //            boost::add_edge(boost::vertex(g.p,G), v, edge_properties(edge_properties::REINFORCE), G);
    //            boost::add_edge(boost::vertex(g.n,G), v, edge_properties(edge_properties::INHIBIT), G);
    //        }
    
    
    
    
    /*! Abstract gate.
     */
    template <typename RandomNumberGenerator>
    struct abstract_gate {
        //! Default constructor.
        abstract_gate() {
        }
        
        //! Destructor.
        virtual ~abstract_gate() {
        }
        
        //! Clears any internal state held in this gate.
        virtual void clear() {
        }
        
        //! Disables adaptation of gate logic.
        virtual void disable_adaptation() {
        }
        
        //! Add edges from this gate to a markov graph.
        virtual void add_edges(const index_vector_type& inputs, const index_vector_type& outputs,
                               markov_graph::vertex_descriptor v, markov_graph& G) const {
            for(std::size_t i=0; i<inputs.size(); ++i) {
                boost::add_edge(boost::vertex(inputs[i],G), v, G);
            }
            for(std::size_t i=0; i<outputs.size(); ++i) {
                boost::add_edge(v, boost::vertex(outputs[i],G), G);
            }
        }
        
        //! Write this gate to a Markov graph.
        virtual void as_graph(markov_graph::vertex_descriptor v, markov_graph& G) {
            add_edges(inputs, outputs, v, G);
        }
        
        //! Returns a pointer to a newly-allocated clone of this gate.
        virtual abstract_gate* clone() = 0;
        
        //! Returns the output of this gate based on input x.
        virtual int operator()(int x, RandomNumberGenerator& rng) = 0;
        
        index_vector_type inputs; //!< Input indices to this node.
        index_vector_type outputs; //!< Output indices from this node.
    };
    
    
    /*! Logic gate.
     */
    template <typename RandomNumberGenerator>
    struct logic_gate : abstract_gate<RandomNumberGenerator> {
        typedef abstract_gate<RandomNumberGenerator> parent_type;
        
        //! Constructor.
        logic_gate() {
        }
        
        //! Destructor.
        virtual ~logic_gate() {
        }
        
        //! Write this gate to a Markov graph.
        virtual void as_graph(markov_graph::vertex_descriptor v, markov_graph& G) {
            parent_type::as_graph(v, G);
            G[v].gt = vertex_properties::LOGIC;
        }
        
        //! Returns a pointer to a newly-allocated clone of this gate.
        virtual parent_type* clone() {
            logic_gate* p = new logic_gate(*this);
            return p;
        }
        
        //! Return the output of this gate based on input x.
        virtual int operator()(int x, RandomNumberGenerator& rng) {
            return M[x];
        }
        
        index_vector_type M; //!< Truth table.
    };
    
    
    /*! Probabilistic (Markov) gate.
     */
    template <typename RandomNumberGenerator>
    struct probabilistic_gate : abstract_gate<RandomNumberGenerator> {
        typedef abstract_gate<RandomNumberGenerator> parent_type;
        
        //! Constructor.
        probabilistic_gate() {
        }
        
        //! Destructor.
        virtual ~probabilistic_gate() {
        }
        
        //! Write this gate to a Markov graph.
        virtual void as_graph(markov_graph::vertex_descriptor v, markov_graph& G) {
            parent_type::as_graph(v, G);
            G[v].gt = vertex_properties::MARKOV;
        }
        
        //! Returns a pointer to a newly-allocated clone of this gate.
        virtual parent_type* clone() {
            probabilistic_gate* p = new probabilistic_gate(*this);
            return p;
        }
        
        //! Update t+1 with input from t.
        virtual int operator()(int x, RandomNumberGenerator& rng) {
            row_type row(M, x);
            double p = rng.p();
            
            for(int j=0; j<static_cast<int>(M.size2()); ++j) {
                if(p <= row[j]) {
                    return j;
                }
                p -= row[j];
            }
            
            // if we get here, there was a floating point precision problem.
            // default to the final column:
            return M.size2()-1;
        }
        
        //! Convenience method for normalizing the probability table.
        void normalize() {
            for(std::size_t i=0; i<M.size1(); ++i) {
                row_type row(M, i);
                ealib::algorithm::normalize(row.begin(), row.end(), 1.0);
            }
        }
        
        matrix_type M; //!< Probability table.
    };
    
    
    /*! Adaptive Markov gate.
     */
    template <typename RandomNumberGenerator>
    struct adaptive_gate : abstract_gate<RandomNumberGenerator> {
        typedef abstract_gate<RandomNumberGenerator> parent_type;
        typedef std::deque<std::pair<std::size_t, std::size_t> > history_type;//!< Type for tracking history of decisions.
        
        //! Constructor.
        adaptive_gate() : _disabled(false) {
        }
        
        //! Destructor.
        virtual ~adaptive_gate() {
        }
        
        //! Clears any internal state held in this gate.
        virtual void clear() {
            H.clear();
            M = Q;
        }
        
        //! Disables adaptation of gate logic.
        virtual void disable_adaptation() {
            _disabled = true;
        }
        
        //! Write this gate to a Markov graph.
        virtual void as_graph(markov_graph::vertex_descriptor v, markov_graph& G) {
            parent_type::as_graph(v, G);
            G[v].gt = vertex_properties::ADAPTIVE;
            boost::add_edge(boost::vertex(parent_type::inputs[0],G), v, edge_properties(edge_properties::REINFORCE), G);
            boost::add_edge(boost::vertex(parent_type::inputs[1],G), v, edge_properties(edge_properties::INHIBIT), G);
        }
        
        //! Returns a pointer to a newly-allocated clone of this gate.
        virtual parent_type* clone() {
            adaptive_gate* p = new adaptive_gate(*this);
            return p;
        }
        
        //! Update t+1 with input from t.
        virtual int operator()(int x, RandomNumberGenerator& rng) {
            // learn first: if one of the feedback bits is on, it means that
            // the previous behavior of this gate should be reinforced.
            // if we wait to learn until after updating, we'll be reinforcing
            // for the *next* output as well.
            while(H.size() > h) { // prune history
                H.pop_front();
            }
            
            if(!_disabled) {
                if(x & 0x01) { // reinforce
                    reinforce();
                }
                if((x>>1) & 0x01) { // inhibit
                    inhibit();
                }
            }
            x = x >> 2; // lop off the two feedback bits
            
            // now handle the next output:
            row_type row(M, x);
            double p = rng.p();
            
            for(int j=0; j<static_cast<int>(M.size2()); ++j) {
                if(p <= row[j]) {
                    H.push_back(std::make_pair(x,j));
                    return j;
                }
                p -= row[j];
            }
            
            // if we get here, there was a floating point precision problem.
            // default to the final column:
            H.push_back(std::make_pair(x,static_cast<int>(M.size2()-1)));
            return M.size2()-1;
        }
        
        //! Convenience method for normalizing the probability table.
        void normalize() {
            for(std::size_t i=0; i<M.size1(); ++i) {
                row_type row(M, i);
                ealib::algorithm::normalize(row.begin(), row.end(), 1.0);
            }
        }
        
        //! Scale the probability of output (i,j) by s.
        void scale(std::size_t i, std::size_t j, double s) {
            M(i,j) *= 1.0 + s;
            row_type row(M, i);
            ealib::algorithm::normalize(row.begin(), row.end(), row.begin(), 1.0);
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
        
        bool _disabled; //!< Adaptation disabled if true.
        std::size_t h; //!< Size of history to keep.
        history_type H; //!< History of decisions made by this node.
        weight_vector_type P; //!< Positive feedback weight vector.
        weight_vector_type N; //!< Negative feedback weight vector.
        matrix_type M; //!< "Working" probability table.
        matrix_type Q; //!< "Pristine" probability table.
    };
    
} // mkv

#endif
