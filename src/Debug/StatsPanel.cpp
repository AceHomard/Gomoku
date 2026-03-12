#ifdef DEBUG_VISU

#include "Debug/StatsPanel.hpp"
#include <sstream>
#include <iomanip>
#include <cmath>

StatsPanel::StatsPanel() {
}

void StatsPanel::drawLabel(sf::RenderWindow& window, sf::Font& font,
                            const std::string& label, const std::string& value,
                            float x, float y, unsigned int size, sf::Color valueColor) {
    sf::Text labelText(font, label, size);
    labelText.setFillColor(sf::Color(160, 160, 180));
    labelText.setPosition(sf::Vector2f(x, y));
    window.draw(labelText);

    sf::Text valueText(font, value, size);
    valueText.setFillColor(valueColor);
    valueText.setStyle(sf::Text::Bold);
    valueText.setPosition(sf::Vector2f(x + 120.0f, y));
    window.draw(valueText);
}

void StatsPanel::render(sf::RenderWindow& window, sf::Font& font,
                         float x, float y, float width, float height,
                         const SearchStats& stats, bool searching) {
    // Background
    sf::RectangleShape bg(sf::Vector2f(width, height));
    bg.setPosition(sf::Vector2f(x - 5.0f, y));
    bg.setFillColor(sf::Color(40, 40, 55, 230));
    bg.setOutlineColor(sf::Color(70, 70, 90));
    bg.setOutlineThickness(1.0f);
    window.draw(bg);

    // Title
    sf::Text title(font, "STATISTIQUES RECHERCHE", 13);
    title.setFillColor(sf::Color(200, 200, 220));
    title.setStyle(sf::Text::Bold);
    title.setPosition(sf::Vector2f(x + 5.0f, y + 8.0f));
    window.draw(title);

    // Status indicator
    if (searching) {
        float elapsed = animClock.getElapsedTime().asSeconds();
        int dots = static_cast<int>(elapsed * 3) % 4;
        std::string searchText = "En cours" + std::string(dots, '.');
        sf::Text searchLabel(font, searchText, 11);
        searchLabel.setFillColor(sf::Color(100, 255, 100));
        searchLabel.setPosition(sf::Vector2f(x + 5.0f, y + 25.0f));
        window.draw(searchLabel);
    } else {
        sf::Text doneLabel(font, "Termine", 11);
        doneLabel.setFillColor(sf::Color(150, 150, 170));
        doneLabel.setPosition(sf::Vector2f(x + 5.0f, y + 25.0f));
        window.draw(doneLabel);
    }

    float lineY = y + 45.0f;
    float lineH = 22.0f;

    // Best move
    std::ostringstream bestStr;
    if (stats.bestMove.x >= 0) {
        bestStr << "(" << stats.bestMove.x << "," << stats.bestMove.y << ") = ";
        if (stats.bestScore >= 0) bestStr << "+";
        bestStr << stats.bestScore;
    } else {
        bestStr << "--";
    }
    sf::Color bestColor = stats.bestScore > 0 ? sf::Color(100, 255, 100) :
                          stats.bestScore < 0 ? sf::Color(255, 100, 100) :
                          sf::Color::White;
    drawLabel(window, font, "Meilleur coup:", bestStr.str(), x + 5.0f, lineY, 11, bestColor);
    lineY += lineH;

    // Nodes evaluated — educational label
    std::ostringstream nodesStr;
    nodesStr << stats.nodesEvaluated;
    drawLabel(window, font, "Positions:", nodesStr.str(), x + 5.0f, lineY, 11, sf::Color::White);
    lineY += lineH;

    // Cutoffs
    std::ostringstream cutsStr;
    cutsStr << stats.cutoffsCount;
    drawLabel(window, font, "Coupures:", cutsStr.str(), x + 5.0f, lineY, 11, sf::Color(255, 200, 100));
    lineY += lineH;

    // Time
    std::ostringstream timeStr;
    timeStr << std::fixed << std::setprecision(0) << stats.searchTimeMs << "ms";
    sf::Color timeColor = stats.searchTimeMs > 500 ? sf::Color(255, 100, 100) :
                          stats.searchTimeMs > 200 ? sf::Color(255, 200, 100) :
                          sf::Color(100, 255, 100);
    drawLabel(window, font, "Temps:", timeStr.str(), x + 5.0f, lineY, 11, timeColor);
    lineY += lineH;

    // Pruning efficiency — educational
    if (stats.nodesEvaluated > 0) {
        float efficiency = static_cast<float>(stats.cutoffsCount) /
                          static_cast<float>(stats.nodesEvaluated) * 100.0f;
        std::ostringstream effStr;
        effStr << std::fixed << std::setprecision(1) << efficiency << "% economie";
        drawLabel(window, font, "Elagage:", effStr.str(), x + 5.0f, lineY, 11, sf::Color(180, 180, 255));
    }

    (void)height;
}

