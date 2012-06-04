#ifndef _EA_ANALYSIS_TOOL_H_
#define _EA_ANALYSIS_TOOL_H_

namespace ea {
    namespace analysis {
        template <typename EA>
        struct unary_function {
            virtual void operator()(EA& ea) = 0;
        };
    }
}

#endif
