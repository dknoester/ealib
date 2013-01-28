#ifndef _NN_TRAITS_H_
#define _NN_TRAITS_H_

namespace nn {
    /*! Default traits for neural networks; must be specialized by the activation type.
     */
    template <typename ActivationType>
    struct neural_network_traits {
    };
    
} // nn


#endif
