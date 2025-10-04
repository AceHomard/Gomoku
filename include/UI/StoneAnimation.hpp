/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   StoneAnimation.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: glamazer <glamazer@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/29 00:00:00 by glamazer          #+#    #+#             */
/*   Updated: 2025/09/29 00:00:00 by glamazer         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef STONEANIMATION_HPP
#define STONEANIMATION_HPP

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include "../Game/Board.hpp"

class StoneAnimation {
public:
    enum Type {
        PLACE,      // Stone placement animation
        CAPTURE,    // Stone capture animation
        HIGHLIGHT,  // Move highlighting
        FADE_IN,    // Fade in effect
        FADE_OUT    // Fade out effect
    };

private:
    Type animationType;
    Position position;
    sf::Clock animationClock;
    float duration;
    bool active;
    sf::CircleShape animatedStone;
    sf::Color originalColor;
    sf::Color targetColor;
    
public:
    StoneAnimation(Type type, const Position& pos, float dur = 0.5f);
    ~StoneAnimation();
    
    void start();
    void update();
    bool isActive() const { return active; }
    bool isComplete() const;
    
    void setColors(sf::Color original, sf::Color target);
    void draw(sf::RenderWindow& window, const sf::Vector2f& boardOffset, float cellSize);
    
    Type getType() const { return animationType; }
    Position getPosition() const { return position; }
};

#endif // STONEANIMATION_HPP