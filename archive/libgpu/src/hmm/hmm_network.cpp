/* hmm_network.cpp
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
#include <assert.h>
#include <strings.h>

#include <fn/hmm/hmm_network.h>

/*! Constructor.
 */
fn::hmm::hmm_network::hmm_network(const genome& g, int in, int out, int hidden)
: _mem_needed(0), _h(0) {
	// locate the genes and find out how much total memory is needed:
	_mem_needed=sizeof(hmm_header);
	std::vector<std::pair<int,int> > genes;
	
	for(std::size_t i=0; i<g.size(); ++i) {
		std::pair<bool,int> sc=hmm_node::start_codon(g,i);
		if(sc.first) {
			_mem_needed += sc.second;
			genes.push_back(std::make_pair(i,sc.second));
		}
	}
	
	// alloc and zero-out memory:
	int nstates = in + out + hidden;
	_mem_needed += sizeof(int)*(nstates*2); // state vectors
	_mem_needed += sizeof(int)*genes.size(); // node map
	_rdata.reset(new char[_mem_needed]);
	bzero(_rdata.get(), _mem_needed);
	
	// fill in the hmm_header:
	_h = reinterpret_cast<hmm_header*>(_rdata.get());
	_h->nin = in;
	_h->nout = out;
	_h->nhidden = hidden;
	_h->nstates = nstates;
	_h->nnodes = genes.size();
	_h->onodemap = sizeof(hmm_header)/sizeof(int); // nodemap is right after header
	_h->otminus1 = _h->onodemap + _h->nnodes; // t-1 state vector follows nodemap
	_h->ot = _h->otminus1 + _h->nstates; // t state vector follow t-1 (initially; rotation moves these around)
	
	//debug:
	//std::fill(_h->_states,_h->_states+_h->nhidden+2*_h->nstates,0xdead);
	
	// now set up the nodes:
	unsigned int onode=_h->ot + _h->nstates;
	for(std::size_t i=0; i<genes.size(); ++i) {
		_h->onode(i) = onode;
		boost::shared_ptr<hmm_node> n(hmm_node::build_node(g,genes[i].first,&(_h->_data[onode])));
		// mod the input and output indices for each node
		for(int j=0; j<n->num_inputs(); ++j) {
			n->xinput(j) %= _h->nstates;
		}
		for(int j=0; j<n->num_outputs(); ++j) {
			n->xoutput(j) %= _h->nstates;
		}
		_nodes.push_back(n);
		onode += genes[i].second/sizeof(int);
	}
	assert(onode==(_mem_needed/sizeof(int)));			
}


/*! Destructor.
 */
fn::hmm::hmm_network::~hmm_network() {
}


/*! Concatenate HMM networks.
 */
fn::hmm::hmm_network& fn::hmm::hmm_network::operator+=(const hmm_network& that) {
    // check for self-assignment:
    if(this == &that) {
        return *this;
    }
    
    // basic dimensions of the network:
    int in = num_inputs() + that.num_inputs();
    int out = num_outputs() + that.num_outputs();
    int hidden = num_hidden() + that.num_hidden();
    int nstates = in + out + hidden;
    int nnodes = num_nodes() + that.num_nodes();
    
    // the total memory needed by these two networks is the sum of each network 
    // minus the size of the header (there are two copies of the header).
    int mem_needed = _mem_needed + that._mem_needed - sizeof(hmm_header);
    
    // alloc and zero memory:
    boost::shared_array<char> rdata(new char[mem_needed]);
    bzero(rdata.get(), mem_needed);

    // fill in the hmm_header:
    hmm_header* h = reinterpret_cast<hmm_header*>(rdata.get());
	h->nin = in;
	h->nout = out;
	h->nhidden = hidden;
	h->nstates = nstates;
	h->nnodes = nnodes;
	h->onodemap = sizeof(hmm_header)/sizeof(int); // nodemap is right after header
	h->otminus1 = h->onodemap + h->nnodes; // t-1 state vector follows nodemap
	h->ot = h->otminus1 + h->nstates; // t state vector follow t-1 (initially; rotation moves these around)

	// offset counter for placing nodes:
	unsigned int onode = h->ot + h->nstates;
    node_list nodes;

    // copy nodes -- since we now have more states than we did, we need to rebase
    // all in order to get the inputs, outputs, and hidden states lined up.
    // remember that the order of states in the state vectors is:
    //  | inputs | outputs | hidden |

    // these are offsets that are to be applied to "this" nodes:
    int oin=0;
    int oout=that.num_inputs();
    int ohid=that.num_inputs() + that.num_outputs();
    
    for(std::size_t i=0; i<_nodes.size(); ++i) {
        h->onode(i) = onode;
        std::pair<boost::shared_ptr<hmm_node>, unsigned int> n = _nodes[i]->copy(&(h->_data[onode]));        
        n.first->rebase(_h, oin, oout, ohid);
        nodes.push_back(n.first);
        onode += n.second/sizeof(int);
    }

    // now, adjust the offsets so that they can be applied to "that" nodes:
    oin += num_inputs();
    oout += num_outputs();
    ohid += num_hidden();
    
    // copy "that" nodes, and rebase their inputs and outputs:
    for(std::size_t i=0; i<that._nodes.size(); ++i) {
        h->onode(i+_nodes.size()) = onode;
        std::pair<boost::shared_ptr<hmm_node>, unsigned int> n = that._nodes[i]->copy(&(h->_data[onode]));
        n.first->rebase(that._h, oin, oout, ohid);
        nodes.push_back(n.first);
        onode += n.second/sizeof(int);
    }	
    
    assert(onode==(mem_needed/sizeof(int)));

    // all done, so update this network's state:
    _mem_needed = mem_needed;
    _h = h;
    _rdata = rdata;
    _nodes = nodes;
    
    return *this;
}


