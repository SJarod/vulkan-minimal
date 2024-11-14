set(component src)

set(exe_name vk)

add_executable(${exe_name})

target_sources(${exe_name}
	PRIVATE
	main.cpp
)

target_link_libraries(${exe_name}
	PUBLIC internal
)