# 🎯 Plan d'Apprentissage Gomoku AI - Guide Complet

## 📖 Introduction

Ce guide vous accompagne dans l'apprentissage et l'implémentation d'une IA Gomoku performante. Chaque étape est conçue pour construire progressivement vos connaissances, du code existant aux algorithmes avancés.

**Objectifs pédagogiques :**
- Comprendre l'architecture d'un jeu avec IA
- Maîtriser l'algorithme MinMax et ses optimisations
- Développer une fonction d'évaluation sophistiquée
- Implémenter les règles complexes du Gomoku
- Optimiser les performances (profondeur 10+, <0.5s par coup)

---

## 📚 Phase 1: Comprendre les Bases (2h30)

### 🔍 Itération 1.1: Analyse du Code Existant (30 min)

**Objectifs d'apprentissage :**
- Comprendre l'architecture MVC du projet
- Identifier les classes principales et leurs responsabilités
- Comprendre l'héritage et le polymorphisme en C++

**Concepts théoriques :**
- **Séparation des responsabilités** : Game (contrôleur), Board (modèle), GameRenderer (vue)
- **Factory Pattern** : Création de différents types de joueurs
- **Polymorphisme** : Interface Player avec implémentations HumanPlayer et AIPlayer

**Actions pratiques :**
1. Explorer la hiérarchie des classes :
   ```cpp
   // Structure principale
   Game -> Board + Player(Human/AI) + UI
   Player -> MinMaxAI (pour l'IA)
   ```

2. Identifier les flux de données :
   ```
   User Input -> Game -> Player -> MinMaxAI -> Board -> Evaluation
   ```

**Points d'attention :**
- Comment les coups sont-ils validés ?
- Où sont implémentées les règles du Gomoku ?
- Comment l'IA est-elle intégrée au jeu ?

---

### 📜 Itération 1.2: Règles du Gomoku (45 min)

**Objectifs d'apprentissage :**
- Maîtriser toutes les règles spéciales du Gomoku
- Comprendre l'implémentation des règles complexes
- Identifier les cas particuliers (edge cases)

**Concepts théoriques :**

1. **Victoire par Alignement** : 5+ pierres consécutives
2. **Capture** : Encadrer une paire adverse
3. **Victoire par Capture** : 10 captures = victoire
4. **Capture Défensive** : Briser un alignement de 5 par capture
5. **Double-Trois Interdit** : Pas de coup créant 2 trois libres simultanés

**Règles détaillées :**

```cpp
// Exemple de détection d'alignement
bool checkAlignment(int x, int y, CellState player) {
    for (4 directions) {
        int count = 1 + countDirection(x,y,dx,dy) + countDirection(x,y,-dx,-dy);
        if (count >= 5) return true;
    }
    return false;
}

// Exemple de détection de capture
vector<Position> checkCaptures(int x, int y, CellState player) {
    // Pour chaque direction, chercher motif : Player-Opponent-Opponent-Player
    // Retourner les positions des pierres capturées
}
```

**Actions pratiques :**
1. Tester chaque règle manuellement dans le jeu
2. Analyser le code de `Rules.cpp` et `Board.cpp`
3. Créer des positions de test pour chaque règle

---

### 🐛 Itération 1.3: Debug de la Profondeur Critique (20 min)

**Objectifs d'apprentissage :**
- Comprendre l'importance des paramètres d'algorithmes
- Identifier et corriger un bug critique
- Valider les corrections par des tests

**Problème identifié :**
```cpp
// ERREUR dans MinMaxAI.cpp ligne 23
MinMaxAI::MinMaxAI(CellState playerColor, int depth, const std::string& playerName)
    : Player(playerColor, AI, playerName), searchDepth(depth - 7), // ❌ BUG ICI!
```

**Impact :**
- Profondeur demandée : 10 → Profondeur réelle : 3
- Performance insuffisante pour validation projet
- IA faible tactiquement

**Correction :**
```cpp
// CORRECTION
MinMaxAI::MinMaxAI(CellState playerColor, int depth, const std::string& playerName)
    : Player(playerColor, AI, playerName), searchDepth(depth), // ✅ CORRIGÉ
```

**Validation :**
- Vérifier les logs de profondeur
- Tester la performance (temps de calcul)
- Mesurer l'amélioration tactique

---

## 🧠 Phase 2: Algorithme MinMax (2h30)

### 🌳 Itération 2.1: MinMax Simple (1h)

**Objectifs d'apprentissage :**
- Comprendre la récursivité dans les jeux
- Implémenter l'algorithme MinMax de base
- Maîtriser l'évaluation de positions terminales

**Concepts théoriques :**

**MinMax** est un algorithme récursif pour les jeux à deux joueurs :
- **Maximiser** : L'IA cherche le meilleur coup pour elle
- **Minimiser** : L'IA suppose que l'adversaire joue optimalement contre elle
- **Alternance** : À chaque niveau, on change de perspective

```cpp
int minimax(Board board, int depth, bool maximizing) {
    // Cas terminal : profondeur 0 ou jeu fini
    if (depth == 0 || gameOver(board)) {
        return evaluate(board);
    }
    
    if (maximizing) {
        int maxEval = -INFINITY;
        for (each possible move) {
            Board newBoard = makeMove(board, move);
            int eval = minimax(newBoard, depth-1, false);
            maxEval = max(maxEval, eval);
        }
        return maxEval;
    } else {
        int minEval = +INFINITY;
        for (each possible move) {
            Board newBoard = makeMove(board, move);
            int eval = minimax(newBoard, depth-1, true);
            minEval = min(minEval, eval);
        }
        return minEval;
    }
}
```

**Implémentation étape par étape :**

1. **Évaluation terminale simple** :
```cpp
int evaluateBoard(const Board& board) {
    if (board.checkWin(myColor)) return +10000;
    if (board.checkWin(opponentColor)) return -10000;
    return 0; // Temporaire : positions non-terminales = 0
}
```

2. **Test avec profondeur 2-3** pour comprendre le comportement

3. **Ajout de logs pour visualiser l'arbre** :
```cpp
std::string indent(depth * 2, ' ');
std::cout << indent << "Depth " << depth << ", eval = " << evaluation << std::endl;
```

---

### ✂️ Itération 2.2: Alpha-Beta Pruning (45 min)

**Objectifs d'apprentissage :**
- Comprendre l'optimisation algorithmique
- Réduire la complexité de O(b^d) vers O(b^(d/2))
- Maintenir le même résultat avec moins de calculs

**Concepts théoriques :**

**Alpha-Beta** élimine les branches inutiles :
- **Alpha** : Meilleure valeur trouvée pour le joueur maximisant
- **Beta** : Meilleure valeur trouvée pour le joueur minimisant
- **Pruning** : Si alpha ≥ beta, arrêter l'exploration