/*! Multiply this network, replicating all states and nodes n times.
 */
fn::hmm::hmm_network& fn::hmm::hmm_network::operator*=(std::size_t n) {
    // basic dimensions of the network:
    int in = num_inputs() * n;
    int out = num_outputs() * n;
    int hidden = num_hidden() * n;
    int nstates = in + out + hidden;
    int nnodes = num_nodes() * n;
    
    // the total memory needed by these two networks is the sum of each network 
    // minus the size of the header (there are two copies of the header).
    int mem_needed = _mem_needed * n - (n-1)*sizeof(hmm_header);
    
    // alloc and zero memory:
    boost::shared_array<char> rdata(new char[mem_needed]);
    bzero(rdata.get(), mem_needed);
    
    // fill in the hmm_header:
    hmm_header* h = reinterpret_cast<hmm_header*>(rdata.get());
	h->nin = in;
	h->nout = out;
	h->nhidden = hidden;
	h->nstates = nstates;
	h->nnodes = nnodes;
	h->onodemap = sizeof(hmm_header)/sizeof(int); // nodemap is right after header
	h->otminus1 = h->onodemap + h->nnodes; // t-1 state vector follows nodemap
	h->ot = h->otminus1 + h->nstates; // t state vector follow t-1 (initially; rotation moves these around)
    
	// offset counter for placing nodes:
	unsigned int onode = h->ot + h->nstates;
    node_list nodes;

    // offsets for rebasing each gate:
    int oin=0;
    int oout=(n-1)*num_inputs();
    int ohid=(n-1)*num_inputs() + (n-1)*num_outputs();
    
    // now duplicate the nodes -- rebase the inputs and outputs for each node.
    // remember that the order of states in the state vectors is:
    //  | inputs | outputs | hidden |
    for(std::size_t i=0; i<n; ++i) {
        for(std::size_t j=0; j<_nodes.size(); ++j) {
            h->onode(i*_nodes.size()+j) = onode;
            std::pair<boost::shared_ptr<hmm_node>, unsigned int> n = _nodes[j]->copy(&(h->_data[onode]));        
            n.first->rebase(_h, oin, oout, ohid);
            nodes.push_back(n.first);
            onode += n.second/sizeof(int);
        }
        // adjust offsets so that they can be applied to the next network:
        oin += num_inputs();
        oout += num_outputs();
        ohid += num_hidden();
    }
    
    assert(onode==(mem_needed/sizeof(int)));
    
    // all done, so update this network's state:
    _mem_needed = mem_needed;
    _h = h;
    _rdata = rdata;
    _nodes = nodes;
    return *this; 
}



/*! Outputs the t and t-1 state vectors.
 */
std::ostream& fn::hmm::operator<<(std::ostream& out, const hmm_network& h) {
	out << "t:" << std::endl;
	std::copy(h.header()->t_begin(), h.header()->t_end(), std::ostream_iterator<int>(out, "\n"));
	out << "t-1:" << std::endl;
	std::copy(h.header()->tminus1_begin(), h.header()->tminus1_end(), std::ostream_iterator<int>(out, "\n"));
	return out;
}
