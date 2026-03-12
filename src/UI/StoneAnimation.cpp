#include "UI/StoneAnimation.hpp"
#include "UI/UIUtils.hpp"
#include <cmath>

StoneAnimation::StoneAnimation(Type type, const Position& pos, float dur)
    : animationType(type)
    , position(pos)
    , duration(dur)
    , active(false)
    , originalColor(sf::Color::White)
    , targetColor(sf::Color::Black)
{
    animatedStone.setRadius(10.0f);
    animatedStone.setOrigin(sf::Vector2f(10.0f, 10.0f));
}

StoneAnimation::~StoneAnimation() {
}

void StoneAnimation::start() {
    active = true;
    animationClock.restart();
}

void StoneAnimation::update() {
    if (!active) return;
    
    float elapsed = animationClock.getElapsedTime().asSeconds();
    if (elapsed >= duration) {
        active = false;
        return;
    }
    
    float progress = elapsed / duration;
    
    switch (animationType) {
        case PLACE: {
            // Scale animation for stone placement
            float scale = UIUtils::easeOut(progress);
            animatedStone.setScale(sf::Vector2f(scale, scale));
            
            // Color transition
            sf::Color currentColor = UIUtils::blendColors(originalColor, targetColor, progress);
            animatedStone.setFillColor(currentColor);
            break;
        }
        
        case CAPTURE: {
            // Shrink and fade animation for captures
            float scale = 1.0f - UIUtils::easeIn(progress);
            animatedStone.setScale(sf::Vector2f(scale, scale));
            
            sf::Color fadeColor = targetColor;
            fadeColor.a = static_cast<unsigned char>(255 * (1.0f - progress));
            animatedStone.setFillColor(fadeColor);
            break;
        }
        
        case HIGHLIGHT: {
            // Pulsing highlight effect
            float alpha = 0.5f + 0.5f * std::sin(progress * 6.28f * 2.0f); // 2 cycles
            sf::Color highlightColor = originalColor;
            highlightColor.a = static_cast<unsigned char>(255 * alpha);
            animatedStone.setFillColor(highlightColor);
            break;
        }
        
        case FADE_IN: {
            sf::Color fadeColor = targetColor;
            fadeColor.a = static_cast<unsigned char>(255 * UIUtils::easeOut(progress));
            animatedStone.setFillColor(fadeColor);
            break;
        }
        
        case FADE_OUT: {
            sf::Color fadeColor = originalColor;
            fadeColor.a = static_cast<unsigned char>(255 * (1.0f - UIUtils::easeIn(progress)));
            animatedStone.setFillColor(fadeColor);
            break;
        }
    }
}

bool StoneAnimation::isComplete() const {
    return !active;
}

void StoneAnimation::setColors(sf::Color original, sf::Color target) {
    originalColor = original;
    targetColor = target;
}

void StoneAnimation::draw(sf::RenderWindow& window, const sf::Vector2f& boardOffset, float cellSize) {
    if (!active) return;
    
    sf::Vector2f screenPos(boardOffset.x + position.x * cellSize,
                          boardOffset.y + position.y * cellSize);
    
    animatedStone.setRadius(cellSize * 0.4f);
    animatedStone.setOrigin(sf::Vector2f(cellSize * 0.4f, cellSize * 0.4f));
    animatedStone.setPosition(screenPos);
    
    window.draw(animatedStone);
}