/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   StatsPanel.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: glamazer <glamazer@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/30 00:00:00 by glamazer          #+#    #+#             */
/*   Updated: 2025/09/30 00:00:00 by glamazer         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef STATSPANEL_HPP
#define STATSPANEL_HPP

#ifdef DEBUG_VISU

#include <SFML/Graphics.hpp>
#include "SearchDataCollector.hpp"

class StatsPanel {
public:
    StatsPanel();

    void render(sf::RenderWindow& window, sf::Font& font,
                float x, float y, float width, float height,
                const SearchStats& stats, bool searching);

    float renderNodeDetails(sf::RenderWindow& window, sf::Font& font,
                            float x, float y, float width,
                            const DebugTreeNode* node);

    void renderLegend(sf::RenderWindow& window, sf::Font& font,
                      float x, float y, float width);

private:
    sf::Clock animClock;

    void drawLabel(sf::RenderWindow& window, sf::Font& font,
                   const std::string& label, const std::string& value,
                   float x, float y, unsigned int size, sf::Color valueColor);

    float drawWrappedText(sf::RenderWindow& window, sf::Font& font,
                          const std::string& text,
                          float x, float y, float maxWidth,
                          unsigned int fontSize, sf::Color color,
                          bool dryRun = false);

    std::string getNodeExplanation(const DebugTreeNode* node);
    std::string getScoreExplanation(int score);
};

#endif // DEBUG_VISU
#endif // STATSPANEL_HPP
