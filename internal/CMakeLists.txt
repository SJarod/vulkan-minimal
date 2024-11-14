set(component internal)

add_library(${component} INTERFACE "")

set_property(TARGET ${component}
    PROPERTY PUBLIC_HEADER
    data.hpp
    utils.hpp
    vertex.hpp
    vulkan_tutorial.hpp
    wsi.hpp
)

target_link_libraries(${component}
    INTERFACE glfw
    INTERFACE glad_vk_13
    INTERFACE mathematics
)

target_include_directories(${component} INTERFACE "${CMAKE_CURRENT_LIST_DIR}")
target_include_directories(${component} INTERFACE "${CMAKE_CURRENT_LIST_DIR}/..")