# Gomoku - Jeu de Stratégie avec IA

Un jeu de Gomoku complet implémenté en Python avec Pygame, incluant une IA sophistiquée utilisant l'algorithme Minimax avec élagage Alpha-Beta.

## Fonctionnalités

### Règles du Jeu
- **Plateau 19x19** traditionnel
- **Victoire par alignement** : 5 pierres alignées ou plus
- **Règle de capture** : Capturer 10 pierres (5 paires) pour gagner
- **Capture en fin de partie** : Possibilité de briser un alignement de 5 par capture
- **Interdiction des double-trois** : Coup interdit créant deux alignements de trois libres

### Modes de Jeu
- **Humain vs Humain** : Partie classique à deux joueurs
- **Humain vs IA** : Trois niveaux de difficulté (Facile/Normal/Difficile)
- **Suggestions de coups** : L'IA peut suggérer des coups aux joueurs humains

### Intelligence Artificielle
- **Algorithme Minimax** avec élagage Alpha-Beta
- **Profondeur de recherche** : Minimum 10 niveaux (configurable)
- **Limite de temps** : Moins de 0.5 seconde par coup
- **Heuristique sophistiquée** : Évaluation des patterns, captures, positions
- **Optimisations** : Table de transposition, killer moves, tri des coups

### Interface Utilisateur
- **Interface graphique** complète avec Pygame
- **Timer visible** pour mesurer les performances de l'IA
- **Mode debug** pour visualiser le raisonnement de l'IA
- **Affichage des statistiques** : Coups évalués, temps de recherche
- **Suggestions visuelles** : Affichage des meilleurs coups

## Installation et Utilisation

### Prérequis
- Python 3.8 ou supérieur
- pip (gestionnaire de paquets Python)

### Installation
```bash
# Cloner le projet
git clone <repository_url>
cd gomoku

# Installer les dépendances et construire
make all

# Ou manuellement :
pip install -r requirements.txt
```

### Lancement
```bash
# Via le Makefile
make run

# Ou directement
./Gomoku

# Ou avec Python
cd src && python main.py
```

### Commandes Makefile
- `make all` - Construire l'exécutable
- `make install` - Installer les dépendances
- `make run` - Lancer le jeu
- `make test` - Tests de base
- `make clean` - Nettoyer les fichiers compilés
- `make fclean` - Suppression complète
- `make re` - Reconstruction complète

## Contrôles du Jeu

- **Clic gauche** : Placer une pierre
- **R** : Recommencer la partie
- **C** : Afficher/masquer les coordonnées
- **D** : Mode debug (visualisation IA)
- **S** : Suggestions de coups
- **ESC** : Quitter

## Architecture du Projet

```
gomoku/
├── src/
│   ├── game/           # Logique du jeu
│   │   ├── board.py    # Plateau 19x19
│   │   ├── rules.py    # Règles spéciales
│   │   ├── game.py     # Gestion des parties
│   │   └── player.py   # Classes des joueurs
│   ├── ai/             # Intelligence Artificielle
│   │   ├── minimax.py  # Algorithme Minimax
│   │   ├── heuristic.py # Fonction d'évaluation
│   │   └── ai_player.py # Joueur IA
│   ├── ui/             # Interface utilisateur
│   │   ├── game_ui.py  # Interface principale
│   │   ├── timer.py    # Timer pour l'IA
│   │   └── debug_ui.py # Interface de debug
│   └── main.py         # Point d'entrée
├── Makefile           # Système de build
└── requirements.txt   # Dépendances
```

## Performance de l'IA

L'IA respecte les contraintes du projet :
- **Temps de réflexion** : < 0.5 seconde en moyenne
- **Profondeur de recherche** : ≥ 10 niveaux
- **Optimisations** : Recherche itérative, élagage efficace
- **Heuristique rapide** : Évaluation précise des positions

## Développement et Debug

Le mode debug permet de :
- Visualiser les statistiques de recherche
- Afficher les meilleurs coups envisagés
- Comprendre le raisonnement de l'IA
- Analyser les performances en temps réel

## Conformité au Sujet

Ce projet respecte toutes les exigences :
- ✅ Exécutable nommé "Gomoku"
- ✅ Règles complètes (alignements, captures, double-trois)
- ✅ IA Minimax profondeur ≥ 10
- ✅ Temps de réflexion < 0.5s
- ✅ Timer visible obligatoire
- ✅ Interface graphique utilisable
- ✅ Mode contre IA et entre humains
- ✅ Makefile avec règles standard
- ✅ Pas de crash, gestion robuste des erreurs