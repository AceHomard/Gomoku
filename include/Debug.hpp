#ifndef DEBUG_HPP
#define DEBUG_HPP

#include <iostream>

// Define DEBUG_MODE to enable debug logging
// Pass -DDEBUG_MODE to the compiler or use `make debug` to enable
// #define DEBUG_MODE

// Define DEBUG_VISU to enable AI debug visualizer window
// Uncomment for visualization, or compile with: make debug_visu
// #define DEBUG_VISU

#ifdef DEBUG_VISU
    #define VISU_HOOK(x) x
#else
    #define VISU_HOOK(x) ((void)0)
#endif

#ifdef DEBUG_MODE
    #define LOG_DEBUG(msg) std::cout << "[DEBUG] " << msg << std::endl
    #define LOG_INFO(msg) std::cout << "[INFO] " << msg << std::endl
    #define LOG_PERF(msg) std::cout << "[PERF] " << msg << std::endl
#else
    // In release mode, these macros do nothing (zero overhead)
    #define LOG_DEBUG(msg) ((void)0)
    #define LOG_INFO(msg) ((void)0)
    #define LOG_PERF(msg) ((void)0)
#endif

// Critical logs - active in debug mode only (errors, game rules)
#ifdef DEBUG_MODE
    #define LOG_ERROR(msg) std::cerr << "[ERROR] " << msg << std::endl
    #define LOG_RULE(msg) std::cout << "[RULE] " << msg << std::endl
    #define LOG_MANDATORY(msg) std::cout << "[MANDATORY] " << msg << std::endl
#else
    #define LOG_ERROR(msg) ((void)0)
    #define LOG_RULE(msg) ((void)0)
    #define LOG_MANDATORY(msg) ((void)0)
#endif

#endif // DEBUG_HPP