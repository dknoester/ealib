#ifndef _FN_HMM_PROBABILISTIC_NODE_H_
#define _FN_HMM_PROBABILISTIC_NODE_H_

#include <boost/shared_array.hpp>
#include <boost/shared_ptr.hpp>
#include <utility>

#include <fn/hmm/hmm_header.h>
#include <fn/hmm/hmm_node.h>

namespace fn {
	namespace hmm {
		namespace detail {
			
			/*! Probabilistic HMM node.
			 */
			class probabilistic_node : public hmm_node {
			public:			
				//! Return the number of bytes needed to store this node in memory.
				static int nodesize(const genome& g, int start);
				
				//! Offsets into the genome.
				enum {
					SC0=0, // start codon
					SC1=1,
					NIN=2, // number of inputs
					NOUT=3, // number of outputs
					BEGIN_IO=4 // beginning of IO indices
				};
				
				//! Constructor.
				probabilistic_node(const genome& g, int start, int* where=0);
				
				//! Destructor.
				virtual ~probabilistic_node() {
				}
                
                //! Copy this node to the given destination, returning a pointer to the node and its size in bytes.
                std::pair<hmm_node*, unsigned int> copy(int* where);
				
				//! Update this node.
				virtual void update(hmm_header* hdr, int rn);
				
				//! Return the number of inputs to this node.
				inline virtual int num_inputs() const { return _ndr->nin; }
				
				//! Return the number of outputs from this node.
				inline virtual int num_outputs() const { return _ndr->nout; }
				
				//! Return the index of input i.
				inline virtual int& xinput(int i) { return _ndr->xinput(i); }
				
				//! Return the index of output i.
				inline virtual int& xoutput(int i) { return _ndr->xoutput(i); }	
				
				//! Return a probability table entry.
				inline int& ptable(int i, int j) { return _ndr->entry(i,j); }
				
                //! Return a string suitable for graphviz output.
                virtual std::string graphviz();

			protected:
				table_header* _ndr; //!< This node's header / table_header in memory.
				boost::shared_array<char> _data; //!< Memory for this node.
				
			private:
				probabilistic_node();
			};
			
		} // detail	
	} // hmm
} // fn

#endif