std::string StatsPanel::getScoreExplanation(int score) {
    if (score > 100000)
        return "Victoire detectee pour l'IA.";
    if (score < -100000)
        return "Victoire detectee pour l'adversaire.";
    if (score > 500)
        return "Position favorable pour l'IA.";
    if (score < -500)
        return "Position favorable pour l'adversaire.";
    return "Position equilibree.";
}

std::string StatsPanel::getNodeExplanation(const DebugTreeNode* node) {
    if (!node) return "";

    std::ostringstream oss;

    // Node type explanation
    if (node->move.x < 0) {
        oss << "Racine: l'IA evalue tous les coups possibles depuis cette position.";
    } else if (node->isBestMove) {
        if (node->isMaximizing) {
            oss << "Meilleur coup de l'IA: parmi tous les coups testes, "
                << "(" << node->move.x << "," << node->move.y << ") "
                << "donne le meilleur score (" << node->score << ").";
        } else {
            oss << "Reponse de l'adversaire: le coup "
                << "(" << node->move.x << "," << node->move.y << ") "
                << "est le plus defavorable pour l'IA (score " << node->score << ").";
        }
    } else if (node->isPruned) {
        if (node->isMaximizing) {
            oss << "Coupe beta: ce noeud retourne " << node->score
                << " qui depasse beta=" << node->beta
                << ". L'adversaire (MIN au-dessus) ne choisira "
                << "jamais cette branche car il a deja mieux. "
                << "Freres restants ignores.";
        } else {
            oss << "Coupe alpha: ce noeud retourne " << node->score
                << " alors que l'IA a deja garanti alpha=" << node->alpha
                << " via un autre chemin. Comme " << node->score
                << " < " << node->alpha
                << ", l'IA ignorera cette branche. "
                << "Freres restants ignores.";
        }
    } else if (node->children.empty()) {
        oss << "Feuille (profondeur max): le plateau est evalue "
            << "par l'heuristique (patterns, captures, menaces). "
            << "Score = " << node->score << ".";
    } else {
        // Internal node, not pruned, not best move
        if (node->isMaximizing) {
            oss << "Tour IA: l'IA joue en (" << node->move.x << "," << node->move.y
                << ") et obtient " << node->score
                << " apres exploration du sous-arbre.";
        } else {
            oss << "Tour adversaire: l'adversaire joue en (" << node->move.x << "," << node->move.y
                << ") et le sous-arbre retourne " << node->score << ".";
        }
    }

    // Score interpretation
    oss << "\n" << getScoreExplanation(node->score);

    return oss.str();
}

float StatsPanel::drawWrappedText(sf::RenderWindow& window, sf::Font& font,
                                   const std::string& text,
                                   float x, float y, float maxWidth,
                                   unsigned int fontSize, sf::Color color,
                                   bool dryRun) {
    float lineHeight = static_cast<float>(fontSize) + 4.0f;
    float currentY = y;
    float margin = 10.0f;
    float usableWidth = maxWidth - margin;

    // Split text into lines on \n first
    std::vector<std::string> paragraphs;
    std::istringstream stream(text);
    std::string paragraph;
    while (std::getline(stream, paragraph)) {
        paragraphs.push_back(paragraph);
    }

    for (auto& para : paragraphs) {
        // Word-wrap each paragraph
        std::istringstream words(para);
        std::string word;
        std::string currentLine;

        while (words >> word) {
            std::string testLine = currentLine.empty() ? word : currentLine + " " + word;
            sf::Text measure(font, testLine, fontSize);
            float textWidth = measure.getLocalBounds().size.x;

            if (textWidth > usableWidth && !currentLine.empty()) {
                // Draw current line and start new one
                if (!dryRun) {
                    sf::Text lineText(font, currentLine, fontSize);
                    lineText.setFillColor(color);
                    lineText.setPosition(sf::Vector2f(x, currentY));
                    window.draw(lineText);
                }
                currentY += lineHeight;
                currentLine = word;
            } else {
                currentLine = testLine;
            }
        }

        // Draw remaining text in paragraph
        if (!currentLine.empty()) {
            if (!dryRun) {
                sf::Text lineText(font, currentLine, fontSize);
                lineText.setFillColor(color);
                lineText.setPosition(sf::Vector2f(x, currentY));
                window.draw(lineText);
            }
            currentY += lineHeight;
        }
    }

    return currentY - y;
}

