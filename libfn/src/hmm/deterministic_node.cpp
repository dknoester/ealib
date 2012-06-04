#include <algorithm>
#include <bitset>
#include <sstream>
#include <string.h>

#include <fn/hmm/deterministic_node.h>
#include <fn/hmm/hmm_network.h>
#include <fn/hmm/output.h>

/*! Return the number of bytes needed to store this node in memory.
 */
int fn::hmm::detail::deterministic_node::nodesize(const genome& g, int start) {
	const int nin=hmm::options::num_inputs(g[start+NIN]);
	const int nout=hmm::options::num_outputs(g[start+NOUT]);
	return sizeof(table_header) + sizeof(int)*(nin+nout+(1<<nin));
}


/*! Constructor.
 
 Translate the gene starting at position i of genome g into a deterministic HMM node.
 */
fn::hmm::detail::deterministic_node::deterministic_node(const genome& g, int start, int* where)
: _ndr(0) {
	// are we building in previously allocated memory?
	if(where) {
		_ndr = reinterpret_cast<table_header*>(where);
	} else {
		_data.reset(new char[nodesize(g,start)]);
		_ndr = reinterpret_cast<table_header*>(_data.get());
	}
	
	// now fill in the memory:
	_ndr->start_codon[0] = g[start+SC0];
	_ndr->start_codon[1] = g[start+SC1];
	_ndr->nin = hmm::options::num_inputs(g[start+NIN]);
	_ndr->nout = hmm::options::num_outputs(g[start+NOUT]);
	_ndr->oin = sizeof(table_header)/sizeof(int);
	_ndr->oout = _ndr->oin + _ndr->nin;
	_ndr->ncol = 1; // always one for deterministic tables...
	_ndr->otable = _ndr->oout + _ndr->nout;
	
	for(int i=0; i<(_ndr->nin+_ndr->nout); ++i) {
		_ndr->_data[_ndr->oin+i] = g[start+BEGIN_IO+i];
	}
	
	const int dtablestart=start + BEGIN_IO + _ndr->nin + _ndr->nout;
	for(int i=0; i<(1<<_ndr->nin); ++i) { // row
		dtable(i) = g[dtablestart+i] % (1<<_ndr->nout);
	}
}


/*! Default ctor, used only by copy().
 */
fn::hmm::detail::deterministic_node::deterministic_node() : _ndr(0) {
}


/*! Copy this node to the given destination, returning a pointer to the node and its size in bytes.
 */
std::pair<fn::hmm::hmm_node*, unsigned int> fn::hmm::detail::deterministic_node::copy(int* where) {
    unsigned int byte_size = sizeof(table_header) + sizeof(int)*(_ndr->nin
                                                                 + _ndr->nout
                                                                 + (1<<_ndr->nin));
    deterministic_node* n = new deterministic_node();
    n->_ndr = reinterpret_cast<table_header*>(where);
    memcpy(n->_ndr, _ndr, byte_size);
    return std::make_pair(n, byte_size);
}


/*! Update this node.
 
 Read from tminus1, write to t.
 
 Assumes that row indices are sequential in the ptable.
 Note that because we use two state_vectors, we don't allow rewriting of the inputs.
 
 Note that we offset the bits in order to make the order of inputs & outputs in the
 genome align with the row and col indices.
 */
void fn::hmm::detail::deterministic_node::update(hmm_header* hdr, int rn) {
	hmm::deterministic_output(hdr,_ndr);
}

/*! Return a string suitable for graphviz output.
 */
std::string fn::hmm::detail::deterministic_node::graphviz() {
    std::ostringstream out;
    out << "[shape=record,label=\"{inputs | ";
    
    for(int i=0; i<num_inputs(); ++i) {
        out << xinput(i) << " ";
    }
    
    for(int i=0; i<(1<<num_inputs()); ++i) {
        out << "| " << std::bitset<4>(i) << " ";
    }
    
    out << "} | { outputs | ";
    for(int i=0; i<num_outputs(); ++i) {
        out << xoutput(i) << " ";
    }

    for(int i=0; i<(1<<num_inputs()); ++i) {
        out << "| " << std::bitset<4>(dtable(i)) << " ";
    }
    
    out << "}\"];";
    
    return out.str();
}
