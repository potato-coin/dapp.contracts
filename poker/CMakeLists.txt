add_contract(poker poker ${CMAKE_CURRENT_SOURCE_DIR}/src/poker.cpp)
target_include_directories(poker.wasm
   PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/include
   $ENV{SYSTEM_CONTRACTS_PATH}/pc.token/include)

set_target_properties(poker.wasm
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
