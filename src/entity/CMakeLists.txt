
# set(DIR_LIB_SRCS
#     Computation.cpp
#     IODiskPlugin.cpp
#     Node.cpp
#    NodeManager.cpp
#    TCPLayer.cpp
#    Time.cpp
# )

# set(DIR_LIB_NODE_MANAGER
#     NodeManager.cpp
#     Node.cpp    
# )

aux_source_directory(. DIR_LIB_SRCS)

add_library(Entity ${DIR_LIB_SRCS})
# add_library(NodeManager ${DIR_LIB_NODE_MANAGER})