include(CMakeForceCompiler)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(gcc_dir
	"C:/Program Files (x86)/GNU Tools ARM Embedded/4.9 2015q2")

# specify the cross compiler
CMAKE_FORCE_C_COMPILER(${gcc_dir}/bin/arm-none-eabi-gcc.exe GNU)
CMAKE_FORCE_CXX_COMPILER(${gcc_dir}/bin/arm-none-eabi-g++.exe GNU)

# per i comandi FIND_XX() 
set(CMAKE_FIND_ROOT_PATH  ${gcc_dir})

# This sets the default behaviour for the FIND_PROGRAM() 
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM ONLY)
# This is the same as above, but for the FIND_LIBRARY() 
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
# This is the same as above and used for both FIND_PATH() and FIND_FILE()
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# Opzioni per stm32f429 ---------------	
set(arm_flag 
	"-mthumb -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 --specs=nosys.specs")


	