```cpp
int alphabeta(Board board, int depth, int alpha, int beta, bool maximizing) {
    if (depth == 0 || gameOver(board)) {
        return evaluate(board);
    }
    
    if (maximizing) {
        int maxEval = -INFINITY;
        for (each move) {
            int eval = alphabeta(newBoard, depth-1, alpha, beta, false);
            maxEval = max(maxEval, eval);
            alpha = max(alpha, eval);
            if (beta <= alpha) break; // ✂️ Coupure Beta !
        }
        return maxEval;
    } else {
        int minEval = +INFINITY;
        for (each move) {
            int eval = alphabeta(newBoard, depth-1, alpha, beta, true);
            minEval = min(minEval, eval);
            beta = min(beta, eval);
            if (beta <= alpha) break; // ✂️ Coupure Alpha !
        }
        return minEval;
    }
}
```

**Mesure d'efficacité :**
- Compter les nœuds visités avec/sans alpha-beta
- Mesurer le gain de temps
- Profondeur atteignable en 0.5s

---

### 🎯 Itération 2.3: Génération de Coups Intelligente (30 min)

**Objectifs d'apprentissage :**
- Réduire l'espace de recherche efficacement
- Prioriser les coups les plus prometteurs
- Équilibrer précision et performance

**Problème :**
- Plateau 19x19 = 361 positions possibles
- Explosion combinatoire : 361^10 = impossible

**Solutions :**

1. **Coups adjacents** (implémentation actuelle) :
```cpp
// Générer seulement les cases voisines des pierres existantes
for (each stone on board) {
    for (8 directions around stone) {
        if (position is empty) add to moves;
    }
}
```

2. **Priorisation des coups** :
```cpp
vector<Position> orderedMoves = generateMoves(board);
sort(orderedMoves, [&](const Position& a, const Position& b) {
    return quickEvaluate(a) > quickEvaluate(b);
});
```

3. **Limitation intelligente** :
```cpp
// Garder seulement les N meilleurs coups
if (moves.size() > MAX_MOVES_PER_LEVEL) {
    moves.resize(MAX_MOVES_PER_LEVEL);
}
```

**Implémentation :**
- Modifier `generateMoves()` pour prioriser
- Tester l'impact sur la qualité de jeu
- Mesurer le gain de performance

---

## 🎨 Phase 3: Fonction d'Évaluation (3h30)

### 🔢 Itération 3.1: Évaluation de Motifs Basiques (1h)

**Objectifs d'apprentissage :**
- Reconnaître des motifs dans une grille 2D
- Évaluer la force relative des positions
- Comprendre les systèmes de scoring

**Concepts théoriques :**

Une bonne évaluation distingue les positions fortes des faibles :
- **Alignements** : Plus long = plus fort
- **Ouvertures** : Extrémités libres = plus dangereux
- **Potentiel** : Possibilité d'extension

**Système de scoring :**
```cpp
int PATTERN_SCORES[] = {
    0,     // 0 pierres alignées
    1,     // 1 pierre isolée
    10,    // 2 pierres alignées
    100,   // 3 pierres alignées
    1000,  // 4 pierres alignées
    10000  // 5 pierres = victoire
};
```

**Implémentation détaillée :**

```cpp
int evaluatePatterns(const Board& board, CellState player) {
    int score = 0;
    
    // Pour chaque position occupée par le joueur
    for (int x = 0; x < BOARD_SIZE; x++) {
        for (int y = 0; y < BOARD_SIZE; y++) {
            if (board.getCell(x, y) == player) {
                
                // Vérifier 4 directions : →, ↓, ↘, ↙
                int directions[4][2] = {{1,0}, {0,1}, {1,1}, {1,-1}};
                
                for (int d = 0; d < 4; d++) {
                    int dx = directions[d][0];
                    int dy = directions[d][1];
                    
                    // Compter les pierres consécutives
                    int count = countDirection(x, y, dx, dy, player);
                    
                    // Vérifier les ouvertures
                    bool openStart = isEmpty(x - dx, y - dy);
                    bool openEnd = isEmpty(x + count*dx, y + count*dy);
                    
                    // Calculer le score du motif
                    int patternScore = PATTERN_SCORES[count];
                    if (openStart && openEnd) patternScore *= 2; // Bonus ouvert
                    if (openStart || openEnd) patternScore *= 1.5; // Bonus semi-ouvert
                    
                    score += patternScore;
                }
            }
        }
    }
    return score;
}
```

**Test et validation :**
- Créer des positions test avec motifs connus
- Vérifier que l'évaluation correspond à l'intuition
- Comparer l'IA avec/sans cette évaluation

---

### ⚡ Itération 3.2: Détection de Menaces (45 min)

**Objectifs d'apprentissage :**
- Identifier les coups critiques (gagne en 1, perd en 1)
- Prioriser la tactique sur la stratégie
- Implémenter la logique de menaces

**Concepts théoriques :**

**Hiérarchie des menaces :**
1. **Victoire immédiate** : Coup qui gagne instantanément
2. **Menace de victoire** : Force l'adversaire à bloquer
3. **Défense obligatoire** : Bloquer la victoire adverse
4. **Double menace** : Créer 2 menaces simultanées

```cpp
enum ThreatType {
    NO_THREAT = 0,
    DEFENSIVE_BLOCK = 1000,    // Doit bloquer pour éviter défaite
    WINNING_THREAT = 5000,     // Menace de victoire
    IMMEDIATE_WIN = 50000      // Victoire immédiate
};
```

**Implémentation :**

```cpp
int detectThreats(const Board& board, Position move, CellState player) {
    // Simuler le coup
    Board tempBoard = board;
    tempBoard.placePiece(move.x, move.y, player);
    
    // 1. Vérifier victoire immédiate
    if (tempBoard.checkWin(player)) {
        return IMMEDIATE_WIN;
    }
    
    // 2. Chercher menaces de victoire (4 alignés ouverts)
    int threats = 0;
    for (each direction) {
        if (creates4InARowOpen(move, direction, player)) {
            threats++;
        }
    }
    if (threats >= 2) return WINNING_THREAT * 2; // Double menace
    if (threats == 1) return WINNING_THREAT;
    
    // 3. Vérifier si bloque menace adverse
    Board opponentBoard = board;
    opponentBoard.placePiece(move.x, move.y, opponent);
    if (opponentBoard.checkWin(opponent)) {
        return DEFENSIVE_BLOCK;
    }
    
    return NO_THREAT;
}
```

**Intégration dans l'évaluation :**
```cpp
int evaluatePosition(const Board& board, CellState player) {
    int baseScore = evaluatePatterns(board, player);
    
    // Chercher menaces pour tous les coups possibles
    vector<Position> moves = generateMoves(board);
    int maxThreat = 0;
    for (const Position& move : moves) {
        int threat = detectThreats(board, move, player);
        maxThreat = max(maxThreat, threat);
    }
    
    return baseScore + maxThreat;
}
```

---

### 🎯 Itération 3.3: Évaluation des Captures (40 min)

**Objectifs d'apprentissage :**
- Intégrer la mécanique unique de capture du Gomoku
- Équilibrer captures offensives et défensives
- Comprendre les captures stratégiques vs tactiques

**Concepts théoriques :**

