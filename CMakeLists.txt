cmake_minimum_required(VERSION 3.6)

project(btstack)


if (CMAKE_CROSSCOMPILING)    
    set(CMAKE_C_FLAGS
	    ${arm_flag})
    set(CMAKE_CXX_FLAGS
	    ${arm_flag})
endif ()

#message(STATUS "linker: " ${CMAKE_LINKER})


# Opzioni comuni a c e c++
set(gcc_flags "-Wall -Wextra -Werror")

# Altre regole utili
set(gcc_flags 
	"${gcc_flags} -fstrict-aliasing -Wshadow")
set(gcc_flags 
	"${gcc_flags} -Wendif-labels -Wpointer-arith -Wcast-align")
	
set(gcc_flags 
	"${gcc_flags} -Wwrite-strings")

#####################################################################
# C++
	# Uso c++11
set(gcc_cpp_flags "-std=c++11")
	# Ulteriore controllo
if (USA_EFFCPP)
	set(gcc_cpp_flags "${gcc_cpp_flags} -Weffc++")
endif ()

#####################################################################
# C
set(gcc_c_flags "-std=c99")

#####################################################################
# Ai compilatori
set(CMAKE_C_FLAGS
	"${CMAKE_C_FLAGS} ${gcc_flags} ${gcc_c_flags}")
set(CMAKE_CXX_FLAGS
	"${CMAKE_CXX_FLAGS} ${gcc_flags} ${gcc_cpp_flags}")	


#####################################################################
add_subdirectory(src)
