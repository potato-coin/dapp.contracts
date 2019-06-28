add_contract(pocker pocker ${CMAKE_CURRENT_SOURCE_DIR}/src/pocker.cpp)
target_include_directories(pocker.wasm
   PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/include
   $ENV{SYSTEM_CONTRACTS_PATH}/pc.token/include)

set_target_properties(pocker.wasm
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
