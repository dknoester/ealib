/* common.h
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
#ifndef _MKV_EA_MKV_COMMON_H_
#define _MKV_EA_MKV_COMMON_H_

#include <set>

#include <ea/meta_data.h>

namespace mkv {

    LIBEA_MD_DECL(MKV_DESC, "markov_network.desc", std::string);
    LIBEA_MD_DECL(MKV_UPDATE_N, "markov_network.update.n", int);
    LIBEA_MD_DECL(MKV_GATE_TYPES, "markov_network.gate_types", std::string);
    LIBEA_MD_DECL(MKV_INITIAL_GATES, "markov_network.initial_gates", std::size_t);
    LIBEA_MD_DECL(MKV_MIN_GATES, "markov_network.min_gates", std::size_t);
    LIBEA_MD_DECL(MKV_MAX_GATES, "markov_network.max_gates", std::size_t);
    
    LIBEA_MD_DECL(GATE_INPUT_LIMIT, "markov_network.gate.input.limit", int);
    LIBEA_MD_DECL(GATE_INPUT_FLOOR, "markov_network.gate.input.floor", int);
    LIBEA_MD_DECL(GATE_OUTPUT_LIMIT, "markov_network.gate.output.limit", int);
    LIBEA_MD_DECL(GATE_OUTPUT_FLOOR, "markov_network.gate.output.floor", int);
    LIBEA_MD_DECL(GATE_HISTORY_FLOOR, "markov_network.gate.history.floor", int);
    LIBEA_MD_DECL(GATE_HISTORY_LIMIT, "markov_network.gate.history.limit", int);
    LIBEA_MD_DECL(GATE_WV_STEPS, "markov_network.gate.wv_steps", int);
    
    enum gate_type { PROBABILISTIC=42, LOGIC=43, ADAPTIVE=44 };
    
    typedef std::set<gate_type> gate_selector_type;
    
} // mkv

#endif