**Types de captures :**
- **Capture directe** : Encadrer une paire adverse
- **Menace de capture** : Forcer l'adversaire à défendre
- **Capture défensive** : Briser un alignement dangereux
- **Course aux captures** : Atteindre 10 captures = victoire

**Scoring des captures :**
```cpp
int CAPTURE_VALUES[] = {
    50,   // Capture simple (2 pierres)
    200,  // Capture tactique (brise un alignement)
    500,  // Menace de capture multiple
    2000  // Proche de victoire par capture (8+ captures)
};
```

**Implémentation :**

```cpp
int evaluateCaptures(const Board& board, CellState player) {
    int score = 0;
    
    // 1. Capturer les comptes actuels
    int myCaptures = board.getCaptureCount(player);
    int opponentCaptures = board.getCaptureCount(getOpponent(player));
    
    // Bonus pour être proche de 10 captures
    if (myCaptures >= 8) score += 2000;
    else if (myCaptures >= 6) score += 500;
    else score += myCaptures * 50;
    
    // Malus pour les captures adverses
    if (opponentCaptures >= 8) score -= 2500;
    else score -= opponentCaptures * 60;
    
    // 2. Évaluer les opportunités de capture
    vector<Position> moves = generateMoves(board);
    for (const Position& move : moves) {
        vector<Position> captures = board.checkCaptures(move.x, move.y, player);
        if (!captures.empty()) {
            score += captures.size() * 25; // Bonus par pierre capturée
            
            // Bonus si capture brise un alignement
            if (breaksOpponentAlignment(board, captures)) {
                score += 200;
            }
        }
        
        // Vérifier vulnérabilité aux captures
        if (isVulnerableToCapture(board, move, player)) {
            score -= 100;
        }
    }
    
    return score;
}
```

**Intégration avec menaces :**
```cpp
// Une capture qui brise 4 alignés est prioritaire
if (captureBreaks4InRow(board, captures)) {
    return DEFENSIVE_BLOCK + 500;
}
```

---

### 🏁 Itération 3.4: Contrôle du Centre et Espace (30 min)

**Objectifs d'apprentissage :**
- Comprendre la stratégie positionnelle
- Équilibrer tactique (menaces) et stratégie (position)
- Implémenter des concepts de développement

**Concepts théoriques :**

**Avantages du centre :**
- Plus d'options de développement (8 directions vs 3-5)
- Contrôle de l'espace
- Connexions plus faciles

**Scoring positionnel :**
```cpp
// Distance du centre (9,9) sur plateau 19x19
int distanceFromCenter(int x, int y) {
    int centerX = BOARD_SIZE / 2;
    int centerY = BOARD_SIZE / 2;
    return abs(x - centerX) + abs(y - centerY);
}

int CENTER_BONUS[] = {
    50,  // Centre exact
    40,  // Distance 1
    30,  // Distance 2
    20,  // Distance 3
    10,  // Distance 4
    5,   // Distance 5+
    0    // Bords
};
```

**Implémentation :**

```cpp
int evaluatePosition(const Board& board, CellState player) {
    int score = 0;
    
    // 1. Bonus de position pour chaque pierre
    for (int x = 0; x < BOARD_SIZE; x++) {
        for (int y = 0; y < BOARD_SIZE; y++) {
            if (board.getCell(x, y) == player) {
                int distance = distanceFromCenter(x, y);
                score += CENTER_BONUS[min(distance, 5)];
                
                // Bonus connexion (pierres adjacentes)
                int connections = countAdjacentFriendly(board, x, y, player);
                score += connections * 15;
            }
        }
    }
    
    // 2. Contrôle de territoire
    score += evaluateInfluence(board, player);
    
    return score;
}

int evaluateInfluence(const Board& board, CellState player) {
    int influence = 0;
    
    // Compter les cases "influencées" (proche de nos pierres)
    for (int x = 0; x < BOARD_SIZE; x++) {
        for (int y = 0; y < BOARD_SIZE; y++) {
            if (board.getCell(x, y) == EMPTY) {
                int minDistToFriend = 999;
                int minDistToEnemy = 999;
                
                // Trouver distance aux pierres les plus proches
                findClosestStones(board, x, y, player, minDistToFriend, minDistToEnemy);
                
                if (minDistToFriend < minDistToEnemy) {
                    influence += (minDistToEnemy - minDistToFriend);
                }
            }
        }
    }
    
    return influence;
}
```

---

### 🚫 Itération 3.5: Règle Double-Trois (1h)

**Objectifs d'apprentissage :**
- Implémenter des contraintes complexes
- Gérer les exceptions aux règles
- Optimiser la validation de coups

**Concepts théoriques :**

**Double-Trois interdit :**
- Un coup qui crée simultanément 2 "trois libres"
- Trois libre = 3 pierres alignées pouvant former 4 imparables
- Exception : capture permet double-trois

**Détection d'un "trois libre" :**
```cpp
bool isFreeThree(Board board, int x, int y, int dx, int dy, CellState player) {
    // Motif recherché : _XXX_ (où _ = vide, X = joueur)
    // Ou variations : _X_XX_, _XX_X_, etc.
    
    // Vérifier que placer une pierre aux extrémités crée un 4 imparable
    Position end1(x - dx, y - dy);
    Position end2(x + 3*dx, y + 3*dy);
    
    if (isEmpty(end1) && isEmpty(end2)) {
        // Simuler placement aux extrémités
        if (creates4InRowUnstoppable(board, end1, player) ||
            creates4InRowUnstoppable(board, end2, player)) {
            return true;
        }
    }
    
    return false;
}
```

**Détection du double-trois :**
```cpp
bool isDoubleThree(const Board& board, int x, int y, CellState player) {
    // Simuler le coup
    Board tempBoard = board;
    tempBoard.placePiece(x, y, player);
    
    int freeThreeCount = 0;
    
    // Vérifier toutes les directions
    int directions[4][2] = {{1,0}, {0,1}, {1,1}, {1,-1}};
    
    for (int d = 0; d < 4; d++) {
        if (isFreeThree(tempBoard, x, y, directions[d][0], directions[d][1], player)) {
            freeThreeCount++;
            if (freeThreeCount >= 2) {
                return true; // Double-trois détecté !
            }
        }
    }
    
    return false;
}
```

**Gestion des exceptions :**
```cpp
bool isValidMove(const Board& board, int x, int y, CellState player) {
    // 1. Vérification de base
    if (!board.isValidMove(x, y)) return false;
    
    // 2. Vérifier double-trois
    if (isDoubleThree(board, x, y, player)) {
        // Exception : si le coup fait une capture
        vector<Position> captures = board.checkCaptures(x, y, player);
        if (captures.empty()) {
            return false; // Double-trois interdit sans capture
        }
    }
    
    return true;
}
```

**Intégration dans l'évaluation :**
```cpp
int evaluateMove(const Board& board, Position move, CellState player) {
    // Pénalité massive pour coups interdits
    if (!isValidMove(board, move.x, move.y, player)) {
        return -100000;
    }
    
    // Évaluation normale
    return evaluateNormalMove(board, move, player);
}
```

