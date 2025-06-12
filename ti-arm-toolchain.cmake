set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR ARM)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# TI Compiler Pfad
set(TI_COMPILER_DIR "/Volumes/Programme/Programme/ti/ccs1220/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/bin")

# Compiler definieren
set(CMAKE_C_COMPILER   "${TI_COMPILER_DIR}/armcl")
set(CMAKE_CXX_COMPILER "${TI_COMPILER_DIR}/armcl")
set(CMAKE_AR           "${TI_COMPILER_DIR}/armar")

# Linker ist der gleiche wie der Compiler beim TI ARM Compiler
set(CMAKE_LINKER       "${TI_COMPILER_DIR}/armcl")

# Such-Pfade
set(CMAKE_FIND_ROOT_PATH ${TI_COMPILER_DIR}/..)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# TI spezifische Einstellungen
set(CMAKE_C_LINK_EXECUTABLE "<CMAKE_LINKER> <CMAKE_C_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")

# # ti-arm-toolchain.cmake
# set(CMAKE_SYSTEM_NAME Generic)
# set(CMAKE_SYSTEM_PROCESSOR ARM)

# set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# # Dieser Pfad wurde durch Ihre 'find'-Suche bestätigt und ist korrekt.
# set(TI_COMPILER_DIR "/Volumes/Programme/Programme/ti/ccs1220/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/bin")

# # Hier definieren wir die exakten Compiler-Programme.
# set(CMAKE_C_COMPILER   "${TI_COMPILER_DIR}/armcl")
# set(CMAKE_CXX_COMPILER "${TI_COMPILER_DIR}/armcl")
# set(CMAKE_AR           "${TI_COMPILER_DIR}/armar")

# # Sagt CMake, dass es nur in unseren Toolchain-Pfaden nach Programmen suchen soll.
# set(CMAKE_FIND_ROOT_PATH ${TI_COMPILER_DIR})
# set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
# set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)