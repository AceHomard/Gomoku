/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   UIUtils.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: glamazer <glamazer@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/29 00:00:00 by glamazer          #+#    #+#             */
/*   Updated: 2025/09/29 00:00:00 by glamazer         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "UI/UIUtils.hpp"
#include <algorithm>
#include <cmath>

namespace UIUtils {

sf::Color blendColors(const sf::Color& color1, const sf::Color& color2, float ratio) {
    ratio = std::max(0.0f, std::min(1.0f, ratio));
    
    unsigned char r = static_cast<unsigned char>(color1.r * (1.0f - ratio) + color2.r * ratio);
    unsigned char g = static_cast<unsigned char>(color1.g * (1.0f - ratio) + color2.g * ratio);
    unsigned char b = static_cast<unsigned char>(color1.b * (1.0f - ratio) + color2.b * ratio);
    unsigned char a = static_cast<unsigned char>(color1.a * (1.0f - ratio) + color2.a * ratio);
    
    return sf::Color(r, g, b, a);
}

sf::Color adjustBrightness(const sf::Color& color, float factor) {
    factor = std::max(0.0f, factor);
    
    unsigned char r = static_cast<unsigned char>(std::min(255.0f, color.r * factor));
    unsigned char g = static_cast<unsigned char>(std::min(255.0f, color.g * factor));
    unsigned char b = static_cast<unsigned char>(std::min(255.0f, color.b * factor));
    
    return sf::Color(r, g, b, color.a);
}

sf::Color adjustAlpha(const sf::Color& color, unsigned char alpha) {
    return sf::Color(color.r, color.g, color.b, alpha);
}

sf::Vector2f centerText(const sf::Text& text, const sf::FloatRect& bounds) {
    sf::FloatRect textBounds = text.getLocalBounds();
    
    float x = bounds.position.x + (bounds.size.x - textBounds.size.x) / 2.0f;
    float y = bounds.position.y + (bounds.size.y - textBounds.size.y) / 2.0f;
    
    return sf::Vector2f(x, y);
}

void setTextCentered(sf::Text& text, float x, float y) {
    sf::FloatRect bounds = text.getLocalBounds();
    text.setPosition(sf::Vector2f(x - bounds.size.x / 2.0f, y - bounds.size.y / 2.0f));
}

float easeInOut(float t) {
    return t * t * (3.0f - 2.0f * t);
}

float easeIn(float t) {
    return t * t;
}

float easeOut(float t) {
    return 1.0f - (1.0f - t) * (1.0f - t);
}

float bounce(float t) {
    if (t < 0.5f) {
        return 2.0f * t * t;
    } else {
        return -1.0f + 4.0f * t - 2.0f * t * t;
    }
}

bool isPointInCircle(const sf::Vector2f& point, const sf::Vector2f& center, float radius) {
    float dx = point.x - center.x;
    float dy = point.y - center.y;
    return (dx * dx + dy * dy) <= (radius * radius);
}

bool isPointInRect(const sf::Vector2f& point, const sf::FloatRect& rect) {
    return rect.contains(point);
}

sf::Vector2f rotatePoint(const sf::Vector2f& point, const sf::Vector2f& center, float angle) {
    float cos_a = std::cos(angle);
    float sin_a = std::sin(angle);
    
    float dx = point.x - center.x;
    float dy = point.y - center.y;
    
    float x = center.x + dx * cos_a - dy * sin_a;
    float y = center.y + dx * sin_a + dy * cos_a;
    
    return sf::Vector2f(x, y);
}

} // namespace UIUtils