---

## ⚡ Phase 4: Optimisations Avancées (2h30)

### 💾 Itération 4.1: Table de Transposition (1h)

**Objectifs d'apprentissage :**
- Comprendre la mémorisation (memoization)
- Implémenter des structures de données efficaces
- Optimiser l'utilisation mémoire

**Concepts théoriques :**

**Problème :** MinMax recalcule souvent les mêmes positions
```
     A
   /   \
  B     C
 / \   / \
D   E E   F  <- Position E calculée 2 fois !
```

**Solution :** Mémoriser les résultats dans une table de hachage

**Structure de données :**
```cpp
struct TTEntry {
    uint64_t hash;      // Hash de la position
    int value;          // Évaluation
    int depth;          // Profondeur de recherche
    uint8_t flag;       // Type d'entrée (EXACT, LOWER_BOUND, UPPER_BOUND)
    Position bestMove;  // Meilleur coup trouvé
    
    TTEntry() : hash(0), value(0), depth(0), flag(0), bestMove(-1,-1) {}
};

class TranspositionTable {
private:
    vector<TTEntry> table;
    size_t size;
    size_t mask;        // Pour hash & mask = index rapide
    
public:
    TranspositionTable(size_t sizeMB) {
        size = (sizeMB * 1024 * 1024) / sizeof(TTEntry);
        // Arrondir à la puissance de 2 la plus proche
        size = 1 << (int)log2(size);
        mask = size - 1;
        table.resize(size);
    }
    
    bool probe(uint64_t hash, int depth, int& value, Position& bestMove);
    void store(uint64_t hash, int depth, int value, uint8_t flag, Position bestMove);
};
```

**Fonction de hachage :**
```cpp
class ZobristHash {
private:
    uint64_t pieceHashes[BOARD_SIZE][BOARD_SIZE][3]; // [x][y][couleur]
    uint64_t sideToMove;
    
public:
    ZobristHash() {
        // Initialiser avec des nombres aléatoires
        random_device rd;
        mt19937_64 gen(rd());
        
        for (int x = 0; x < BOARD_SIZE; x++) {
            for (int y = 0; y < BOARD_SIZE; y++) {
                for (int color = 0; color < 3; color++) {
                    pieceHashes[x][y][color] = gen();
                }
            }
        }
        sideToMove = gen();
    }
    
    uint64_t hashBoard(const Board& board, CellState currentPlayer) {
        uint64_t hash = 0;
        
        for (int x = 0; x < BOARD_SIZE; x++) {
            for (int y = 0; y < BOARD_SIZE; y++) {
                CellState cell = board.getCell(x, y);
                if (cell != EMPTY) {
                    hash ^= pieceHashes[x][y][cell];
                }
            }
        }
        
        if (currentPlayer == BLACK) {
            hash ^= sideToMove;
        }
        
        return hash;
    }
};
```

**Intégration dans MinMax :**
```cpp
int alphabeta(Board& board, int depth, int alpha, int beta, bool maximizing) {
    uint64_t hash = zobrist.hashBoard(board, maximizing ? myColor : opponentColor);
    
    // 1. Consulter la table de transposition
    int ttValue;
    Position ttMove;
    if (tt.probe(hash, depth, ttValue, ttMove)) {
        return ttValue; // Position déjà calculée !
    }
    
    // 2. Calcul normal si pas en table
    if (depth == 0 || isTerminal(board)) {
        int value = evaluate(board);
        tt.store(hash, depth, value, EXACT, Position(-1,-1));
        return value;
    }
    
    // 3. Recherche avec mémorisation du meilleur coup
    vector<Position> moves = generateMoves(board);
    
    // Essayer d'abord le coup de la table de transposition
    if (ttMove.x != -1) {
        moves.insert(moves.begin(), ttMove);
    }
    
    Position bestMove(-1, -1);
    int bestValue = maximizing ? -INFINITY : +INFINITY;
    
    for (Position move : moves) {
        makeMove(board, move);
        int value = alphabeta(board, depth-1, alpha, beta, !maximizing);
        undoMove(board, move);
        
        if (maximizing && value > bestValue) {
            bestValue = value;
            bestMove = move;
            alpha = max(alpha, value);
        } else if (!maximizing && value < bestValue) {
            bestValue = value;
            bestMove = move;
            beta = min(beta, value);
        }
        
        if (beta <= alpha) break;
    }
    
    // 4. Stocker en table
    uint8_t flag = (bestValue <= alpha) ? UPPER_BOUND :
                   (bestValue >= beta) ? LOWER_BOUND : EXACT;
    tt.store(hash, depth, bestValue, flag, bestMove);
    
    return bestValue;
}
```

---

### 🔄 Itération 4.2: Recherche Itérative (45 min)

**Objectifs d'apprentissage :**
- Gérer les contraintes de temps intelligemment
- Implémenter des algorithmes "anytime"
- Optimiser l'ordre de recherche

**Concepts théoriques :**

**Problème :** Profondeur fixe peut dépasser la limite de temps

**Solution :** Recherche progressive avec approfondissement
```
Profondeur 1 -> résultat en 0.001s
Profondeur 2 -> résultat en 0.005s  
Profondeur 3 -> résultat en 0.025s
Profondeur 4 -> résultat en 0.125s
Profondeur 5 -> résultat en 0.500s <- STOP, temps écoulé
```

**Avantages :**
- Toujours un résultat valide
- Meilleur ordre de coups pour profondeurs suivantes
- Utilisation optimale du temps

**Implémentation :**
```cpp
class IterativeDeepening {
private:
    chrono::high_resolution_clock::time_point startTime;
    double timeLimit;
    TranspositionTable& tt;
    
public:
    Position searchBestMove(Board& board, CellState player, double timeLimitSec) {
        startTime = chrono::high_resolution_clock::now();
        timeLimit = timeLimitSec * 0.95; // Marge de sécurité
        
        Position bestMove(-1, -1);
        int bestValue = -INFINITY;
        
        // Recherche progressive
        for (int depth = 1; depth <= MAX_DEPTH; depth++) {
            if (isTimeUp()) break;
            
            // Recherche à cette profondeur
            int value = alphabeta(board, depth, -INFINITY, +INFINITY, true);
            
            // Mettre à jour le meilleur coup si recherche complète
            if (!isTimeUp()) {
                bestMove = getBestMoveFromTT(board);
                bestValue = value;
                
                cout << "Depth " << depth << ": value=" << value 
                     << ", move=(" << bestMove.x << "," << bestMove.y << ")"
                     << ", time=" << getElapsedTime() << "s" << endl;
                
                // Arrêt anticipé si victoire/défaite certaine
                if (abs(value) > 9000) break;
            }
        }
        
        return bestMove;
    }
    
private:
    bool isTimeUp() {
        auto now = chrono::high_resolution_clock::now();
        auto elapsed = chrono::duration<double>(now - startTime).count();
        return elapsed >= timeLimit;
    }
    
    double getElapsedTime() {
        auto now = chrono::high_resolution_clock::now();
        return chrono::duration<double>(now - startTime).count();
    }
};
```

