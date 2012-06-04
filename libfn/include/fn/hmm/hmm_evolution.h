#ifndef _FN_HMM_HMM_EVOLUTION_H_
#define _FN_HMM_HMM_EVOLUTION_H_

#include <ea/interface.h>
#include <ea/meta_data.h>
#include <ea/analysis.h>
#include <fn/hmm/graph.h>
#include <fn/hmm/hmm_instruments.h>

namespace ea {
    LIBEA_MD_DECL(HMM_INPUT_N, "hmm.input.n", unsigned int);
    LIBEA_MD_DECL(HMM_OUTPUT_N, "hmm.output.n", unsigned int);
    LIBEA_MD_DECL(HMM_HIDDEN_N, "hmm.hidden.n", unsigned int);
    LIBEA_MD_DECL(HMM_UPDATE_N, "hmm.update.n", unsigned int);
    LIBEA_MD_DECL(HMM_INPUT_FLOOR, "hmm.gate.input_floor", unsigned int);
    LIBEA_MD_DECL(HMM_INPUT_LIMIT, "hmm.gate.input_limit", unsigned int);
    LIBEA_MD_DECL(HMM_OUTPUT_FLOOR, "hmm.gate.output_floor", unsigned int);
    LIBEA_MD_DECL(HMM_OUTPUT_LIMIT, "hmm.gate.output_limit", unsigned int);
	
	/*! HMM mutation.
	 */
	struct hmm_mutation {
		//! Mutate a single point in the given representation.
		template <typename Representation, typename EA>
		void operator()(Representation& repr, EA& ea) {
			typedef typename std::vector<typename Representation::codon_type> codon_buffer;
			
			double per_site_p = get<MUTATION_PER_SITE_P>(ea);
            int imax = get<MUTATION_UNIFORM_INT_MAX>(ea);
			for(typename Representation::iterator i=repr.begin(); i!=repr.end(); ++i) {
				if(ea.rng().p(per_site_p)) {
					*i = ea.rng()(imax);
				}
			}
			
            // gene duplication
            // (the below looks a little crude, but there were some problems related
            // to incorrect compiler optimization.)
			if(ea.rng().p(get<MUTATION_DUPLICATION_P>(ea)) && (repr.size()<20000)) {
                int start = ea.rng().uniform_integer(0, repr.size());
                int extent = ea.rng().uniform_integer(16, 513);
                codon_buffer buf(extent);
                for(int i=0; i<extent; ++i) {
                    buf[i] = repr[start+i];
                }                
				repr.insert(ea.rng().choice(repr.begin(),repr.end()), buf.begin(), buf.end());
			}

            // gene deletion
			if(ea.rng().p(get<MUTATION_DELETION_P>(ea)) && (repr.size()>1000)) {
				int start, extent;
				extent = 15+ea.rng()(512);
				start = ea.rng()(repr.size()-extent);
				repr.erase(repr.begin()+start, repr.begin()+start+extent);
			}
		}
	};
	
	
	/*! Generates random HMM individuals.
	 */
	struct hmm_random_individual {
        template <typename EA>
        typename EA::population_entry_type operator()(EA& ea) {
            typedef typename EA::representation_type representation_type;
            typename EA::individual_type ind;
            ind.name() = next<INDIVIDUAL_COUNT>(ea);
            ind.repr().resize(get<REPRESENTATION_SIZE>(ea), 127);            
            representation_type& repr=ind.repr();
			
			int i,j;
			for(i=0; i<4; ++i) {
				j=ea.rng()(repr.size()-100);
				repr[j]=42;
				repr[j+1]=(255-42);
				for(int k=2; k<20; ++k) {
					repr[j+k]=ea.rng()(256);
				}
			}
			return make_population_entry(ind,ea);
		}
	};
    

    /*! Save the dominant individual in graphviz format.
     */
    template <typename EA>
    struct hmm_genetic_graph : public ea::analysis::unary_function<EA> {
        static const char* name() { return "hmm_genetic_graph"; }

        virtual void operator()(EA& ea) {
            using namespace ea;
            using namespace ea::analysis;
            typename EA::individual_type i = analysis::find_most_fit_individual(ea);
            fn::hmm::hmm_network network(i.repr(),
                                         get<HMM_INPUT_N>(ea),
                                         get<HMM_OUTPUT_N>(ea),
                                         get<HMM_HIDDEN_N>(ea));
            datafile df(get<ANALYSIS_OUTPUT>(ea));
            std::ostringstream title;
            title << "individual=" << i.name() << "; generation=" << i.generation() << "; fitness=" << static_cast<std::string>(i.fitness());
            fn::hmm::write_graphviz(title.str(), df, fn::hmm::as_genetic_graph(network));
        }
    };
    
    
    /*! Save the dominant individual in graphviz format.
     */
    template <typename EA>
    struct hmm_reduced_graph : public ea::analysis::unary_function<EA> {
        static const char* name() { return "hmm_reduced_graph"; }
        
        virtual void operator()(EA& ea) {
            using namespace ea;
            using namespace ea::analysis;
            typename EA::individual_type i = analysis::find_most_fit_individual(ea);
            fn::hmm::hmm_network network(i.repr(),
                                         get<HMM_INPUT_N>(ea),
                                         get<HMM_OUTPUT_N>(ea),
                                         get<HMM_HIDDEN_N>(ea));
            datafile df(get<ANALYSIS_OUTPUT>(ea));
            std::ostringstream title;
            title << "individual=" << i.name() << "; generation=" << i.generation() << "; fitness=" << static_cast<std::string>(i.fitness());
            fn::hmm::write_graphviz(title.str(), df, fn::hmm::as_reduced_graph(network));
        }
    };
    
    
    /*! Save the detailed graph of the dominant individual in graphviz format.
     */
    template <typename EA>
    struct hmm_detailed_graph : public ea::analysis::unary_function<EA> {
        static const char* name() { return "hmm_detailed_graph"; }

        virtual void operator()(EA& ea) {
            using namespace ea;
            using namespace ea::analysis;
            typename EA::individual_type i = analysis::find_most_fit_individual(ea);
            fn::hmm::hmm_network network(i.repr(),
                                         get<HMM_INPUT_N>(ea),
                                         get<HMM_OUTPUT_N>(ea),
                                         get<HMM_HIDDEN_N>(ea));
            datafile df(get<ANALYSIS_OUTPUT>(ea));
            std::ostringstream title;
            title << "individual=" << i.name() << "; generation=" << i.generation() << "; fitness=" << static_cast<std::string>(i.fitness());
            fn::hmm::write_graphviz(title.str(), df, fn::hmm::as_reduced_graph(network), true);
        }
    };
    
    
    /*! Save the causal graph of the dominant individual in graphviz format.
     */
    template <typename EA>
    struct hmm_causal_graph : public ea::analysis::unary_function<EA> {
        static const char* name() { return "hmm_causal_graph"; }

        virtual void operator()(EA& ea) {
            using namespace ea;
            using namespace ea::analysis;
            typename EA::individual_type i = analysis::find_most_fit_individual(ea);
            fn::hmm::hmm_network network(i.repr(),
                                         get<HMM_INPUT_N>(ea),
                                         get<HMM_OUTPUT_N>(ea),
                                         get<HMM_HIDDEN_N>(ea));
            datafile df(get<ANALYSIS_OUTPUT>(ea));
            std::ostringstream title;
            title << "individual=" << i.name() << "; generation=" << i.generation() << "; fitness=" << static_cast<std::string>(i.fitness());
            fn::hmm::write_graphviz(title.str(), df, fn::hmm::as_causal_graph(network));
        }
    };

} // ea

#endif
