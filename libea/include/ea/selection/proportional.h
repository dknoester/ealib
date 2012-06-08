#ifndef _EA_SELECTION_PROPORTIONAL_H_
#define _EA_SELECTION_PROPORTIONAL_H_

#include <ea/attributes.h>
#include <ea/interface.h>


namespace ea {
	namespace selection {
        
		/*! Proportional selection.
		 
         This strategy selects individuals proportionally by some attribute,
         usually fitness.
		 */
        template <typename AttributeAccessor=attributes::fitness>
		struct proportional {
            typedef AttributeAccessor acc_type; //!< Accessor for proportional selection.
            
			//! Initializing constructor.
			template <typename Population, typename EA>
			proportional(std::size_t n, Population& src, EA& ea) : _sum(0.0) {
                for(typename Population::iterator i=src.begin(); i!=src.end(); ++i) {
                    _sum += static_cast<double>(_acc(**i));
                }
                assert(_sum > 0.0);
			}
            
            //! Adjust the sum of fitnesses by val.
            void adjust(double val) {
                _sum += val;
            }
			
			/*! Select n individuals via fitness-proportional selection.
             
             We go through a few hoops here in order to avoid n linear time lookups...
             */
			template <typename Population, typename EA>
			void operator()(Population& src, Population& dst, std::size_t n, EA& ea) {
                // build a whole bunch of random numbers, and sort them:
                std::vector<double> rnums(n);
                std::generate(rnums.begin(), rnums.end(), ea.rng().uniform_real_rng(0,1.0));
                std::sort(rnums.begin(), rnums.end()); // ascending
                
                // since the rnums are sorted, we can scan through linearly.
                // since the fsum is monotonically increasing, all we need to do is scale
                // the current rnum by the fraction of fitness we're currently looking at:
                typename Population::iterator p=src.begin();
                double running=static_cast<double>(_acc(**p));
                for(std::vector<double>::iterator i=rnums.begin(); i!=rnums.end(); ++i) {
                    // while our (the fraction of) running fitness is strictly less than the current
                    // random number, go to the next individual in the population:
                    while((running/_sum) < (*i)) {
                        ++p;
                        assert(p!=src.end());
                        running += static_cast<double>(_acc(**p));
                    }
                    
                    // ok, running fitness is >= random number; select the p'th
                    // individual for replication:
                    dst.append(p);
                }
            }
            
            acc_type _acc; //!< Accessor for value used for proportional selection.
			double _sum; //!< Sum of fitnesses in the population being selected from.
		};
		
        
        
        /*! Roulette wheel selection.
		 
		 Returns an iterator selected from the range [f,l), based on the sum of values
		 of the items in the sequence.
		 
		 If a value could not be found, return l.
		 */
		template <typename T, typename InputIterator, typename EA>
		InputIterator roulette_wheel(const T& target, InputIterator f, InputIterator l, EA& ea) {
			T running=0.0;
			for( ; f!=l; ++f) {
				running += static_cast<T>(ind(f,ea).fitness());
				if(running >= target) {
					return f;
				}
			}			
			return l;
		}
        
        
        
	} // selection
} // ealib

#endif
