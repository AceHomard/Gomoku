/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Debug.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: glamazer <glamazer@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/30 00:00:00 by glamazer          #+#    #+#             */
/*   Updated: 2025/09/30 00:00:00 by glamazer         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef DEBUG_HPP
#define DEBUG_HPP

#include <iostream>

// Define DEBUG_MODE to enable debug logging
// Uncomment the line below for development/debugging
#define DEBUG_MODE

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

// Critical logs - always active (errors, game rules)
#define LOG_ERROR(msg) std::cerr << "[ERROR] " << msg << std::endl
#define LOG_RULE(msg) std::cout << "[RULE] " << msg << std::endl
#define LOG_MANDATORY(msg) std::cout << "[MANDATORY] " << msg << std::endl

#endif // DEBUG_HPP