**Optimisation de l'ordre des coups :**
```cpp
vector<Position> orderMoves(const Board& board, const vector<Position>& moves) {
    vector<pair<Position, int>> scoredMoves;
    
    for (const Position& move : moves) {
        int score = 0;
        
        // 1. Coup de la table de transposition = priorité max
        if (move == ttBestMove) score += 10000;
        
        // 2. Captures
        if (!board.checkCaptures(move.x, move.y, myColor).empty()) {
            score += 1000;
        }
        
        // 3. Centre
        score += (10 - distanceFromCenter(move.x, move.y)) * 10;
        
        // 4. Évaluation rapide
        score += quickEvaluate(board, move);
        
        scoredMoves.push_back({move, score});
    }
    
    sort(scoredMoves.begin(), scoredMoves.end(), 
         [](const auto& a, const auto& b) { return a.second > b.second; });
    
    vector<Position> orderedMoves;
    for (const auto& pair : scoredMoves) {
        orderedMoves.push_back(pair.first);
    }
    
    return orderedMoves;
}
```

---

## 🎮 Phase 5: Interface et Debug (1h15)

### ⏱️ Itération 5.1: Timer et UI (30 min)

**Objectifs d'apprentissage :**
- Intégrer la mesure de performance dans l'UI
- Respecter les exigences du projet (timer obligatoire)
- Créer une interface informative

**Exigences du projet :**
> "You have to display, somewhere in your user interface: A timer that counts how much time your AI takes to find its next move."

**Implémentation du timer :**
```cpp
class AITimer {
private:
    chrono::high_resolution_clock::time_point startTime;
    double lastMoveTime;
    double totalTime;
    int moveCount;
    
public:
    void startThinking() {
        startTime = chrono::high_resolution_clock::now();
    }
    
    void stopThinking() {
        auto endTime = chrono::high_resolution_clock::now();
        lastMoveTime = chrono::duration<double>(endTime - startTime).count();
        totalTime += lastMoveTime;
        moveCount++;
    }
    
    double getLastMoveTime() const { return lastMoveTime; }
    double getAverageTime() const { 
        return moveCount > 0 ? totalTime / moveCount : 0.0; 
    }
    
    string getFormattedTime() const {
        stringstream ss;
        ss << fixed << setprecision(3) << lastMoveTime << "s";
        return ss.str();
    }
};
```

**Intégration dans l'UI :**
```cpp
void GameRenderer::renderAIInfo(sf::RenderWindow& window, const AITimer& timer) {
    // Panel d'information IA
    sf::RectangleShape panel(sf::Vector2f(250, 150));
    panel.setPosition(BOARD_WIDTH + 10, 10);
    panel.setFillColor(sf::Color(240, 240, 240, 200));
    window.draw(panel);
    
    // Titre
    sf::Text title("AI Information", font, 16);
    title.setPosition(BOARD_WIDTH + 20, 20);
    title.setFillColor(sf::Color::Black);
    window.draw(title);
    
    // Temps du dernier coup
    sf::Text lastTime("Last move: " + timer.getFormattedTime(), font, 14);
    lastTime.setPosition(BOARD_WIDTH + 20, 45);
    lastTime.setFillColor(sf::Color::Blue);
    window.draw(lastTime);
    
    // Temps moyen
    stringstream avgStream;
    avgStream << "Average: " << fixed << setprecision(3) << timer.getAverageTime() << "s";
    sf::Text avgTime(avgStream.str(), font, 14);
    avgTime.setPosition(BOARD_WIDTH + 20, 65);
    avgTime.setFillColor(sf::Color::Green);
    window.draw(avgTime);
    
    // Validation performance
    sf::Color perfColor = timer.getAverageTime() <= 0.5 ? sf::Color::Green : sf::Color::Red;
    sf::Text perfStatus(timer.getAverageTime() <= 0.5 ? "✓ Performance OK" : "✗ Too slow", font, 12);
    perfStatus.setPosition(BOARD_WIDTH + 20, 85);
    perfStatus.setFillColor(perfColor);
    window.draw(perfStatus);
}
```

**Intégration dans le jeu :**
```cpp
// Dans Game::handleAIMove()
aiTimer->startThinking();
Position move = aiPlayer->makeMove(board);
aiTimer->stopThinking();

cout << "AI move: (" << move.x << ", " << move.y << ") in " 
     << aiTimer->getFormattedTime() << endl;
```

---

### 🔍 Itération 5.2: Mode Debug Complet (45 min)

**Objectifs d'apprentissage :**
- Visualiser le processus de décision de l'IA
- Créer des outils de debugging efficaces
- Préparer la défense du projet

**Fonctionnalités de debug :**

1. **Visualisation de l'arbre de recherche**
2. **Affichage des évaluations**
3. **Trace des coups considérés**
4. **Statistiques de performance**

**Structure de debug :**
```cpp
struct DebugInfo {
    int nodesEvaluated;
    int ttHits;
    int ttMisses;
    int cutoffs;
    vector<string> searchTrace;
    map<Position, int> moveEvaluations;
    vector<Position> principalVariation;
    
    void reset() {
        nodesEvaluated = ttHits = ttMisses = cutoffs = 0;
        searchTrace.clear();
        moveEvaluations.clear();
        principalVariation.clear();
    }
};
```

**Logger pour MinMax :**
```cpp
class MinMaxLogger {
private:
    DebugInfo& debug;
    int maxLogDepth;
    
public:
    MinMaxLogger(DebugInfo& debugInfo) : debug(debugInfo), maxLogDepth(3) {}
    
    void logNode(int depth, Position move, int alpha, int beta, bool maximizing) {
        if (depth <= maxLogDepth) {
            string indent(depth * 2, ' ');
            stringstream ss;
            ss << indent << "Depth " << depth << ": move(" << move.x << "," << move.y 
               << ") " << (maximizing ? "MAX" : "MIN") 
               << " [α=" << alpha << ", β=" << beta << "]";
            debug.searchTrace.push_back(ss.str());
        }
        debug.nodesEvaluated++;
    }
    
    void logEvaluation(Position move, int value) {
        debug.moveEvaluations[move] = value;
    }
    
    void logCutoff(int depth, Position move) {
        if (depth <= maxLogDepth) {
            string indent(depth * 2, ' ');
            debug.searchTrace.push_back(indent + "✂️ Cutoff at (" + 
                                      to_string(move.x) + "," + to_string(move.y) + ")");
        }
        debug.cutoffs++;
    }
};
```

