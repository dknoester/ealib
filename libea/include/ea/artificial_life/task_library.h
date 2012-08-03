/* artificial_life/artificial_life.h 
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

#ifndef _EA_ARTIFICIAL_LIFE_TASK_LIBRARY_H_
#define _EA_ARTIFICIAL_LIFE_TASK_LIBRARY_H_

#include <boost/shared_ptr.hpp>
#include <string>
#include <vector>

namespace ea {
    namespace tasks {
        
        //! Abstract base class for all task types.
        struct abstract_task {
            //! Returns the name of this task.
            virtual const std::string& name() = 0;
            //! Returns true if this task was performed, false otherwise.
            virtual bool check(int in0, int in1, int out0) = 0;
            //! Consumes resources associated with this task, returns amount consumed.
            virtual double consume() = 0;
            //! Catalyze consumed resources r, adjusting current priority p, returns new priority.
            virtual double catalyze(double r, double p) = 0;
        };
        
        //! Task object, which is parameterized on predicate, resource, and catalyst types.
        template <typename Predicate, typename Resource, typename Catalyst>
        struct task : abstract_task {
            typedef Predicate predicate_type; //!< Task predicate type.
            typedef Resource resource_type; //!< Task resource type.
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
            
            //! Consumes resources associated with this task, returns amount consumed.
            virtual double consume() {
                return _res();
            }
            
            //! Catalyze consumed resources into priority
            virtual double catalyze(double r, double p) {
                return _cat(r, p);
            }
                        
            const std::string _name; //!< Name of this task.
            predicate_type _pred; //!< Predicate that calculates if a task has been performed.
            resource_type _res; //!< Resource that is consumed when a task has been performed.
            catalyst_type _cat; //!< Catalyst that converts consumed resources to fitness.
        };

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

    namespace resources {
        //! Unlimited resource type.
        struct unlimited {
            double operator()() {
                return 1.0;
            }
        };
        struct limited {
            limited(double c) : _cap(c) { }
            double operator()() {
                double consumed;
                _cap -= consumed;
                return consumed;
            }
            double _cap;
        };

    } // resources
    
    namespace catalysts {
        //! "Additive" catalyst type.
        template <int T>
        struct additive {
            double operator()(double r, double p) {
                return p + T;
            }
        };
    } // catalysts
    
    
    /*! Contains the tasks (and their associated resource & catalyst types) that are
     active for the current EA.
     */
    class task_library {
    public:
        typedef boost::shared_ptr<tasks::abstract_task> task_ptr_type;
        typedef std::vector<task_ptr_type> tasklist_type;

        //! Append a task to the task library.
        void append(task_ptr_type p) {
            _tasklist.push_back(p);
        }
        
        /*! Updates the priority for the given individual.
         */
        template <typename Individual, typename EA>
        void prioritize(Individual& org, EA& ea) {
            typename EA::priority_type p=1.0;
            
            for(tasklist_type::iterator i=_tasklist.begin(); i!=_tasklist.end(); ++i) {
                tasks::abstract_task& task=(**i);
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
        template <typename EA>
        void check_tasks(typename EA::individual_type& org, EA& ea) {
            typedef typename EA::individual_type::io_type io_type;
            typedef typename EA::individual_type::iobuffer_type iobuffer_type;
            
            iobuffer_type& inputs = org.inputs();
            iobuffer_type& outputs = org.outputs();
            
            if((inputs.size() >= 2) && (!outputs.empty())) {
                for(tasklist_type::iterator i=_tasklist.begin(); i!=_tasklist.end(); ++i) {
                    tasks::abstract_task& task=(**i);
                    if(task.check(inputs[0], inputs[1], outputs[0])) {
                        double r = task.consume();
                        org.phenotype()[task.name()] += r;
                        ea.events().task_performed(org, r, task.name(), ea);
                    }
                }
            }
        }
        
    protected:
        tasklist_type _tasklist; //!< Active tasks.
    };

    //! Helper method to add tasks to the task library.
    template <typename Predicate, typename Resource, typename Catalyst, typename EA>
    void add_task(const std::string& name, EA& ea) {
        boost::shared_ptr<tasks::abstract_task> p(new tasks::task<Predicate,Resource,Catalyst>(name));
        ea.tasklib().append(p);
    }

} // ea

#endif
