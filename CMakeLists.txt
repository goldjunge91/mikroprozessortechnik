# Mindestanforderung an die CMake-Version
cmake_minimum_required(VERSION 3.16)

# Sicherstellen, dass die Toolchain-Datei verwendet wird.
if(NOT CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/ti-arm-toolchain.cmake" CACHE STRING "Toolchain File")
endif()

# Projektname definieren
project(mikroprozessortechnik C)

# --- Pfade definieren ---
set(TIVAWARE_PATH "/Volumes/Programme/Programme/ti/TivaWare_C_Series-2.2.0.295")
set(TI_COMPILER_PATH "/Volumes/Programme/Programme/ti/ccs1220/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS")
set(LINKER_CMD_FILE "${CMAKE_CURRENT_SOURCE_DIR}/tm4c1294ncpdt.cmd")

# --- Alle Quelldateien sammeln ---
file(GLOB_RECURSE TASK_SOURCES "src/*.c")
add_executable(${PROJECT_NAME}
    "main.c"
    "tm4c1294ncpdt_startup_ccs.c"
    ${TASK_SOURCES}
)

# ===================================================================
# --- Compiler-Einstellungen ---
# ===================================================================
target_compile_options(${PROJECT_NAME} PRIVATE
    -mv7M4
    --code_state=16
    --float_support=FPv4SPD16
    -me
    -g
    --diag_warning=225
    --diag_wrap=off
    --display_error_number
    --abi=eabi
)
target_include_directories(${PROJECT_NAME} PUBLIC
    "src"
    ${TIVAWARE_PATH}
    "${TI_COMPILER_PATH}/include"
)
target_compile_definitions(${PROJECT_NAME} PRIVATE
    ccs="ccs"
    PART_TM4C1294NCPDT
)

# ===================================================================
# --- Linker-Einstellungen (NEUER, ROBUSTER ANSATZ) ---
# ===================================================================

# KORREKTUR: Wir setzen die Linker-Flags direkt in die globale Variable.
# Das ist die zuverlässigste Methode für die TI-Toolchain.
set(CMAKE_EXE_LINKER_FLAGS
    "-m${PROJECT_NAME}.map --rom_model --warn_sections --stack_size=4096 --heap_size=0 --reread_libs"
    CACHE STRING "Linker Flags" FORCE
)

# Wir übergeben die Bibliotheken und die .cmd-Datei weiterhin hier.
# Die Reihenfolge ist wichtig!
target_link_libraries(${PROJECT_NAME} PRIVATE
    "${LINKER_CMD_FILE}"
    "${TIVAWARE_PATH}/driverlib/ccs/Debug/driverlib.lib"
    "${TI_COMPILER_PATH}/lib/libc.a"
)

# ===================================================================
# --- Post-Build: Erstellen der .bin-Datei ---
# ===================================================================
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary ${PROJECT_NAME} ${PROJECT_NAME}.bin
    COMMENT "Creating ${PROJECT_NAME}.bin"
)
# Der Einfachheit halber verwenden wir CMAKE_OBJCOPY, falls es von der Toolchain-Datei korrekt gesetzt wird.
# Falls nicht, ersetzen Sie die obige Zeile durch die folgende:
# add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#     COMMAND ${TI_COMPILER_PATH}/bin/armhex -o ${PROJECT_NAME}.bin "$<TARGET_FILE:${PROJECT_NAME}>"
#     COMMENT "Creating ${PROJECT_NAME}.bin"
# )

