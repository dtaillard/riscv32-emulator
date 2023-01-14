add_subdirectory(rv32ima)

target_sources(rv32-emulator PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/basic_memory.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/emulator_exception.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/mem_map_manager.cpp"
)

target_include_directories(rv32-emulator PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")