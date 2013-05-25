/* task_library.h 
 * 
 * This file is part of EALib.
 * 
 * Copyright 2012 David B. Knoester, Heather J. Goldsby.
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
#ifndef _EA_DIGITAL_EVOLUTION_TASK_LIBRARY_H_
#define _EA_DIGITAL_EVOLUTION_TASK_LIBRARY_H_

#include <boost/shared_ptr.hpp>
#include <string>
#include <vector>
#include <ea/meta_data.h>
#include <ea/digital_evolution/events.h>

namespace ealib {
    
    /*! Abstract base class for all task types.
     
     Tasks depend on the type of resource that they consume/produce.
     */
    template <typename EA>
    struct abstract_task {
        typedef typename EA::environment_type::resource_ptr_type resource_ptr_type; //!< Pointer to resource for this task.
        
        abstract_task() : _limit(0.0), _exclusive(false) { }
        
        //! Returns the name of this task.
        virtual const std::string& name() = 0;
        
        //! Returns true if this task was performed, false otherwise.
        virtual bool check(int in0, int in1, int out0) = 0;

        //! Configure this task to consume resource r.
        virtual abstract_task* consumes(resource_ptr_type r) = 0;
        
        //! Return the resource consumed by this task.
        virtual resource_ptr_type consumed_resource() = 0;
        
        //! Catalyze consumed resources r, adjusting current priority p, returns new priority.
        virtual double catalyze(double r, double p) = 0;
        
        //! Retrieve the meta-data associated with this task.
        virtual meta_data& md() = 0;
        
        //! Returns true if this task is limited.
        virtual bool is_limited() { return _limit > 0.0; }
        
        virtual double limit() { return _limit; }

        virtual abstract_task* limit(double lim) { _limit = lim; return this; }
        
        virtual bool is_exclusive() { return _exclusive; }

        virtual abstract_task* exclusive(bool t) { _exclusive = t; return this; }

        //! Returns true if the reaction associated with this task is allowed to occur, false otherwise.
        bool reaction_occurs(typename EA::individual_type& ind, EA& ea) {
            bool r=true;
            
            // check to see if consumption of the associated resource is limited:
            if(is_limited() && (ind.phenotype()[name()] >= limit())) {
                r = false;
            }
            
            // check to see if this task is exclusive:
            if(r && is_exclusive()) {
                for(typename EA::individual_type::phenotype_map_type::iterator i=ind.phenotype().begin(); i!=ind.phenotype().end(); ++i) {
                    if((i->first != name()) && (i->second > 0.0)) {
                        r = false;
                    }
                }
            }

            return r;
        }
        
        double _limit;
        bool _exclusive;
    };
    

    /*! Task object, which is parameterized on predicate and catalyst types.
     
     When the predicate is true, a "reaction" is said to occur -- The reaction 
     consumes, and possibly produces, resources.
     */
    template <typename Predicate, typename Catalyst, typename EA>
    struct task : abstract_task<EA> {
        typedef typename abstract_task<EA>::resource_ptr_type resource_ptr_type; //!< Resource pointer type.
        typedef Predicate predicate_type; //!< Task predicate type.
        typedef Catalyst catalyst_type; //!< Task catalyst type.
        
        //! Constructor.
        task(const std::string& name) : _name(name) {
        }
        
        //! Returns the name of this task.
        virtual const std::string& name() {
            return _name;
        }
        
        //! Returns true if this task was performed, false otherwise.
        virtual bool check(int in0, int in1, int out0) {
            return _pred(in0, in1, out0);
        }
        
        //! Catalyze consumed resources into priority
        virtual double catalyze(double r, double p) {
            return _cat(r, p);
        }
        
        //! Configure this task to consume resource r.
        abstract_task<EA>* consumes(resource_ptr_type r) {
            _consumed = r;
            return this;
        }
        
        //! Return the resource consumed by this task.
        virtual resource_ptr_type consumed_resource() { 
            return _consumed;
        }
        
        //! Retrieve the meta-data associated with this task.
        meta_data& md() { return _md; }
        
        const std::string _name; //!< Name of this task.
        predicate_type _pred; //!< Predicate that calculates if a task has been performed.
        catalyst_type _cat; //!< Catalyst that converts consumed resources to fitness.
        resource_ptr_type _consumed; //!< Resource consumed when this task is performed.
        meta_data _md; //!< Meta data associated with this task, if any.
    };

    
    /*! Contains the tasks that are active for the current EA.
     */
    template <typename EA>
    class task_library {
    public:
        typedef typename EA::individual_type individual_type;
        typedef abstract_task<EA> abstract_task_type;
        typedef boost::shared_ptr<abstract_task_type> task_ptr_type;
        typedef std::vector<task_ptr_type> tasklist_type;
        
        //! Append a task to the task library.
        void append(task_ptr_type p) {
            _tasklist.push_back(p);
        }

        //! Retrieve the list of active tasks.
        tasklist_type& tasks() { return _tasklist; }

        /*! Updates the priority for the given individual.
         */
        void prioritize(individual_type& org, EA& ea) {
            typename EA::priority_type p=1.0;
            
            for(typename tasklist_type::iterator i=_tasklist.begin(); i!=_tasklist.end(); ++i) {
                abstract_task_type& task=(**i);
                if(org.phenotype()[task.name()] > 0.0) {
                    p = task.catalyze(org.phenotype()[task.name()], p);
                }
            }
            
            org.priority() = p;
            org.phenotype().clear();
        }
        
        /*! Check to see what, if any, tasks the given individual has performed,
         and record their performance in the individual's phenotype.
         
         This works by testing the latest iobuffer entries against all
         tasks in the task library.  For every task performed, the individual's
         phenotype is annotated with the amount of resources consumed.
         */
        void check_tasks(individual_type& org, EA& ea) {
            typedef typename EA::individual_type::io_type io_type;
            typedef typename EA::individual_type::iobuffer_type iobuffer_type;
            
            iobuffer_type& inputs = org.inputs();
            iobuffer_type& outputs = org.outputs();
            
            if((inputs.size() >= 2) && (!outputs.empty())) {
                for(typename tasklist_type::iterator i=_tasklist.begin(); i!=_tasklist.end(); ++i) {
                    abstract_task_type& task=(**i);
                    if(task.check(inputs[0], inputs[1], outputs[0])) {
                        // ok, the *task* was performed.
                        ea.events().task(org, *i, ea);
                        
                        if(task.reaction_occurs(org,ea)) {
                            // if the reaction occurs, consume resources:
                            double r = ea.env().reaction(task.consumed_resource(), org, ea);
                            org.phenotype()[task.name()] += r;
                            ea.events().reaction(org, *i, r, ea);
                        } else {
                            // if the reaction did not occur, let's still update the
                            // phenotype to indicate that the task was performed:
                            org.phenotype()[task.name()] += 0.0;
                        }
                    }
                }
            }
        }
        
    protected:
        tasklist_type _tasklist; //!< Active tasks.
    };
    
    //! Helper method that builds tasks and adds them to the task library.
    template <typename Predicate, typename Catalyst, typename EA>
    typename EA::tasklib_type::task_ptr_type make_task(const std::string& name, EA& ea) {
        typedef typename EA::tasklib_type::task_ptr_type task_ptr_type;
        task_ptr_type p(new task<Predicate,Catalyst,EA>(name));
        ea.tasklib().append(p);
        return p;
    }
        
    
    namespace tasks {

        //! Not: returns true if z == !x or z == !y. 
        struct task_not{
            bool operator()(int in0, int in1, int out0) {
                return ((out0 == ~in0) || (out0 == ~in1));
            }
        };

        //! Nand: returns true if z == x nand y.
        struct task_nand {
            bool operator()(int in0, int in1, int out0) {
                return out0 == ~(in0 & in1);
            }
        };
        
        //! And: returns true if z == (x and y).
        struct task_and {
            bool operator()(int in0, int in1, int out0) {
                return out0 == (in0 & in1);
            }
        };
        
        //! Ornot: returns true if (z == (a or !b)) || (z== (!a or b)).
        struct task_ornot {
            bool operator()(int in0, int in1, int out0) {
                return ((out0 == (in0 | ~in1)) || (out0 == (~in0 | in1)));
            }
        };
        
        //! Or: returns true if z == (a or b).
        struct task_or {
            bool operator()(int in0, int in1, int out0) {
                return (out0 == (in0 | in1));
            }
        };
        
        //! Andnot: returns true if (z == (a and !b)) || (z== (!a and b)).
        struct task_andnot {
            bool operator()(int in0, int in1, int out0) {
                return ((out0 == (in0 & ~in1)) || (out0 == (~in0 & in1)));
            }
        };
            
        //! Nor: returns true if z == !(a or b).
        struct task_nor {
            bool operator()(int in0, int in1, int out0) {
                return (out0 == ~(in0 | in1));
            }
        };
        
        //! Xor: returns true if z == ((a and !b) || (!a and b)).
        struct task_xor {
            bool operator()(int in0, int in1, int out0) {
                return (out0 == ((in0 & ~in1) | (~in0 & in1)));
            }  
        };
        
        //! Equals: returns true if z == ((a and b) || (!a and !b)).
        struct task_equals {
            bool operator()(int in0, int in1, int out0) {
                return (out0 == ((in0 & in1) | (~in0 & ~in1)));
            }  
        };
        
        //! True: always returns true. (Used for testing)
        struct task_true {
            bool operator()(int in0, int in1, int out0) {
                return true;
            }  
        };
        
    } // tasks
    
    namespace catalysts {
        //! "Additive" catalyst type.
        template <int T>
        struct additive {
            double operator()(double r, double p) {
                return p + r * T;
            }
        };
    } // catalysts

} // ea

#endif