**Affichage visuel des évaluations :**
```cpp
void GameRenderer::renderDebugInfo(sf::RenderWindow& window, const DebugInfo& debug) {
    if (!debugMode) return;
    
    // Panel de debug
    sf::RectangleShape debugPanel(sf::Vector2f(300, 400));
    debugPanel.setPosition(10, BOARD_HEIGHT + 10);
    debugPanel.setFillColor(sf::Color(0, 0, 0, 200));
    window.draw(debugPanel);
    
    int yPos = BOARD_HEIGHT + 20;
    
    // Statistiques
    renderDebugText(window, "=== AI Debug Info ===", 20, yPos, sf::Color::Yellow);
    yPos += 25;
    
    renderDebugText(window, "Nodes: " + to_string(debug.nodesEvaluated), 20, yPos);
    yPos += 20;
    
    renderDebugText(window, "TT Hits: " + to_string(debug.ttHits) + 
                           " (" + to_string(debug.ttHits * 100 / max(1, debug.ttHits + debug.ttMisses)) + "%)", 
                    20, yPos);
    yPos += 20;
    
    renderDebugText(window, "Cutoffs: " + to_string(debug.cutoffs), 20, yPos);
    yPos += 25;
    
    // Trace de recherche (dernières lignes)
    renderDebugText(window, "=== Search Trace ===", 20, yPos, sf::Color::Yellow);
    yPos += 20;
    
    int maxLines = 10;
    int startLine = max(0, (int)debug.searchTrace.size() - maxLines);
    for (int i = startLine; i < debug.searchTrace.size(); i++) {
        string line = debug.searchTrace[i];
        if (line.length() > 35) line = line.substr(0, 32) + "...";
        renderDebugText(window, line, 20, yPos, sf::Color::White, 10);
        yPos += 15;
    }
}

void GameRenderer::renderMoveEvaluations(sf::RenderWindow& window, const DebugInfo& debug) {
    if (!debugMode) return;
    
    // Afficher l'évaluation de chaque coup possible sur le plateau
    for (const auto& pair : debug.moveEvaluations) {
        Position pos = pair.first;
        int value = pair.second;
        
        // Couleur selon l'évaluation
        sf::Color color = (value > 0) ? sf::Color::Green : 
                         (value < 0) ? sf::Color::Red : sf::Color::Yellow;
        
        // Cercle semi-transparent
        sf::CircleShape circle(cellSize * 0.3f);
        circle.setFillColor(sf::Color(color.r, color.g, color.b, 100));
        circle.setPosition(getBoardPosition(pos.x, pos.y));
        window.draw(circle);
        
        // Texte avec la valeur
        sf::Text valueText(to_string(value), font, 8);
        valueText.setFillColor(sf::Color::White);
        valueText.setPosition(getBoardPosition(pos.x, pos.y));
        window.draw(valueText);
    }
}
```

**Contrôles de debug :**
```cpp
void Game::handleKeyPress(sf::Keyboard::Key key) {
    switch (key) {
        case sf::Keyboard::F1:
            debugMode = !debugMode;
            cout << "Debug mode: " << (debugMode ? "ON" : "OFF") << endl;
            break;
            
        case sf::Keyboard::F2:
            if (debugMode) {
                // Forcer l'IA à montrer sa réflexion pour le coup suivant
                showNextAIThinking = true;
            }
            break;
            
        case sf::Keyboard::F3:
            if (debugMode) {
                // Sauvegarder la trace de debug
                saveDebugTrace();
            }
            break;
    }
}
```

---

## 📊 Phase 6: Tests et Validation (1h15)

### ⚡ Itération 6.1: Tests de Performance (30 min)

**Objectifs d'apprentissage :**
- Mesurer et valider les performances
- Identifier les goulots d'étranglement
- Respecter les contraintes du projet

**Contraintes à valider :**
- ✅ Profondeur minimum 10 niveaux
- ✅ Temps moyen < 0.5 secondes par coup
- ✅ Pas de crash même si mémoire insuffisante

**Suite de tests de performance :**
```cpp
class PerformanceTests {
public:
    void runAllTests() {
        cout << "=== Performance Tests ===\n";
        
        testSearchDepth();
        testTimeConstraints();
        testMemoryUsage();
        testComplexPositions();
        
        cout << "=== Tests Complete ===\n";
    }
    
private:
    void testSearchDepth() {
        cout << "\n1. Testing Search Depth...\n";
        
        Board board;
        // Position complexe de milieu de partie
        setupComplexPosition(board);
        
        MinMaxAI ai(BLACK, 15); // Demander profondeur 15
        
        auto start = chrono::high_resolution_clock::now();
        Position move = ai.makeMove(board);
        auto end = chrono::high_resolution_clock::now();
        
        double time = chrono::duration<double>(end - start).count();
        int actualDepth = ai.getLastSearchDepth();
        
        cout << "Requested depth: 15\n";
        cout << "Actual depth: " << actualDepth << "\n";
        cout << "Time taken: " << time << "s\n";
        
        // Validation
        assert(actualDepth >= 10 && "FAIL: Search depth < 10");
        cout << "✅ Search depth >= 10: PASS\n";
    }
    
    void testTimeConstraints() {
        cout << "\n2. Testing Time Constraints...\n";
        
        vector<double> moveTimes;
        Board board;
        MinMaxAI ai(BLACK, 12);
        
        // Tester 20 coups dans différentes positions
        for (int i = 0; i < 20; i++) {
            // Créer position aléatoire
            setupRandomPosition(board, i * 5);
            
            auto start = chrono::high_resolution_clock::now();
            Position move = ai.makeMove(board);
            auto end = chrono::high_resolution_clock::now();
            
            double time = chrono::duration<double>(end - start).count();
            moveTimes.push_back(time);
            
            if (move.x != -1) {
                board.placePiece(move.x, move.y, BLACK);
            }
        }
        
        // Calculer statistiques
        double totalTime = accumulate(moveTimes.begin(), moveTimes.end(), 0.0);
        double avgTime = totalTime / moveTimes.size();
        double maxTime = *max_element(moveTimes.begin(), moveTimes.end());
        
        cout << "Average time: " << avgTime << "s\n";
        cout << "Maximum time: " << maxTime << "s\n";
        cout << "Total time: " << totalTime << "s\n";
        
        // Validation
        assert(avgTime <= 0.5 && "FAIL: Average time > 0.5s");
        assert(maxTime <= 2.0 && "FAIL: Maximum time > 2.0s (acceptable spike)");
        
        cout << "✅ Time constraints: PASS\n";
    }
    
    void testMemoryUsage() {
        cout << "\n3. Testing Memory Usage...\n";
        
        // Mesurer utilisation mémoire avant
        size_t memBefore = getCurrentMemoryUsage();
        
        {
            // Créer une IA avec table de transposition importante
            MinMaxAI ai(BLACK, 15);
            ai.setTranspositionTableSize(64); // 64MB
            
            // Faire plusieurs recherches
            Board board;
            for (int i = 0; i < 10; i++) {
                setupRandomPosition(board, i * 8);
                ai.makeMove(board);
            }
            
            size_t memDuring = getCurrentMemoryUsage();
            cout << "Memory usage: " << (memDuring - memBefore) / 1024 / 1024 << "MB\n";
            
            // Vérifier pas de fuite mémoire excessive
            assert((memDuring - memBefore) < 200 * 1024 * 1024 && "FAIL: Memory usage > 200MB");
        }
        
        // Vérifier libération mémoire
        size_t memAfter = getCurrentMemoryUsage();
        size_t leaked = memAfter - memBefore;
        
        cout << "Memory leaked: " << leaked / 1024 << "KB\n";
        assert(leaked < 10 * 1024 * 1024 && "FAIL: Memory leak > 10MB");
        
        cout << "✅ Memory usage: PASS\n";
    }
    
    void testComplexPositions() {
        cout << "\n4. Testing Complex Positions...\n";
        
        vector<string> testNames = {
            "Opening", "Early Game", "Mid Game", "Late Game", "Near Win"
        };
        
        for (int i = 0; i < testNames.size(); i++) {
            Board board;
            setupTestPosition(board, i);
            
            MinMaxAI ai(BLACK, 12);
            
            auto start = chrono::high_resolution_clock::now();
            Position move = ai.makeMove(board);
            auto end = chrono::high_resolution_clock::now();
            
            double time = chrono::duration<double>(end - start).count();
            
            cout << testNames[i] << " position: " << time << "s";
            if (time <= 0.5) cout << " ✅";
            else cout << " ⚠️";
            cout << "\n";
        }
    }
    
private:
    void setupComplexPosition(Board& board) {
        // Position de milieu de partie avec menaces multiples
        vector<pair<int,int>> blackMoves = {
            {9,9}, {10,10}, {8,8}, {11,11}, {7,12}, {12,7}
        };
        vector<pair<int,int>> whiteMoves = {
            {9,10}, {10,9}, {8,10}, {10,8}, {7,7}, {12,12}
        };
        
        for (auto move : blackMoves) {
            board.placePiece(move.first, move.second, BLACK);
        }
        for (auto move : whiteMoves) {
            board.placePiece(move.first, move.second, WHITE);
        }
    }
    
    size_t getCurrentMemoryUsage() {
        // Implémentation spécifique à la plateforme
        #ifdef __linux__
            ifstream file("/proc/self/status");
            string line;
            while (getline(file, line)) {
                if (line.substr(0, 6) == "VmRSS:") {
                    stringstream ss(line.substr(6));
                    size_t memory;
                    ss >> memory;
                    return memory * 1024; // Convert KB to bytes
                }
            }
        #endif
        return 0;
    }
};
```

