#ifndef _EA_ENVIRONMENT_H_
#define _EA_ENVIRONMENT_H_

#include <boost/serialization/nvp.hpp>


namespace ea {
    
    /*! Abstract environment type.
     */
    template <typename Topology, typename ReplacementStrategy, typename Scheduler>
    class environment {
    public:
        typedef Topology topology_type; //!< Type for this environment's topology.
        
        //! Initialize this environment.
        template <typename AL>
        void initialize(AL& al) {
        };
        
        //! Read from the environment.
        template <typename Organism>
        int read(Organism& org) {
            return 0;
        }

    protected:
        topology_type _topo; //!< This environment's topology.
        
    private:
        friend class boost::serialization::access;
        template <class Archive>
        void serialize(Archive& ar, const unsigned int version) {
            ar & boost::serialization::make_nvp("topology", _topo);
        }
    };

} // ea

#endif
