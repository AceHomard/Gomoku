/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   UIUtils.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: glamazer <glamazer@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/29 00:00:00 by glamazer          #+#    #+#             */
/*   Updated: 2025/09/29 00:00:00 by glamazer         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UIUTILS_HPP
#define UIUTILS_HPP

#include <SFML/Graphics.hpp>

namespace UIUtils {
    // Color utilities
    sf::Color blendColors(const sf::Color& color1, const sf::Color& color2, float ratio);
    sf::Color adjustBrightness(const sf::Color& color, float factor);
    sf::Color adjustAlpha(const sf::Color& color, unsigned char alpha);
    
    // Text utilities
    sf::Vector2f centerText(const sf::Text& text, const sf::FloatRect& bounds);
    void setTextCentered(sf::Text& text, float x, float y);
    
    // Animation easing functions
    float easeInOut(float t);
    float easeIn(float t);
    float easeOut(float t);
    float bounce(float t);
    
    // Geometric helpers
    bool isPointInCircle(const sf::Vector2f& point, const sf::Vector2f& center, float radius);
    bool isPointInRect(const sf::Vector2f& point, const sf::FloatRect& rect);
    sf::Vector2f rotatePoint(const sf::Vector2f& point, const sf::Vector2f& center, float angle);
}

#endif // UIUTILS_HPP