/* weighted_vector.h
 *
 * This file is part of EALib.
 *
 * Copyright 2014 David B. Knoester, Charles A. Ofria.
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
#ifndef _EA_WEIGHTED_VECTOR_H_
#define _EA_WEIGHTED_VECTOR_H_

#include <vector>

namespace ealib {
    
    /*! A vector whose elements are weighted, and can be selected proportionately
     to their weight.
     */
    template
    < typename T
    , typename W=double
    > weighted_vector : public std::vector<std::pair<T,W> {
    public:
        typedef std::vector<std::pair<T,W> > parent;
        typedef T element_type;
        typedef W weight_type;
        
        
    }
    
    class ProbSchedule
    {
    private:
        const int num_items;
        std::vector<double> weights;
        std::vector<double> tree_weights;
        Random m_rng;
        
        ProbSchedule(const ProbSchedule&); // @not_implemented
        ProbSchedule& operator=(const ProbSchedule&); // @not_implemented
        
        int CalcID(double rand_pos, int cur_id) {
            // If our target is in the current node, return it!
            const double cur_weight = weights[cur_id];
            if (rand_pos < cur_weight) return cur_id;
            
            // Otherwise determine if we need to recurse left or right.
            rand_pos -= cur_weight;
            const int left_id = cur_id*2 + 1;
            const double left_weight = tree_weights[left_id];
            
            return (rand_pos < left_weight) ? CalcID(rand_pos, left_id) : CalcID(rand_pos-left_weight, left_id+1);
        }
        
    public:
        ProbSchedule(int _items, int seed=-1) : num_items(_items), weights(_items+1), tree_weights(_items+1), m_rng(seed) {
            for (int i = 0; i < (int) weights.size(); i++)  weights[i] = tree_weights[i] = 0.0;
        }
        ~ProbSchedule() { ; }
        
        double GetWeight(int id) const { return weights[id]; }
        double GetSubtreeWeight(int id) const { return tree_weights[id]; }
        
        void Adjust(int id, const double in_weight) {
            weights[id] = in_weight;
            
            // Determine the child ids to adjust subtree weight.
            const int left_id = 2*id + 1;
            const int right_id = 2*id + 2;
            
            // Make sure the subtrees looked for haven't fallen off the end of this tree.
            const double st1_weight = (left_id < num_items) ? tree_weights[left_id] : 0.0;
            const double st2_weight = (right_id < num_items) ? tree_weights[right_id] : 0.0;
            tree_weights[id] = in_weight + st1_weight + st2_weight;
            
            // Cascade the change up the tree to the root.
            while (id) {
                id = (id-1) / 2;
                tree_weights[id] = weights[id] + tree_weights[id*2+1] + tree_weights[id*2+2];
            }
        }
        
        int NextID() {
            const double total_weight = tree_weights[0];
            
            // Make sure it's possible to schedule...
            if (total_weight == 0.0) return -1;
            
            // If so, choose a random number to use for the scheduling.
            double rand_pos = m_rng.GetDouble(total_weight);
            return CalcID(rand_pos, 0);
        }
    };
    
} // ea

#endif