float StatsPanel::renderNodeDetails(sf::RenderWindow& window, sf::Font& font,
                                    float x, float y, float width,
                                    const DebugTreeNode* node) {
    if (!node) return 0.0f;

    // Total height: header(30) + 7 fields(7*22=154) + padding(10)
    float totalHeight = 30.0f + 7 * 22.0f + 10.0f;

    // Background
    sf::RectangleShape bg(sf::Vector2f(width, totalHeight));
    bg.setPosition(sf::Vector2f(x - 5.0f, y));
    bg.setFillColor(sf::Color(40, 40, 55, 230));
    bg.setOutlineColor(sf::Color(70, 70, 90));
    bg.setOutlineThickness(1.0f);
    window.draw(bg);

    // Title
    sf::Text title(font, "DETAIL DU NOEUD", 13);
    title.setFillColor(sf::Color(200, 200, 220));
    title.setStyle(sf::Text::Bold);
    title.setPosition(sf::Vector2f(x + 5.0f, y + 8.0f));
    window.draw(title);

    float lineY = y + 30.0f;
    float lineH = 22.0f;

    // Move
    std::ostringstream moveStr;
    if (node->move.x >= 0) {
        moveStr << "(" << node->move.x << "," << node->move.y << ")";
    } else {
        moveStr << "RACINE";
    }
    drawLabel(window, font, "Position:", moveStr.str(), x + 5.0f, lineY, 11, sf::Color::White);
    lineY += lineH;

    // Type
    std::string typeExpl = node->isMaximizing ? "MAX (tour IA)" : "MIN (tour adversaire)";
    drawLabel(window, font, "Type:", typeExpl,
              x + 5.0f, lineY, 11,
              node->isMaximizing ? sf::Color(100, 200, 255) : sf::Color(255, 150, 100));
    lineY += lineH;

    // Score
    std::ostringstream scoreStr;
    if (node->score >= 0) scoreStr << "+";
    scoreStr << node->score;
    sf::Color scoreColor = node->score > 0 ? sf::Color(100, 255, 100) :
                           node->score < 0 ? sf::Color(255, 100, 100) :
                           sf::Color::White;
    drawLabel(window, font, "Score:", scoreStr.str(), x + 5.0f, lineY, 11, scoreColor);
    lineY += lineH;

    // Alpha/Beta
    std::ostringstream abStr;
    abStr << "a=" << node->alpha << " b=" << node->beta;
    drawLabel(window, font, "Fenetre:", abStr.str(), x + 5.0f, lineY, 11, sf::Color(180, 180, 200));
    lineY += lineH;

    // Depth
    std::ostringstream depthStr;
    depthStr << node->depth << " niveaux restants";
    drawLabel(window, font, "Profondeur:", depthStr.str(), x + 5.0f, lineY, 11, sf::Color::White);
    lineY += lineH;

    // Status
    std::string status;
    sf::Color statusColor;
    if (node->isBestMove) {
        status = "Meilleur coup";
        statusColor = sf::Color(255, 215, 0);
    } else if (node->isPruned) {
        status = "Elague (coupe)";
        statusColor = sf::Color(255, 100, 100);
    } else {
        status = "Evalue";
        statusColor = sf::Color(150, 150, 170);
    }
    drawLabel(window, font, "Statut:", status, x + 5.0f, lineY, 11, statusColor);
    lineY += lineH;

    // Children count
    std::ostringstream childStr;
    childStr << node->children.size() << " sous-branches";
    drawLabel(window, font, "Branches:", childStr.str(), x + 5.0f, lineY, 11, sf::Color(180, 180, 200));
    lineY += lineH;

    return totalHeight;
}

