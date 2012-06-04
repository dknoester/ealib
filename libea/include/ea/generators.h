#ifndef _EA_GENERATORS_H_
#define _EA_GENERATORS_H_

#include <ea/interface.h>
#include <ea/meta_data.h>


namespace ea {
	
	/*! Adapter to make a selection strategy behave like a std::generator.
	 */
	template <typename SelectionStrategy, typename Population, typename EA>
	struct selection_generator {
		typedef typename Population::value_type value_type;
		typedef SelectionStrategy selection_type;
		typedef EA ea_type;
		
		selection_generator(Population& p, EA& ea) : _src(p), _ea(ea), _selector(p,ea) {
		}

		value_type operator()() {
			return _selector(_src, _ea);
		}

        value_type operator()(EA& ea) {
			return _selector(_src, _ea);
		}

		Population& _src; //!< Population from which individuals will be drawn.
		EA& _ea; //!< Evolutionary algorithm.
		selection_type _selector; //!< Selection strategy.
	};
	
    /*! Generator for a monotonically increasing series.
     */
    template <typename T>
    struct series_generator {
        typedef T result_type;
        
        series_generator(T init, T delta): _current(init), _delta(delta) {
        }
        
        result_type operator()() {
            result_type r = _current;
            _current += _delta;
            return r;
        }
        
        T _current;
        T _delta;
    };
    
    
} // ea

#endif
