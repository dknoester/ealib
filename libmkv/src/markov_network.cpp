#include <mkv/markov_network.h>
#include <mkv/instruments.h>

//! Called immediately before network nodes are updated.
void mkv::markov_network::top_half() {
    for(instrument_list_type::iterator i=_instr.begin(); i!=_instr.end(); ++i) {
        (*i)->top_half(*this);
    }
}

//! Called immediately after network nodes are updated.
void mkv::markov_network::bottom_half() {
    for(instrument_list_type::iterator i=_instr.begin(); i!=_instr.end(); ++i) {
        (*i)->bottom_half(*this);
    }
}
