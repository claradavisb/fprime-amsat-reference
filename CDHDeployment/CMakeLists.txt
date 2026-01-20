#####
# 'CDHDeployment' Deployment:
#####

###
# Topology and Components
###
add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/Top/")
add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/RadioBridge/")  # Remove for now
add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/AMSATFramer/")

register_fprime_deployment(
    SOURCES
        "${CMAKE_CURRENT_LIST_DIR}/Main.cpp"
    DEPENDS
        ${FPRIME_CURRENT_MODULE}_Top
)
add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/AMSATDeframer/")