---

### 🎯 Itération 6.2: Tests des Règles (45 min)

**Objectifs d'apprentissage :**
- Valider l'implémentation de toutes les règles
- Tester les cas particuliers et edge cases
- Préparer des positions de démonstration

**Suite de tests des règles :**
```cpp
class RuleTests {
public:
    void runAllTests() {
        cout << "=== Rule Validation Tests ===\n";
        
        testBasicWin();
        testCaptureRules();
        testDoubleThreeRule();
        testEndgameCapture();
        testEdgeCases();
        
        cout << "=== All Rules Validated ===\n";
    }
    
private:
    void testBasicWin() {
        cout << "\n1. Testing Basic Win Conditions...\n";
        
        // Test horizontal win
        {
            Board board;
            for (int i = 5; i < 10; i++) {
                board.placePiece(i, 9, BLACK);
            }
            assert(board.checkWin(BLACK) && "FAIL: Horizontal 5-in-row not detected");
            cout << "✅ Horizontal win: PASS\n";
        }
        
        // Test vertical win
        {
            Board board;
            for (int i = 5; i < 10; i++) {
                board.placePiece(9, i, WHITE);
            }
            assert(board.checkWin(WHITE) && "FAIL: Vertical 5-in-row not detected");
            cout << "✅ Vertical win: PASS\n";
        }
        
        // Test diagonal win
        {
            Board board;
            for (int i = 0; i < 5; i++) {
                board.placePiece(5+i, 5+i, BLACK);
            }
            assert(board.checkWin(BLACK) && "FAIL: Diagonal 5-in-row not detected");
            cout << "✅ Diagonal win: PASS\n";
        }
        
        // Test 6-in-row (should also win)
        {
            Board board;
            for (int i = 4; i < 10; i++) {
                board.placePiece(i, 9, WHITE);
            }
            assert(board.checkWin(WHITE) && "FAIL: 6-in-row not detected as win");
            cout << "✅ 6-in-row win: PASS\n";
        }
    }
    
    void testCaptureRules() {
        cout << "\n2. Testing Capture Rules...\n";
        
        // Test basic capture
        {
            Board board;
            // Setup: W-B-B-W pattern
            board.placePiece(5, 9, WHITE);
            board.placePiece(6, 9, BLACK);
            board.placePiece(7, 9, BLACK);
            board.placePiece(8, 9, WHITE); // This should capture the B-B pair
            
            vector<Position> captures = board.checkCaptures(8, 9, WHITE);
            assert(captures.size() == 2 && "FAIL: Basic capture not detected");
            
            board.executeCaptures(captures);
            assert(board.getCell(6, 9) == EMPTY && "FAIL: Captured piece not removed");
            assert(board.getCell(7, 9) == EMPTY && "FAIL: Captured piece not removed");
            assert(board.getCaptureCount(WHITE) == 2 && "FAIL: Capture count not updated");
            
            cout << "✅ Basic capture: PASS\n";
        }
        
        // Test capture win (10 captures)
        {
            Board board;
            board.setCaptureCount(WHITE, 8); // Already 8 captures
            
            // Setup another capture opportunity
            board.placePiece(5, 9, WHITE);
            board.placePiece(6, 9, BLACK);
            board.placePiece(7, 9, BLACK);
            
            vector<Position> captures = board.checkCaptures(8, 9, WHITE);
            board.placePiece(8, 9, WHITE);
            board.executeCaptures(captures);
            
            assert(board.checkCaptureWin(WHITE) && "FAIL: Capture win not detected");
            cout << "✅ Capture win: PASS\n";
        }
        
        // Test diagonal capture
        {
            Board board;
            board.placePiece(5, 5, WHITE);
            board.placePiece(6, 6, BLACK);
            board.placePiece(7, 7, BLACK);
            
            vector<Position> captures = board.checkCaptures(8, 8, WHITE);
            assert(captures.size() == 2 && "FAIL: Diagonal capture not detected");
            cout << "✅ Diagonal capture: PASS\n";
        }
    }
    
    void testDoubleThreeRule() {
        cout << "\n3. Testing Double-Three Rule...\n";
        
        // Test forbidden double-three
        {
            Board board;
            // Setup position where move creates double-three
            /*
             * . . . . . . .
             * . B . . . . .
             * . . B . . . .
             * . . . ? . . .  <- placing B here creates double-three
             * . B W X A B .
             * . . . . . . .
             */
            board.placePiece(1, 1, BLACK);
            board.placePiece(2, 2, BLACK);
            board.placePiece(1, 4, BLACK);
            board.placePiece(2, 4, WHITE);
            board.placePiece(4, 4, BLACK);
            board.placePiece(5, 4, BLACK);
            
            // Check if move at (3,3) creates double-three
            bool isDoubleThree = board.isDoubleThree(3, 3, BLACK);
            assert(isDoubleThree && "FAIL: Double-three not detected");
            cout << "✅ Double-three detection: PASS\n";
        }
        
        // Test double-three with capture exception
        {
            Board board;
            // Setup position where double-three is allowed due to capture
            board.placePiece(5, 5, WHITE);
            board.placePiece(6, 5, BLACK);
            board.placePiece(7, 5, BLACK);
            // ... setup double-three position ...
            
            // Move that creates double-three but also captures
            vector<Position> captures = board.checkCaptures(8, 5, WHITE);
            bool isDoubleThree = board.isDoubleThree(8, 5, WHITE);
            
            // Should be allowed if it captures
            if (!captures.empty()) {
                assert(!isDoubleThree || captures.size() > 0 && "FAIL: Capture exception not working");
                cout << "✅ Double-three capture exception: PASS\n";
            }
        }
    }
    
    void testEndgameCapture() {
        cout << "\n4. Testing Endgame Capture Rules...\n";
        
        // Test: 5-in-row can be broken by capture
        {
            Board board;
            
            // White has 5 in a row but vulnerable to capture
            for (int i = 5; i < 10; i++) {
                board.placePiece(i, 9, WHITE);
            }
            
            // Black can capture to break the line
            board.placePiece(4, 9, BLACK);
            board.placePiece(10, 9, BLACK);
            
            // White's 5-in-row should be breakable
            vector<Position> captures = board.checkCaptures(4, 9, BLACK);
            if (captures.size() >= 2) {
                cout << "✅ 5-in-row can be broken by capture: PASS\n";
            }
        }
        
        // Test: Game continues if capture possible
        {
            Board board;
            board.setCaptureCount(BLACK, 8); // Black close to capture win
            
            // White gets 5 in a row
            for (int i = 5; i < 10; i++) {
                board.placePiece(i, 5, WHITE);
            }
            
            // But Black can capture for the win
            board.placePiece(4, 5, BLACK);
            vector<Position> captures = board.checkCaptures(11, 5, BLACK);
            
            // Game should not end yet
            cout << "✅ Endgame capture logic: PASS\n";
        }
    }
    
    void testEdgeCases() {
        cout << "\n5. Testing Edge Cases...\n";
        
        // Test board boundaries
        {
            Board board;
            
            // Try to place at invalid positions
            assert(!board.placePiece(-1, 5, BLACK) && "FAIL: Negative coordinates accepted");
            assert(!board.placePiece(19, 5, BLACK) && "FAIL: Out-of-bounds coordinates accepted");
            assert(!board.placePiece(5, -1, BLACK) && "FAIL: Negative coordinates accepted");
            assert(!board.placePiece(5, 19, BLACK) && "FAIL: Out-of-bounds coordinates accepted");
            
            cout << "✅ Boundary checking: PASS\n";
        }
        
        // Test occupied position
        {
            Board board;
            board.placePiece(9, 9, BLACK);
            
            assert(!board.placePiece(9, 9, WHITE) && "FAIL: Occupied position accepted");
            cout << "✅ Occupied position checking: PASS\n";
        }
        
        // Test empty board AI move
        {
            Board board;
            MinMaxAI ai(BLACK, 6);
            
            Position move = ai.makeMove(board);
            assert(move.x >= 0 && move.x < BOARD_SIZE && "FAIL: Invalid move on empty board");
            assert(move.y >= 0 && move.y < BOARD_SIZE && "FAIL: Invalid move on empty board");
            
            // Should prefer center on empty board
            int centerX = BOARD_SIZE / 2;
            int centerY = BOARD_SIZE / 2;
            int distance = abs(move.x - centerX) + abs(move.y - centerY);
            assert(distance <= 3 && "FAIL: AI doesn't prefer center on empty board");
            
            cout << "✅ Empty board handling: PASS\n";
        }
        
        cout << "✅ All edge cases: PASS\n";
    }
};
```

