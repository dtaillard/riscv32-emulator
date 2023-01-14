target_sources(rv32-emulator PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/decoder.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/hart.cpp"
)

target_include_directories(rv32-emulator PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
