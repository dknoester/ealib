#ifndef _FN_HMM_PRIMITIVE_H_
#define _FN_HMM_PRIMITIVE_H_

// To provide for common code across host and device, we'll play a few games
// with macros...
#ifdef __CUDACC__
#define __GPU_DECL__ __device__ inline
#else
#define __GPU_DECL__ inline
#endif

#endif
