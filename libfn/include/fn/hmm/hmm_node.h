#ifndef _FN_HMM_HMM_NODE_H_
#define _FN_HMM_HMM_NODE_H_

#include <boost/shared_ptr.hpp>
#include <utility>
#include <string>

#include <fn/hmm/hmm.h>

namespace fn {
	namespace hmm {

		struct hmm_header;
		
		/*! Base node type for HMM networks.
		 */
		class hmm_node {
		public:
			/*! Returns true if position i of genome g is a start codon, and if so, how 
			 much memory is required for this node, false otherwise. */
			static std::pair<bool,int> start_codon(const genome& g, int start);
			
			//! Build an HMM node based on the start codon i.
			static hmm_node* build_node(const genome& g, int start, int* where=0);
			
			//! Constructor.
			hmm_node() {
			}
			
			//! Destructor.
			virtual ~hmm_node() {
			}
			
            //! Copy this node to the given destination, returning a pointer to the node and its size in bytes.
            virtual std::pair<hmm_node*, unsigned int> copy(int* where) = 0;
            
			//! Update this node.
			virtual void update(hmm_header* hdr, int rn) = 0;
			
			//! Return the number of inputs to this node.
			virtual int num_inputs() const = 0;
			
			//! Return the number of outputs from this node.
			virtual int num_outputs() const = 0;
			
			//! Return the index of input i.
			virtual int& xinput(int i) = 0;
			
			//! Return the index of output i.
			virtual int& xoutput(int i) = 0;
            
            //! Rebase this node's inputs and outputs
            virtual void rebase(hmm_header* hdr, int oin, int oout, int ohid);
            
            //! Return a string suitable for graphviz output.
            virtual std::string graphviz() = 0;
		};
		
	} // hmm
} // fn

#endif
