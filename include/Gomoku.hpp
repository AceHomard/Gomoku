/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Gomoku.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: glamazer <glamazer@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/09 10:00:00 by glamazer          #+#    #+#             */
/*   Updated: 2025/09/09 10:00:00 by glamazer         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef GOMOKU_HPP
#define GOMOKU_HPP

#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <string>

// Forward declarations
class Game;
class Board;
class Player;

// Game constants
const int BOARD_SIZE = 19;
const int CELL_SIZE = 30;
const int WINDOW_WIDTH = BOARD_SIZE * CELL_SIZE + 100;
const int WINDOW_HEIGHT = BOARD_SIZE * CELL_SIZE + 100;

// Color constants
const sf::Color BOARD_COLOR = sf::Color(139, 69, 19);    // Brown
const sf::Color LINE_COLOR = sf::Color::Black;
const sf::Color BLACK_STONE = sf::Color::Black;
const sf::Color WHITE_STONE = sf::Color::White;

#endif // GOMOKU_HPP