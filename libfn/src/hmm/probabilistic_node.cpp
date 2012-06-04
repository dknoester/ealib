#include <bitset>
#include <limits>
#include <sstream>
#include <iomanip>
#include <string.h>

#include <fn/hmm/probabilistic_node.h>
#include <fn/hmm/hmm_network.h>
#include <fn/hmm/output.h>


/*! Return the number of bytes needed to store this node in memory.
 */
int fn::hmm::detail::probabilistic_node::nodesize(const genome& g, int start) {
	const int nin=hmm::options::num_inputs(g[start+NIN]);
	const int nout=hmm::options::num_outputs(g[start+NOUT]);
	return sizeof(table_header) + sizeof(int)*(nin+nout+(1<<nin)*((1<<nout)+1));		
}


/*! Constructor.
 
 Translate the gene starting at position i of genome g into a probabilistic HMM node.
 */
fn::hmm::detail::probabilistic_node::probabilistic_node(const genome& g, int start, int* where)
: _ndr(0) {
	// are we building in previously allocated memory?	
	if(where) {
		_ndr = reinterpret_cast<table_header*>(where);
	} else {
		_data.reset(new char[nodesize(g,start)]);
		_ndr = reinterpret_cast<table_header*>(_data.get());
	}
	_ndr->start_codon[0] = g[start+SC0];
	_ndr->start_codon[1] = g[start+SC1];
	_ndr->nin = hmm::options::num_inputs(g[start+NIN]);
	_ndr->nout = hmm::options::num_outputs(g[start+NOUT]);
	_ndr->ncol = (1<<_ndr->nout)+1;
	_ndr->oin = sizeof(table_header)/sizeof(int);
	_ndr->oout = _ndr->oin + _ndr->nin;
	_ndr->otable = _ndr->oout + _ndr->nout;
	
	for(int i=0; i<(_ndr->nin+_ndr->nout); ++i) {
		_ndr->_data[_ndr->oin+i] = g[start+BEGIN_IO+i];
	}
		
	const int ptablestart=start + BEGIN_IO + _ndr->nin + _ndr->nout;
	const int growsize=1<<_ndr->nout;
	for(int i=0; i<(1<<_ndr->nin); ++i) { // row
		int sum=0;
		for(int j=0; j<(1<<_ndr->nout); ++j) { // col
			int entry = g[ptablestart + i*growsize + j];
			if(entry == 0) {
				entry = 1;
			}
			sum += entry;
			ptable(i,j) = entry;
		}
		ptable(i,_ndr->ncol-1) = sum;
	}
}
	

/*! Default ctor, used only by copy().
 */
fn::hmm::detail::probabilistic_node::probabilistic_node() {
}


/*! Copy this node to the given destination, returning a pointer to the node and its size in bytes.
 */
std::pair<fn::hmm::hmm_node*, unsigned int> fn::hmm::detail::probabilistic_node::copy(int* where) {
    unsigned int byte_size = sizeof(table_header) + sizeof(int)*(_ndr->nin
                                                                 + _ndr->nout
                                                                 + (1<<_ndr->nin)*((1<<_ndr->nout)+1));
    probabilistic_node* n = new probabilistic_node();
    n->_ndr = reinterpret_cast<table_header*>(where);
    memcpy(n->_ndr, _ndr, byte_size);
    return std::make_pair(n, byte_size);
}


/*! Update this node.
 
 Read from tminus1, write to t.
 
 Assumes that row indices are sequential in the ptable.
 Note that because we use two state_vectors, we don't allow rewriting of the inputs.
 */
void fn::hmm::detail::probabilistic_node::update(hmm_header* hdr, int rn) {
	hmm::probabilistic_output(hdr, _ndr, rn);
}


/*! Return a string suitable for graphviz output.
 */
std::string fn::hmm::detail::probabilistic_node::graphviz() {
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
        out << "| ";
        for(int j=0; j<(1<<num_outputs()); ++j) {
            double f = static_cast<double>(ptable(i,j))/static_cast<double>(ptable(i,_ndr->ncol-1));
            if(f > 0.05) {
                out << std::bitset<4>(j) << "(" << std::fixed << std::setprecision(2) << f << ") ";
            }
        }
    }
    
    out << "}\"];";
    
    return out.str();
}
