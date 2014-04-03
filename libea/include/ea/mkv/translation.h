/* translate.h
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
#ifndef _EA_MKV_TRANSLATE_H_
#define _EA_MKV_TRANSLATE_H_

#include <set>
#include <string>

#include <ea/mkv/gates.h>

namespace ealib {
    namespace mkv {
        LIBEA_MD_DECL(MKV_LAYERS_N, "markov_network.layers.n", std::size_t);
        LIBEA_MD_DECL(MKV_INPUT_N, "markov_network.input.n", std::size_t);
        LIBEA_MD_DECL(MKV_OUTPUT_N, "markov_network.output.n", std::size_t);
        LIBEA_MD_DECL(MKV_HIDDEN_N, "markov_network.hidden.n", std::size_t);
        LIBEA_MD_DECL(MKV_INITIAL_GATES, "markov_network.initial_gates", std::size_t);
        LIBEA_MD_DECL(MKV_GATE_TYPES, "markov_network.gate_types", std::string);

        //! Start codon detector.
        struct start_codon {
            template <typename ForwardIterator>
            bool operator()(ForwardIterator f) const {
                return ((*f + *(f+1)) == 255);
            }
        };

        enum gate_type { LOGIC=42, PROBABILISTIC=43, ADAPTIVE=44 };
        
        /*! Translator to build a Markov network from a circular genome.
         */
        class genome_translator {
        public:
            typedef std::set<gate_type> enabled_gate_set;
            
            //! Constructor.
            genome_translator(int in_lb=4, int in_ub=4, int out_lb=4, int out_ub=4,
                       int h_lb=4, int h_ub=4, int wv_steps=1024)
            : _in_lb(in_lb), _in_ub(in_ub), _out_lb(out_lb), _out_ub(out_ub), _h_lb(h_lb), _h_ub(h_ub), _wv_steps(wv_steps) {
                _enabled.insert(LOGIC);
                _enabled.insert(PROBABILISTIC);
                _enabled.insert(ADAPTIVE);
            }
            
            //! Disable a gate type.
            void disable(gate_type g) {
                _enabled.erase(g);
            }
            
            //! Add the gene starting at f to Markov network N.
            template <typename ForwardIterator, typename MarkovNetwork>
            void operator()(ForwardIterator f, MarkovNetwork& N) const {
                if(!_enabled.count(static_cast<gate_type>(*f))) {
                    return;
                }
                switch(*f) {
                    case LOGIC: { // build a logic gate
                        parse_logic_gate(f+2, N);
                        break;
                    }
                    case PROBABILISTIC: { // build a markov gate
                        parse_probabilistic_gate(f+2, N);
                        break;
                    }
                    case ADAPTIVE: { // build an adaptive gate
                        parse_adaptive_gate(f+2, N);
                        break;
                    }
                    default: {
                        // do nothing; bogus start codon.
                    }
                }
            }
            
            //! Retrieves the set of enabled gate types.
            enabled_gate_set& enabled() { return _enabled; }
            
        protected:
            //! Parse the number and indices for a gate's IO vector.
            template <typename ForwardIterator, typename MarkovNetwork>
            ForwardIterator parse_io(ForwardIterator f, index_vector_type& inputs, index_vector_type& outputs, MarkovNetwork& N) const {
                using namespace algorithm;
                int nin=modnorm(*f++, _in_lb, _in_ub);
                int nout=modnorm(*f++, _out_lb, _out_ub);
                inputs.resize(nin);
                outputs.resize(nout);
                for(int i=0; i<nin; ++i,++f) {
                    inputs[i] = modnorm(static_cast<std::size_t>(*f), static_cast<std::size_t>(0), static_cast<std::size_t>(N.nstates()));
                }
                for(int i=0; i<nout; ++i,++f) {
                    outputs[i] = modnorm(static_cast<std::size_t>(*f), static_cast<std::size_t>(0), static_cast<std::size_t>(N.nstates()));
                }
                return f;
            }
            
            //! Parse a logic gate from f and add it to Markov network N.
            template <typename ForwardIterator, typename MarkovNetwork>
            void parse_logic_gate(ForwardIterator f, MarkovNetwork& N) const {
                typedef logic_gate<typename MarkovNetwork::rng_type> gate_type;
                boost::shared_ptr<gate_type> p(new gate_type());
                gate_type& g=*p;
                f = parse_io(f, g.inputs, g.outputs, N);
                g.M.resize(1<<g.inputs.size());
                for(std::size_t i=0; i<g.M.size(); ++i, ++f) {
                    g.M[i] = *f;
                }
                
                N.gates().push_back(p);
            }
            
            //! Parse a probabilistic gate from f and add it to Markov network N.
            template <typename ForwardIterator, typename MarkovNetwork>
            void parse_probabilistic_gate(ForwardIterator f, MarkovNetwork& N) const {
                typedef probabilistic_gate<typename MarkovNetwork::rng_type> gate_type;
                boost::shared_ptr<gate_type> p(new gate_type());
                gate_type& g=*p;
                f = parse_io(f, g.inputs, g.outputs, N);
                g.M.resize(1<<g.inputs.size(), 1<<g.outputs.size());
                for(std::size_t i=0; i<g.M.size1(); ++i) {
                    row_type row(g.M, i);
                    f = algorithm::normalize(f, f+g.M.size2(), row.begin(), 1.0);
                }
                
                N.gates().push_back(p);
            }
            
            //! Parse an adaptive gate from f and add it to Markov network N.
            template <typename ForwardIterator, typename MarkovNetwork>
            void parse_adaptive_gate(ForwardIterator f, MarkovNetwork& N) const {
                using namespace algorithm;
                
                typedef adaptive_gate<typename MarkovNetwork::rng_type> gate_type;
                boost::shared_ptr<gate_type> p(new gate_type());
                gate_type& g=*p;
                
                int nin=2+modnorm(*f++, _in_lb, _in_ub); // +2 for positive and negative feedback
                int nout=modnorm(*f++, _out_lb, _out_ub);
                g.inputs.resize(nin);
                g.outputs.resize(nout);
                for(int i=0; i<nin; ++i,++f) {
                    g.inputs[i] = modnorm(static_cast<std::size_t>(*f), static_cast<std::size_t>(0), static_cast<std::size_t>(N.nstates()));
                }
                for(int i=0; i<nout; ++i,++f) {
                    g.outputs[i] = modnorm(static_cast<std::size_t>(*f), static_cast<std::size_t>(0), static_cast<std::size_t>(N.nstates()));
                }
                
                g.h = modnorm(*f++, _h_lb, _h_ub);
                g.P.resize(g.h);
                for(std::size_t i=0; i<g.h; ++i,++f) {
                    g.P[i] = (*f % _wv_steps) * (1.0 / static_cast<double>(_wv_steps));
                }
                g.N.resize(g.h);
                for(std::size_t i=0; i<g.h; ++i,++f) {
                    g.N[i] = (*f % _wv_steps) * (1.0 / static_cast<double>(_wv_steps));
                }
                g.M.resize(1<<nin, 1<<nout);
                for(std::size_t i=0; i<g.M.size1(); ++i) {
                    row_type row(g.M, i);
                    f = algorithm::normalize(f, f+g.M.size2(), row.begin(), 1.0);
                }
                
                N.gates().push_back(p);
            }
            
            int _in_lb, _in_ub, _out_lb, _out_ub; //!< Fan-in and fan-out lower and upper bounds.
            int _h_lb, _h_ub, _wv_steps; //!< History vector lower and upper bounds.
            enabled_gate_set _enabled; //!< Enabled gates.
        };
        
        
        /*! Translator to build a Markov network from a circular genome.
         */
        class deep_genome_translator : public genome_translator {
        public:
            typedef genome_translator base_type;
            
            //! Constructor.
            deep_genome_translator(int in_lb=4, int in_ub=4, int out_lb=4, int out_ub=4,
                                   int h_lb=4, int h_ub=4, int wv_steps=1024)
            : genome_translator(in_lb, in_ub, out_lb, out_ub, h_lb, h_ub, wv_steps) {
            }
            
            //! Add the gene starting at f to Markov network N.
            template <typename ForwardIterator, typename MarkovNetwork>
            void operator()(ForwardIterator f, MarkovNetwork& N) const {
                if(!_enabled.count(static_cast<gate_type>(*f))) {
                    return;
                }
                std::size_t layer = *(f+2) % N.nlayers();
                switch(*f) {
                    case LOGIC: { // build a logic gate
                        parse_logic_gate(f+3, N[layer]);
                        break;
                    }
                    case PROBABILISTIC: { // build a markov gate
                        parse_probabilistic_gate(f+3, N[layer]);
                        break;
                    }
                    case ADAPTIVE: { // build an adaptive gate
                        parse_adaptive_gate(f+3, N[layer]);
                        break;
                    }
                    default: {
                        // do nothing; bogus start codon.
                    }
                }
            }
        };
        
        /*! Generates random Markov network-based individuals.
         */
        struct ancestor_generator {
            template <typename EA>
            typename EA::representation_type operator()(EA& ea) {
                using namespace ealib;
                
                typename EA::representation_type repr;
                repr.resize(get<REPRESENTATION_INITIAL_SIZE>(ea), 127);
                
                for(std::size_t i=0; i<get<MKV_INITIAL_GATES>(ea); ++i) {
                    std::size_t csize=ea.rng()(get<MUTATION_INDEL_MIN_SIZE>(ea),
                                               get<MUTATION_INDEL_MAX_SIZE>(ea));
                    int j=ea.rng()(repr.size()-csize);
                    genome_translator& t = ea.config().translator;
                    int gate=*ea.rng().choice(t.enabled().begin(), t.enabled().end());
                    repr[j] = gate;
                    repr[j+1] = 255-gate;
                    for(std::size_t k=2; k<csize; ++k) {
                        repr[j+k]=ea.rng()(get<MUTATION_UNIFORM_INT_MIN>(ea), get<MUTATION_UNIFORM_INT_MAX>(ea));
                    }
                }
                return repr;
            }
        };

    } // mkv
} // ealib

#endif
