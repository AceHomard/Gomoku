# Gomoku - Guide pour Claude Code

Ce document décrit les conventions et workflows pour travailler sur ce projet avec Claude Code.

## Documentation Requirements

**All technical decisions and implementations must be documented** in the Obsidian vault `[DOC]-Gomoku/` following the conventions below.

### When to Document

| Action | Document Type | Location |
|--------|---------------|----------|
| New architecture decision | `ADR-XXX-Title.md` | `06-ADR/` |
| New feature implementation | `FEAT-XXX-Title.md` | `04-Features/` |
| API endpoint changes | `API-Module.md` | `05-API/` |
| Database schema changes | `DB-Domain.md` | `02-Database/` |
| Infrastructure changes | `ARCH-Component.md` | `03-Architecture/` |
| Meeting notes | `YYYY-MM-DD-Title.md` | `07-Meetings/` |
| Development notes | `DEV-Title.md` | `08-Dev/` |

### Naming Conventions

- **Prefixes**: `MOC-`, `CDC-`, `SPEC-`, `ARCH-`, `DB-`, `FEAT-`, `API-`, `ADR-`, `DEV-`, `TPL-`
- **Numbering**: 3-digit format (`FEAT-001`, `ADR-002`)
- **Dates**: `YYYY-MM-DD` in frontmatter and meeting filenames

### Required Frontmatter

```yaml
---
title: Document title
type: [moc|spec|arch|feature|api|adr|meeting|dev|database]
status: [draft|review|approved|deprecated]
created: YYYY-MM-DD
updated: YYYY-MM-DD
tags:
  - relevant-tags
---
```

### Vault Structure

```
[DOC]-Gomoku/
├── 00-MOC/          # Maps of Content (indexes)
├── 01-Specs/        # CDC and specifications
├── 02-Database/     # Database documentation
├── 03-Architecture/ # Technical architecture docs
├── 04-Features/     # Feature specifications
├── 05-API/          # API documentation
├── 06-ADR/          # Architecture Decision Records
├── 07-Meetings/     # Meeting notes
├── 08-Dev/          # Development guides
├── 09-Resources/    # External resources
├── 10-Archives/     # Deprecated documents
└── _Templates/      # Obsidian templates
```

Templates are available in `_Templates/` for each document type.

---

## Feature Workflow Skills

Ce projet utilise la **Feature Workflow Suite** pour implémenter les features de manière structurée. Ces skills orchestrent le cycle complet : specification → research → plan → implement → test → fix.

### Skills Disponibles

| Skill | Usage | Output |
|-------|-------|--------|
| `feature-specification` | Spécification interactive avec questionnement approfondi | `CDC-XXX.md` ou `FEAT-XXX.md` |
| `feature-research` | Recherche et analyse avant implémentation | `FEAT-XXX-Findings.md` |
| `implementation-planner` | Création du plan d'implémentation détaillé | `FEAT-XXX-Plan.md` |
| `feature-implementer` | Implémentation du code selon le plan | Code + `FEAT-XXX-Test-Plan.md` |
| `test-plan-generator` | Génération de plan de tests intelligent | `FEAT-XXX-Test-Plan.md` |
| `test-executor` | Exécution des tests | `FEAT-XXX-Test-Results.md` |
| `test-fixer` | Correction des tests échoués | Fixes + re-tests |
| `git-workflow-manager` | Gestion des git worktrees | Branches feature/fix/hotfix |
| `workflow-challenger` | Review critique et analyse de gaps | Questions et recommandations |
| `feature-workflow` | Orchestration complète (tous les skills) | Tous les outputs |

### Quand Utiliser Chaque Skill

```
1. Nouvelle feature (requirements flous) → feature-specification
2. Requirements clairs → feature-research
3. Research déjà faite → implementation-planner
4. Plan déjà fait → feature-implementer
5. Tests à créer → test-plan-generator
6. Tests échouent → test-fixer
7. Besoin de comprendre → feature-research seul
8. Review critique → workflow-challenger
9. Full workflow automatique → feature-workflow
```

### Convention de Stockage (Archives)

Tous les artefacts du workflow sont stockés dans `[DOC]-Gomoku/10-Archives/` :

```
10-Archives/
└── FEAT-XXX-NomFeature/
    ├── FEAT-XXX-Findings.md      # Research findings
    ├── FEAT-XXX-Plan.md          # Implementation plan
    ├── FEAT-XXX-Test-Plan.md     # Test plan generated
    ├── FEAT-XXX-Test-Results.md  # Test execution results
    └── FEAT-XXX-Test-Fixes.md    # Fix documentation (si applicable)
```

### Workflow Typique

```bash
# 1. Specification (si requirements flous)
# Invoke skill: feature-specification
# → Crée: 01-Specs/CDC-001.md ou 04-Features/FEAT-001.md

# 2. Research (manuel ou avec skill)
# Invoke skill: feature-research
# → Crée: 10-Archives/FEAT-001-Auth/FEAT-001-Findings.md

# 3. Planning
# Invoke skill: implementation-planner
# → Crée: 10-Archives/FEAT-001-Auth/FEAT-001-Plan.md

# 4. Implementation
# Invoke skill: feature-implementer
# → Implémente le code + crée test plan

# 5. Test & Fix (itératif)
# Invoke skill: test-executor puis test-fixer si échecs

# 6. Review critique (optionnel)
# Invoke skill: workflow-challenger
# → Analyse et challenge les décisions
```

### Frontmatter pour Artefacts Workflow

```yaml
---
title: FEAT-XXX Feature Name - [Findings|Plan|Test-Plan]
type: research|plan|test
status: [in-progress|completed]
created: YYYY-MM-DD
updated: YYYY-MM-DD
feature: FEAT-XXX
tags:
  - feature-workflow
  - [research|planning|testing]
---
```