void StatsPanel::renderLegend(sf::RenderWindow& window, sf::Font& font,
                               float x, float y, float width) {
    float totalH = 240.0f;

    sf::RectangleShape bg(sf::Vector2f(width, totalH));
    bg.setPosition(sf::Vector2f(x - 5.0f, y));
    bg.setFillColor(sf::Color(40, 40, 55, 230));
    bg.setOutlineColor(sf::Color(70, 70, 90));
    bg.setOutlineThickness(1.0f);
    window.draw(bg);

    sf::Text title(font, "LEGENDE", 13);
    title.setFillColor(sf::Color(200, 200, 220));
    title.setStyle(sf::Text::Bold);
    title.setPosition(sf::Vector2f(x + 5.0f, y + 5.0f));
    window.draw(title);

    float lineY = y + 25.0f;
    float lineH = 17.0f;
    float boxSize = 12.0f;

    // Color legend entries
    struct LegendEntry {
        sf::Color color;
        std::string text;
    };

    LegendEntry entries[] = {
        {sf::Color(46, 125, 50),  "Tres favorable IA (+5000)"},
        {sf::Color(56, 142, 60),  "Favorable IA (score > 0)"},
        {sf::Color(66, 66, 80),   "Neutre (score = 0)"},
        {sf::Color(211, 47, 47),  "Favorable adversaire (< 0)"},
        {sf::Color(183, 28, 28),  "Tres favorable adv. (-5000)"},
        {sf::Color(80, 80, 90),   "Branche elaguee (pruning)"},
    };

    for (auto& entry : entries) {
        sf::RectangleShape box(sf::Vector2f(boxSize, boxSize));
        box.setPosition(sf::Vector2f(x + 5.0f, lineY + 1.0f));
        box.setFillColor(entry.color);
        box.setOutlineColor(sf::Color(100, 100, 120));
        box.setOutlineThickness(1.0f);
        window.draw(box);

        sf::Text label(font, entry.text, 9);
        label.setFillColor(sf::Color(180, 180, 200));
        label.setPosition(sf::Vector2f(x + 22.0f, lineY));
        window.draw(label);

        lineY += lineH;
    }

    // Best path indicator
    sf::RectangleShape goldBox(sf::Vector2f(boxSize, boxSize));
    goldBox.setPosition(sf::Vector2f(x + 5.0f, lineY + 1.0f));
    goldBox.setFillColor(sf::Color(66, 66, 80));
    goldBox.setOutlineColor(sf::Color(255, 215, 0));
    goldBox.setOutlineThickness(2.0f);
    window.draw(goldBox);

    sf::Text pathLabel(font, "Chemin de decision IA", 9);
    pathLabel.setFillColor(sf::Color(255, 215, 0));
    pathLabel.setPosition(sf::Vector2f(x + 22.0f, lineY));
    window.draw(pathLabel);
    lineY += lineH + 5.0f;

    // MAX/MIN explanation
    sf::Text maxText(font, "MAX = Tour IA (maximise)", 9);
    maxText.setFillColor(sf::Color(100, 200, 255));
    maxText.setPosition(sf::Vector2f(x + 5.0f, lineY));
    window.draw(maxText);
    lineY += lineH;

    sf::Text minText(font, "MIN = Tour adversaire (minimise)", 9);
    minText.setFillColor(sf::Color(255, 150, 100));
    minText.setPosition(sf::Vector2f(x + 5.0f, lineY));
    window.draw(minText);
    lineY += lineH + 3.0f;

    // Keyboard shortcuts
    sf::Text shortcutTitle(font, "--- Raccourcis ---", 9);
    shortcutTitle.setFillColor(sf::Color(140, 140, 160));
    shortcutTitle.setPosition(sf::Vector2f(x + 5.0f, lineY));
    window.draw(shortcutTitle);
    lineY += lineH;

    sf::Text shortcuts(font, "F=Plein ecran  Molette=Zoom\nClic=Pan  DblClic=Ouvrir/Fermer\nR=Reset vue  Esc=Fermer", 9);
    shortcuts.setFillColor(sf::Color(120, 120, 140));
    shortcuts.setPosition(sf::Vector2f(x + 5.0f, lineY));
    window.draw(shortcuts);
}

#endif // DEBUG_VISU
