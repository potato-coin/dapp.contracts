add_contract(dice dice ${CMAKE_CURRENT_SOURCE_DIR}/src/dice.cpp)
target_include_directories(dice.wasm
   PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/include
   $ENV{SYSTEM_CONTRACTS_PATH}/pc.token/include)

set_target_properties(dice.wasm
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
