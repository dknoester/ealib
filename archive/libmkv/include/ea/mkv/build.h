/* build.h
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
#ifndef _MKV_EA_MKV_BUILD_H_
#define _MKV_EA_MKV_BUILD_H_

#include <ea/mkv/common.h>

namespace mkv {
    namespace detail {
        
        /*! Parse the inputs and outputs for a Markov network gate.
         */
        template <typename Network, typename ForwardIterator, typename MetaData>
        void build_io(Network& net, std::size_t& layer, index_list_type& inputs, index_list_type& outputs, ForwardIterator& h, MetaData& md) {
            using namespace ealib;
            using namespace ealib::algorithm;
            
            int nin=modnorm(*h++, static_cast<int>(get<GATE_INPUT_FLOOR>(md)), static_cast<int>(get<GATE_INPUT_LIMIT>(md)));
            int nout=modnorm(*h++, static_cast<int>(get<GATE_OUTPUT_FLOOR>(md)), static_cast<int>(get<GATE_OUTPUT_LIMIT>(md)));
            
            inputs.clear();
            inputs.insert(inputs.end(), h, h+nin);
            std::transform(inputs.begin(), inputs.end(), inputs.begin(), std::bind2nd(std::modulus<int>(), net.nstates()));
            h+=nin;
            
            outputs.clear();
            outputs.insert(outputs.end(), h, h+nout);
            std::transform(outputs.begin(), outputs.end(), outputs.begin(), std::bind2nd(std::modulus<int>(), net.nstates()));
            h+=nout;
        }
        
        /*! Build a logic gate.
         */
        template <typename Network, typename ForwardIterator, typename MetaData>
        void build_logic_gate(Network& net, ForwardIterator h, MetaData& md) {
            std::size_t layer=0;
            index_list_type inputs, outputs;
            build_io(net, layer, inputs, outputs, h, md);
            logic_gate g(inputs, outputs, h);
            net.push_back(g);
        }
        
        /*! Build a probabilistic gate.
         */
        template <typename Network, typename ForwardIterator, typename MetaData>
        void build_probabilistic_gate(Network& net, ForwardIterator h, MetaData& md) {
            std::size_t layer=0;
            index_list_type inputs, outputs;
            build_io(net, layer, inputs, outputs, h, md);
            markov_gate g(inputs, outputs, h);
            net.push_back(g);
        }
        
        /*! Build an adaptive gate.
         */
        template <typename Network, typename ForwardIterator, typename MetaData>
        void build_adaptive_gate(Network& net, ForwardIterator h, MetaData& md) {
            using namespace ealib;
            using namespace ealib::algorithm;
            
            std::size_t layer=0;
            index_list_type inputs, outputs;
            build_io(net, layer, inputs, outputs, h, md);
            
            int nhistory=modnorm(*h++, static_cast<int>(get<GATE_HISTORY_FLOOR>(md)), static_cast<int>(get<GATE_HISTORY_LIMIT>(md)));
            int posf=*h++ % net.nstates();
            int negf=*h++ % net.nstates();
            
            weight_vector_type poswv(h, h+nhistory);
            std::transform(poswv.begin(), poswv.end(), poswv.begin(),
                           std::bind2nd(std::modulus<int>(), get<GATE_WV_STEPS>(md)+1));
            std::transform(poswv.begin(), poswv.end(), poswv.begin(),
                           std::bind2nd(std::multiplies<double>(), 1.0/get<GATE_WV_STEPS>(md)));
            h+=nhistory;
            weight_vector_type negwv(h, h+nhistory);
            std::transform(negwv.begin(), negwv.end(), negwv.begin(),
                           std::bind2nd(std::modulus<int>(), get<GATE_WV_STEPS>(md)+1));
            std::transform(negwv.begin(), negwv.end(), negwv.begin(),
                           std::bind2nd(std::multiplies<double>(), -1.0/get<GATE_WV_STEPS>(md)));
            h+=nhistory;
            
            adaptive_gate g(nhistory, posf, poswv, negf, negwv, inputs, outputs, h);
            net.push_back(g);
        }
        
        template <typename ForwardIterator>
        std::size_t get_layer(ForwardIterator h, int max_layer) {
            using namespace ealib;
            using namespace ealib::algorithm;
            return modnorm(*h, 0, max_layer);
        }
        
        /*! Build a gate from f with the given meta data.
         */
        template <typename ForwardIterator, typename MetaData>
        void build_gate(int start_codon, markov_network& net, ForwardIterator f, MetaData& md) {
            switch(start_codon) {
                case PROBABILISTIC: { // build a markov gate
                    build_probabilistic_gate(net, f, md);
                    break;
                }
                case LOGIC: { // build a logic gate
                    build_logic_gate(net, f, md);
                    break;
                }
                case ADAPTIVE: { // build an adaptive gate
                    build_adaptive_gate(net, f, md);
                    break;
                }
                default: {
                    throw ealib::bad_argument_exception("mkv/ea/build.h: unknown gate type");
                }
            }
        }
        
        /*! Build a Deep Markov network from the genome [f,l), with the given meta data.
         */
        template <typename ForwardIterator, typename MetaData>
        void build_deep_markov_network(deep_markov_network& net, ForwardIterator f, ForwardIterator l, MetaData& md, gate_selector_type& gates) {
            if(f == l) { return; }
            
            ForwardIterator last=f;
            ++f;
            
            for( ; f!=l; ++f, ++last) {
                int start_codon = *f + *last;
                if((start_codon == 255) && (gates.count(static_cast<mkv::gate_type>(*last)) > 0)) {
                    markov_network& layer=net[detail::get_layer(f+1,net.size())];
                    detail::build_gate(*last, layer, f+2, md);
                }
            }
        }
        
        /*! Build a Markov network from the genome [f,l), with the given meta data.
         */
        template <typename ForwardIterator, typename MetaData>
        void build_markov_network(markov_network& net, ForwardIterator f, ForwardIterator l, MetaData& md, gate_selector_type& gates) {
            if(f == l) { return; }
            
            ForwardIterator last=f;
            ++f;
            
            for( ; f!=l; ++f, ++last) {
                int start_codon = *f + *last;
                if((start_codon == 255) && (gates.count(static_cast<mkv::gate_type>(*last)) > 0)) {
                    detail::build_gate(*last, net, f+1, md);
                }
            }
        }
        
    } // detail
} // mkv

#endif