---

## Hiérarchie des Sources de Vérité

**Ordre de priorité pour la consultation de la documentation (du plus important au moins important) :**

| Rang | Source | Description | Localisation |
|------|--------|-------------|--------------|
| 1️⃣ | **CDC** | Cahier des Charges - Source ultime | `01-Specs/CDC-*.md` |
| 2️⃣ | **DB-*.md** | Schémas de base de données | `02-Database/` |
| 3️⃣ | **Meetings** | Notes de réunions et décisions | `07-Meetings/` |
| 4️⃣ | **ADR-*.md** | Architecture Decision Records | `06-ADR/` |
| 5️⃣ | **FEAT-*.md** | Spécifications features | `04-Features/` |

**Règles :**
- En cas de conflit entre sources, la source de rang supérieur prévaut
- Les exemples de code dans les FEAT sont **illustratifs** et peuvent être obsolètes
- Toujours vérifier les schémas `DB-*.md` pour les entités exactes
- Le CDC définit le "quoi", les DB définissent le "comment" en base

---

## RÈGLE CRITIQUE : Suivre la Documentation Obsidian

**La documentation Obsidian `[DOC]-Gomoku/` est la source de vérité.** À chaque étape du workflow, il est IMPÉRATIF de :

### 1. LIRE la documentation existante AVANT toute action

- **Database** : `02-Database/DB-*.md` (schémas des tables)
- **Features** : `04-Features/FEAT-*.md` (spécifications)
- **Architecture** : `03-Architecture/ARCH-*.md`
- **ADRs** : `06-ADR/ADR-*.md` (décisions techniques)
- **CDC** : `01-Specs/CDC-*.md` (cahier des charges)

### 2. SUIVRE RIGOUREUSEMENT les schémas documentés

- Les entités doivent correspondre EXACTEMENT aux tables documentées
- Les colonnes, types, contraintes sont déjà définis
- Ne PAS inventer de champs non documentés

### 3. SIGNALER les manques dans la documentation

- Si une information est manquante → **demander à l'utilisateur**
- Si un schéma est incomplet → le signaler AVANT d'implémenter
- Ne JAMAIS inventer ou supposer

### 4. Documents de référence par étape workflow

| Étape | Documents à consulter OBLIGATOIREMENT |
|-------|--------------------------------------|
| **Specification** | `CDC-*.md` existants, `FEAT-*.md` similaires |
| **Research** | `FEAT-XXX.md`, `ADR-*.md`, `DB-*.md` concernés |
| **Planning** | Findings + tous les `DB-*.md` impactés |
| **Implementation** | Plan + `DB-*.md` pour entités exactes |
| **Testing** | `FEAT-XXX.md` pour critères d'acceptation |

**Exemple correct :**
```
Avant de créer User.cs :
1. Lire 02-Database/DB-Users.md
2. Copier EXACTEMENT les colonnes documentées
3. Si colonne manquante → demander à l'utilisateur
```

**Exemple INCORRECT :**
```
Créer User.cs avec des champs inventés
sans vérifier DB-Users.md
```

### 5. METTRE À JOUR la documentation APRÈS implémentation

À la fin de chaque phase, relire la documentation concernée.

Si des modifications ont été nécessaires pendant l'implémentation :
- Mettre à jour `DB-*.md` si le schéma a changé
- Mettre à jour `FEAT-*.md` si les spécifications ont évolué
- Mettre à jour `API-*.md` si les endpoints ont changé
- Créer un `ADR-*.md` si une décision architecturale a été prise
- Mettre à jour le champ `updated` du frontmatter
- Signaler les changements à l'utilisateur

**Cycle complet Documentation ↔ Code :**
```
┌─────────────────────────────────────────────────────────┐
│                                                         │
│   1. LIRE la doc     →   2. IMPLÉMENTER selon doc       │
│         ▲                         │                     │
│         │                         ▼                     │
│   4. METTRE À JOUR   ←   3. NOTER les écarts/ajouts     │
│      la doc si besoin                                   │
│                                                         │
└─────────────────────────────────────────────────────────┘
```

---

## Commandes Disponibles

### `/doc-manager`
Génère automatiquement la documentation Obsidian depuis la conversation actuelle.

**Fonctionnalités:**
- Mode incrémental (traite seulement les nouveaux messages)
- Documentation 100% en français
- Détection automatique des patterns (ADR, FEAT, DEV, etc.)
- Génération de rapports de session

**Utilisation:**
```
# Après une conversation de travail
/doc-manager
```

**Fichiers générés:**
- `.claude/DOC-GENERATION-REPORT-SESSION-X.md` - Rapport de génération
- `.claude/cache/doc-manager/metadata.json` - Metadata pour mode incrémental
- `[DOC]-Gomoku/XX-Category/DOC-XXX.md` - Documents générés

---

## Output Styles

Des styles de sortie personnalisés sont disponibles dans `.claude/output-styles/`:

- **non-dev-explanatory.md** - Explications techniques pour non-développeurs

Pour utiliser un style, référencez-le dans vos conversations avec Claude Code.

---

## Git Workflow

Si vous utilisez `git-workflow-manager` skill:

- **feature/** - Nouvelles fonctionnalités
- **fix/** - Corrections de bugs
- **hotfix/** - Corrections urgentes en production

Le skill gère automatiquement les worktrees Git pour développement parallèle.

---

## Ressources

- Documentation Obsidian: `[DOC]-Gomoku/`
- Templates: `[DOC]-Gomoku/_Templates/`
- Archives workflow: `[DOC]-Gomoku/10-Archives/`
- Cache doc-manager: `.claude/cache/doc-manager/`

---

**Dernière mise à jour**: 2026-03-11
**Généré par**: clai v1.0.4
