
if(RT32 OR CMAKE_SIZEOF_VOID_P EQUAL 4)
    add_definitions(-DRT32)
endif()

add_library(common
    Helpers.cpp)
