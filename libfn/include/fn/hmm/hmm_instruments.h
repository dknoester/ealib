#ifndef _FN_HMM_HMM_INSTRUMENTS_H_
#define _FN_HMM_HMM_INSTRUMENTS_H_

#include <algorithm>
#include <iostream>
#include <iterator>
#include <vector>
#include <ea/algorithm.h>
#include <fn/hmm/hmm.h>
#include <fn/hmm/hmm_header.h>

namespace fn {
	namespace hmm {
        
        /*! Base class for HMM instruments.
         */
        class hmm_instrument {
        public:
            hmm_instrument() { }
            virtual ~hmm_instrument() { }
            virtual void upper_half(hmm_header* hdr) { }
            virtual void bottom_half(hmm_header* hdr) { }
            virtual void clear() { }
            virtual std::ostream& operator<<(std::ostream& out) { return out; }
        };
        
        
        /*! Used to fix a state to a given value.
         */
        class fix : public hmm_instrument {
        public:
            
            //! Constructor.
            fix(std::size_t n, int v) : _n(n), _v(v) {
            }
            
            //! Destructor.
            virtual ~fix() {
            }
            
            //! Called before a network update, after inputs have been established.
            virtual void upper_half(hmm_header* hdr) {
                *(hdr->tminus1_begin()+_n) = _v;
            }

        protected:
            std::size_t _n; //!< which state is being held
            int _v; //!< value the state is being held to
        };        

        
        /*! Used to capture a trace of the behavior of an HMM network.
         */
        class trace : public hmm_instrument {
        public:
            typedef hmm::state_vector_type state_vector_type;
            
            struct record {
                enum record_type { TMINUS1, T, CLEAR };
                record(int time, record_type rt) : t(time), type(rt) { }
                int t;
                record_type type;
                state_vector_type sv;
            };
            
            
            typedef std::vector<record> history_type; //!< Type for storing trace data.
            
            //! Constructor.
            trace() : _n(0) {
            }
            
            //! Destructor.
            virtual ~trace() {
            }
            
            //! Called before a network update, after inputs have been established.
            virtual void upper_half(hmm_header* hdr) {
                // t_minus1 contains data of interest
                record r(_n, record::TMINUS1);
                std::copy(hdr->tminus1_begin(), hdr->tminus1_end(), std::back_insert_iterator<state_vector_type>(r.sv));
                //std::copy(hdr->tminus1_hidden(), hdr->tminus1_end(), std::back_insert_iterator<state_vector_type>(r.sv));
                _history.push_back(r);
            }
            
            //! Called after a network update, after outputs have been established.
            virtual void bottom_half(hmm_header* hdr) {
                // t contains data of interest
                record r(_n, record::T);
                std::copy(hdr->tminus1_begin(), hdr->tminus1_outputs(), std::back_insert_iterator<state_vector_type>(r.sv));
                std::copy(hdr->t_outputs(), hdr->t_end(), std::back_insert_iterator<state_vector_type>(r.sv));
                //std::copy(hdr->t_hidden(), hdr->t_end(), std::back_insert_iterator<state_vector_type>(r.sv));
                _history.push_back(r);
                ++_n;
            }
            
            //! Called when the network is cleared.
            virtual void clear() {
                record r(_n, record::CLEAR);
                _history.push_back(r);
            }

            //! Output this trace to the given stream.
            virtual std::ostream& operator<<(std::ostream& out) { 
                for(history_type::iterator i=_history.begin(); i!=_history.end(); ++i) {
                    out << i->t << ": " << to_string(i->sv) << std::endl; //" --> " << to_string(i->t) << std::endl;
                }
                return out;
            }
            
            //! Convert a feature vector to a string.
            std::string to_string(const state_vector_type& sv) {
                return ea::algorithm::vcat(sv.begin(), sv.end(), "");
            }
            
            //! Retrieve the history.
            history_type& get_history() {
                return _history;
            }
            
        protected:
            int _n; //!< update counter
            history_type _history; //!< storage type for historical information       
        };        

    } // hmm
} // fn

#endif
