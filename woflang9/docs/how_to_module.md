WOFLANG_PLUGIN_EXPORT void register_plugin(WoflangInterpreter& interp) {
    static KanjiOpsPlugin plugin;
    plugin.register_ops(interp);
}
CMakeLists.txt (Root File)
cmakecmake_minimum_required(VERSION 3.16)
project(woflang CXX)
set(WOFLANG_VERSION "3.0.0")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Global compile options for all targets
if(MSVC)
    add_compile_options(/W4 /permissive- /EHsc)
else()
    add_compile_options(-Wall -Wextra -pedantic)
endif()

# Core library
file(GLOB CORE_SRC 
    src/core/*.cpp
    src/ops/*.cpp 
    src/glyph/*.cpp 
    src/io/*.cpp 
    src/simd/*.cpp
)

add_library(woflang_core STATIC ${CORE_SRC})
target_include_directories(woflang_core PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ops
    ${CMAKE_CURRENT_SOURCE_DIR}/src/glyph
    ${CMAKE_CURRENT_SOURCE_DIR}/src/io
    ${CMAKE_CURRENT_SOURCE_DIR}/src/simd
)

# REPL executable
add_executable(woflang_repl src/repl/repl_main.cpp)
target_link_libraries(woflang_repl PRIVATE woflang_core)

# Plugin directory for runtime loading
add_custom_target(create_plugin_dir ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/plugins
)

# Add plugins subdirectory
add_subdirectory(plugins)

# Add tests
add_subdirectory(tests)

# Install targets
install(TARGETS woflang_core woflang_repl
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# Install plugin directory structure
install(DIRECTORY ${CMAKE_BINARY_DIR}/plugins/
    DESTINATION plugins
)
CMakeLists.txt (Plugins Directory)
cmake# Plugins CMakeLists.txt

# Find all plugin sources
file(GLOB PLUGIN_SRCS "*.cpp")

# Function to create plugin targets
function(add_woflang_plugin name source)
    add_library(${name} MODULE ${source})
    target_include_directories(${name} PRIVATE ${CMAKE_SOURCE_DIR}/src)
    target_link_libraries(${name} PRIVATE woflang_core)
    
    # Set output directory to main plugins folder
    set_target_properties(${name} PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
    )
    
    # Install plugin
    install(TARGETS ${name} 
        LIBRARY DESTINATION plugins
        RUNTIME DESTINATION plugins
    )
endfunction()

# Add each plugin
add_woflang_plugin(math_constants math_constants.cpp)
add_woflang_plugin(math_greek math_greek.cpp)
add_woflang_plugin(physics_constants load_physics_constants.cpp)
add_woflang_plugin(modelica_constants load_modelica_constants.cpp)
add_woflang_plugin(crypto_ops crypto_ops.cpp)
add_woflang_plugin(quantum_ops quantum_ops.cpp)
add_woflang_plugin(fractal_ops fractal_ops.cpp)
add_woflang_plugin(mathlib_calculus mathlib_calculus.cpp)
add_woflang_plugin(symbolic_simplify ops_symbolic_simplify_rules.cpp)
add_woflang_plugin(symbolic_solve_linear ops_symbolic_solve_linear.cpp)
add_woflang_plugin(kanji_ops kanji_ops.cpp)
add_woflang_plugin(music_ops music_ops.cpp)
add_woflang_plugin(entropy_op entropy_op.cpp)
add_woflang_plugin(prophecy_op prophecy_op.cpp)
add_woflang_plugin(stack_slayer_op stack_slayer_op.cpp)
add_woflang_plugin(void_division_op void_division_op.cpp)
CMakeLists.txt (Tests Directory)
cmake# Tests CMakeLists.txt

# Test runner executable
add_executable(woflang_test_runner woflang_test_runner.cpp)
target_link_libraries(woflang_test_runner PRIVATE woflang_core)

# Copy test scripts to binary directory
file(GLOB TEST_SCRIPTS "*.wof")
foreach(script ${TEST_SCRIPTS})
    get_filename_component(script_name ${script} NAME)
    configure_file(${script} ${CMAKE_BINARY_DIR}/tests/${script_name} COPYONLY)
endforeach()

# Add test to CTest
add_test(NAME WoflangTests COMMAND woflang_test_runner)
Plugin API Standardization
To ensure all plugins follow a consistent pattern, we should create a plugin template that all new plugins can use:
plugin_template.cpp
cpp#include "../../src/core/woflang.hpp"
#include <iostream>

// Plugin class name should describe its purpose
class ExamplePlugin : public WoflangPlugin {
public:
    void register_ops(WoflangInterpreter& interp) override {
        // Register ops with descriptive lambda names
        interp.register_op("example_op", [](WoflangInterpreter& interp) {
            // 1. Check stack size
            if (interp.stack.size() < 1) {
                std::cout << "example_op: Needs at least one value\n";
                return;
            }
            
            // 2. Pop values safely
            auto v = interp.stack.back();
            interp.stack.pop_back();
            
            // 3. Type check
            if (v.type != WofType::Integer) {
                std::cout << "example_op: Expected integer\n";
                interp.stack.push_back(v); // Put it back
                return;
            }
            
            // 4. Perform operation
            int64_t val = std::get<int64_t>(v.value);
            
            // 5. Push result
            interp.stack.emplace_back(val * 2);
            
            // 6. Optional: Print info/feedback
            std::cout << "example_op: Doubled the value\n";
        });
    }
};

WOFLANG_PLUGIN_EXPORT void register_plugin(WoflangInterpreter& interp) {
    static ExamplePlugin plugin;
    plugin.register_ops(interp);
}
