set(component vk)

add_executable(${component})

target_sources(${component}
	PRIVATE
	main.cpp
)

target_link_libraries(${component}
	PUBLIC internal
	PUBLIC glslc # ensure glslc is built before vk
	PUBLIC stb
)

set(SHADER_SOURCES
	shaders/triangle.vert
	shaders/triangle.frag
)

get_target_property(glslc_BINARY_DIR glslc_exe BINARY_DIR)
set(glslc_dir ${glslc_BINARY_DIR}/${CMAKE_BUILD_TYPE})

add_custom_command(TARGET ${component}
	POST_BUILD
	COMMAND echo ${component}.exe built, looking for glslc.exe in ${glslc_dir}
)

list(LENGTH SHADER_SOURCES SHADER_COUNT)
set(SHADER_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/shaders")
add_custom_command(TARGET ${component}
	POST_BUILD
	COMMAND echo Compiling shader sources in ${SHADER_OUTPUT_DIR}...
	COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADER_OUTPUT_DIR}
)
set(SHADER_I 1)
foreach(SOURCE ${SHADER_SOURCES})
	set(SHADER_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${SOURCE}.spv")
	add_custom_command(TARGET ${component}
		POST_BUILD
		COMMAND echo [${SHADER_I}/${SHADER_COUNT}] ${SOURCE}...
		COMMAND ${glslc_dir}/glslc.exe ${CMAKE_SOURCE_DIR}/${SOURCE} -o ${SHADER_OUTPUT}
		COMMAND echo ${SOURCE} : ${SHADER_OUTPUT}
	)
	MATH(EXPR SHADER_I "${SHADER_I} + 1")
endforeach()