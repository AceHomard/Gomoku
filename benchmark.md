# Build
make benchmark

# Lancer (quiet les logs verbeux)
./benchmark -n 5 -q

# Options
./benchmark -n 10              # 10 parties
./benchmark --noise 30         # Plus de variété (défaut: 15)
./benchmark --noise 0          # Déterministe (même partie répétée)
./benchmark -h                 # Aide
