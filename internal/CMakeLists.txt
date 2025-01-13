set(component internal)

add_library(${component} INTERFACE "")

set_property(TARGET ${component}
    PROPERTY PUBLIC_HEADER
    utils.hpp
    vertex.hpp
    vulkan_minimal.hpp
    wsi.hpp
)

target_link_libraries(${component}
    INTERFACE glfw
    INTERFACE volk
    INTERFACE glm
)

target_include_directories(${component} INTERFACE "${CMAKE_CURRENT_LIST_DIR}")
target_include_directories(${component} INTERFACE "${CMAKE_CURRENT_LIST_DIR}/..")