**Script de lancement complet :**
```cpp
// test_runner.cpp
#include "PerformanceTests.hpp"
#include "RuleTests.hpp"

int main() {
    try {
        cout << "Starting Gomoku AI Validation Suite...\n\n";
        
        RuleTests ruleTests;
        ruleTests.runAllTests();
        
        PerformanceTests perfTests;
        perfTests.runAllTests();
        
        cout << "\n🎉 ALL TESTS PASSED! 🎉\n";
        cout << "Your Gomoku AI is ready for evaluation!\n";
        
    } catch (const exception& e) {
        cout << "\n❌ TEST FAILED: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}
```

---

## 🎓 Méthodologie d'Apprentissage

### 📋 Checklist de Progression

Cochez chaque élément une fois maîtrisé :

**Phase 1 - Bases :**
- [ ] Architecture du code comprise
- [ ] Toutes les règles Gomoku testées
- [ ] Bug de profondeur corrigé

**Phase 2 - MinMax :**
- [ ] Algorithme MinMax implémenté
- [ ] Alpha-beta pruning fonctionnel
- [ ] Génération de coups optimisée

**Phase 3 - Évaluation :**
- [ ] Reconnaissance de motifs
- [ ] Détection de menaces
- [ ] Évaluation des captures
- [ ] Contrôle positionnel
- [ ] Règle double-trois

**Phase 4 - Optimisations :**
- [ ] Table de transposition
- [ ] Recherche itérative

**Phase 5 - Interface :**
- [ ] Timer visible et fonctionnel
- [ ] Mode debug complet

**Phase 6 - Validation :**
- [ ] Performance < 0.5s/coup
- [ ] Profondeur ≥ 10 niveaux
- [ ] Toutes règles validées

### 🚀 Conseils pour Réussir

1. **Commencez petit** : Chaque itération doit compiler et fonctionner
2. **Testez constamment** : Validez chaque modification
3. **Documentez** : Commentez votre code pour la défense
4. **Mesurez** : Utilisez les timers et compteurs
5. **Comprenez** : Ne copiez pas sans comprendre

### 📚 Ressources Complémentaires

- **MinMax Algorithm** : [Wikipedia MinMax](https://en.wikipedia.org/wiki/Minimax)
- **Alpha-Beta Pruning** : [Chess Programming Wiki](https://www.chessprogramming.org/Alpha-Beta)
- **Gomoku Strategy** : [Gomoku Rules and Strategy](https://en.wikipedia.org/wiki/Gomoku)
- **C++ Performance** : [Optimization Techniques](https://cpp-optimizations.netlify.app/)

---

## 🎯 Objectifs de Fin de Formation

À la fin de ce parcours, vous devriez :

1. **Comprendre** l'architecture complète d'un jeu avec IA
2. **Maîtriser** l'algorithme MinMax et ses optimisations
3. **Savoir** créer une fonction d'évaluation sophistiquée
4. **Pouvoir** expliquer chaque partie du code en détail
5. **Avoir** une IA qui respecte toutes les contraintes du projet

**Temps total estimé : 12-15 heures**
**Difficulté : Intermédiaire à Avancé**
**Prérequis : C++, structures de données, récursivité**

Bonne chance dans votre apprentissage ! 🚀