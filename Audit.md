# Audit plan and first pass results

I read the subject in `subject/` and reviewed the codebase to compare what’s required vs what is implemented. Auditing by header/implementation pairs is the right approach; I used it to get a precise picture.

## Subject highlights (constraints we must satisfy)
- Board: 19x19 goban with captures.
- Win: 5 or more in a row OR capture 10 stones.
- Endgame capture rule: a 5+ alignment wins only if the opponent cannot immediately break it by capturing a pair.
- Double-threes forbidden.
- Play modes:
  - Human vs AI (AI adapts)
  - Human vs Human (hotseat) with move-suggestion feature
- Minimax AI:
  - Search depth at least 10
  - Average move time under 0.5s
- UI must display an AI timer (No timer, no validation).

## Findings by module/pair

- GameRenderer.hpp ↔ GameRenderer.cpp
  - GameRenderer is a full modern board/UI renderer.
  - We updated grid to goban-style (edge-to-edge lines) + hoshi points; click mapping now snaps to intersections.
  - The class declares many helpers and features that are not implemented anywhere:
    - `renderMoveHistory`, `renderValidMoveHints`, `renderCaptureEffects`, `renderWinConditionHighlight`
    - `drawStoneWithAnimation`, `processPlaceAnimation`, `processCaptureAnimation`, `processHighlightAnimation`
    - `highlightCaptures`, `clearHighlights`
  - These declarations are unused and unimplemented. They create noise and the potential for confusion.
  - Suggestion feature: Subject requires a “move suggestion” in hotseat. We don’t currently implement this. We should either:
    - Implement “suggestion” (compute AI best move and draw a marker), or
    - Remove the declarations that suggest features we don’t deliver yet and implement only the suggestion feature.

- Game.hpp ↔ Game.cpp
  - Window is now non-resizable, which stabilizes cursor math. Good decision.
  - Input mapping was unified by delegating to [GameRenderer::getBoardPosition()](cci:1://file:///home/jgiampor/Desktop/Gomoku/src/Game/Game.cpp:567:0-569:1); precision issues are resolved.
  - Rendering: [Game::renderBoard()](cci:1://file:///home/jgiampor/Desktop/Gomoku/src/Game/Game.cpp:494:0-508:1) still calls [board.draw(window)](cci:1://file:///home/jgiampor/Desktop/Gomoku/src/UI/GameRenderer.cpp:120:0-131:1) and [board.drawCaptureCount(window, font)](cci:1://file:///home/jgiampor/Desktop/Gomoku/src/Game/Board.cpp:374:0-386:1), i.e., it uses the legacy Board renderer rather than the [GameRenderer](cci:1://file:///home/jgiampor/Desktop/Gomoku/include/UI/GameRenderer.hpp:154:4-154:19) pipeline. This duplicates/contradicts the modern rendering done in [GameRenderer](cci:1://file:///home/jgiampor/Desktop/Gomoku/include/UI/GameRenderer.hpp:154:4-154:19).
    - We should consolidate on one rendering system (GameRenderer) and remove/retire [Board::draw()](cci:1://file:///home/jgiampor/Desktop/Gomoku/src/UI/GameRenderer.cpp:120:0-131:1) from the gameplay loop.

- Board.hpp ↔ Board.cpp
  - Implements rules scaffolding, capture checks, double-three, etc.
  - CRITICAL: [Board::executeCaptures](cci:1://file:///home/jgiampor/Desktop/Gomoku/src/Game/Game.cpp:322:0-330:1) currently FLIPS the captured stones’ colors instead of REMOVING the stones from the board:
    - Code:
      - When capturing WHITE: set to BLACK
      - When capturing BLACK: set to WHITE
    - Subject requires removals: “Blue captures the red pair and removes the stones from the game. The now-free intersections can be played on...”
    - This must be reverted to removal (set to EMPTY). It’s non-compliant.
  - [Board::draw()](cci:1://file:///home/jgiampor/Desktop/Gomoku/src/UI/GameRenderer.cpp:120:0-131:1) and [drawCaptureCount()](cci:1://file:///home/jgiampor/Desktop/Gomoku/src/Game/Board.cpp:374:0-386:1) are a complete legacy renderer with fixed offsets (50,50) and `CELL_SIZE`; this contradicts the new [GameRenderer](cci:1://file:///home/jgiampor/Desktop/Gomoku/include/UI/GameRenderer.hpp:154:4-154:19) layout. These should be removed from the main path and eventually deleted (or kept only for testing, but not used).

- Rules.hpp ↔ Rules.cpp
  - Provides checks for: valid move with double-three rule, win conditions, capture detection helpers, draw, threats, etc.
  - Endgame capture rule (alignment can be broken by immediate capture): I do not see this enforced where a win is decided.
    - [Board::checkWin](cci:1://file:///home/jgiampor/Desktop/Gomoku/src/Game/Board.cpp:80:0-83:1) returns capture win or alignment win straight-away ([checkCaptureWin(player) || checkAlignment(...)](cci:1://file:///home/jgiampor/Desktop/Gomoku/src/Game/Board.cpp:110:0-112:1)), without evaluating the endgame capture exception.
    - Subject says: the player wins by alignment only if the opponent cannot break this line by capturing a pair.
    - We need to integrate this logic: when detecting an alignment, verify opponent lacks an immediate capture that would break it. We can do this at the point where a win is checked (Game-level or Board/Rules-level), likely in [Rules::hasAlignmentWin](cci:1://file:///home/jgiampor/Desktop/Gomoku/src/Game/Rules.cpp:36:0-73:1) or in the win resolution flow in Game.
  - Double-three logic exists (Board delegates to its own helpers). Might be simplistic, but at least present.

- MinMaxAI.hpp ↔ MinMaxAI.cpp
  - Defaults are aligned with subject: `depth = 10`, `timeLimitSec = 0.5`.
  - Good: Has heuristic, TT, killer moves, iterative deepening scaffolding.
  - Ensure at runtime we actually use depth >= 10 and keep under ~0.5s average. This is a grading criteria.

- UI/Timer.hpp ↔ Timer.cpp and DebugUI
  - AI Timer exists and displays in UI. This is required by the subject.
  - DebugUI is present. Optional and helpful.

## Non-compliances and risks
- Non-compliance: Captures flipping instead of removing in [Board::executeCaptures()](cci:1://file:///home/jgiampor/Desktop/Gomoku/src/Game/Game.cpp:322:0-330:1). Must set captured stones to `EMPTY`.
- Missing: Endgame capture exception for alignment wins. Alignments should not auto-win if opponent has an immediate capture to break the line.
- Missing: Hotseat “move suggestion” feature. Subject marks it as mandatory.

## Inconsistencies / unused or misleading code
- Legacy rendering vs modern rendering:
  - [Game::renderBoard()](cci:1://file:///home/jgiampor/Desktop/Gomoku/src/Game/Game.cpp:494:0-508:1) uses [Board::draw()](cci:1://file:///home/jgiampor/Desktop/Gomoku/src/UI/GameRenderer.cpp:120:0-131:1), while we also maintain [GameRenderer](cci:1://file:///home/jgiampor/Desktop/Gomoku/include/UI/GameRenderer.hpp:154:4-154:19). This is a source of bugs/inconsistency. Consolidate on [GameRenderer](cci:1://file:///home/jgiampor/Desktop/Gomoku/include/UI/GameRenderer.hpp:154:4-154:19) and stop calling [Board::draw()](cci:1://file:///home/jgiampor/Desktop/Gomoku/src/UI/GameRenderer.cpp:120:0-131:1).
- [GameRenderer.hpp](cci:7://file:///home/jgiampor/Desktop/Gomoku/include/UI/GameRenderer.hpp:0:0-0:0) contains many declared but unimplemented and unused methods. Either implement them or remove them to declutter. Given the subject, we should keep only what we need and add a clean “suggestion” feature.

## Recommended actions (ordered)
1) Compliance fixes first:
   - Remove flipping capture; set captured stones to `EMPTY` in [Board::executeCaptures()](cci:1://file:///home/jgiampor/Desktop/Gomoku/src/Game/Game.cpp:322:0-330:1).
   - Implement endgame capture rule:
     - When an alignment (>=5) is found, check if the opponent has an immediate capturing move that would break it. If yes, defer the win; otherwise, grant alignment win.
     - This is best enforced in one place (e.g., [Rules::hasAlignmentWin](cci:1://file:///home/jgiampor/Desktop/Gomoku/src/Game/Rules.cpp:36:0-73:1) enhanced with “cannot be broken by opponent capture” check), or as a compound check in [Game](cci:1://file:///home/jgiampor/Desktop/Gomoku/src/Game/Game.cpp:26:0-70:1) when resolving a move.
2) Rendering consolidation:
   - Stop calling [board.draw(window)](cci:1://file:///home/jgiampor/Desktop/Gomoku/src/UI/GameRenderer.cpp:120:0-131:1) and [board.drawCaptureCount](cci:1://file:///home/jgiampor/Desktop/Gomoku/src/Game/Board.cpp:374:0-386:1) in [Game::renderBoard()](cci:1://file:///home/jgiampor/Desktop/Gomoku/src/Game/Game.cpp:494:0-508:1).
   - Use [GameRenderer](cci:1://file:///home/jgiampor/Desktop/Gomoku/include/UI/GameRenderer.hpp:154:4-154:19) exclusively for rendering (we already have a pipeline rendering the board, hoshi, stones, panels, timer).
   - If you want capture counts, draw them via [GameRenderer](cci:1://file:///home/jgiampor/Desktop/Gomoku/include/UI/GameRenderer.hpp:154:4-154:19) in its side panel rather than [Board::drawCaptureCount](cci:1://file:///home/jgiampor/Desktop/Gomoku/src/Game/Board.cpp:374:0-386:1).
   - Mark legacy board rendering as deprecated and then remove it once unused.
3) Subject feature gap:
   - Implement “move suggestion” for hotseat:
     - When in Human vs Human, on a keypress (e.g., `S`) or as a toggle, run the AI (shallow depth or time-limited) from the human side and display a visual hint (marker/circle) on the suggested intersection using [GameRenderer](cci:1://file:///home/jgiampor/Desktop/Gomoku/include/UI/GameRenderer.hpp:154:4-154:19).
     - Add a clean API like `renderer->showSuggestion(Position)` and `renderer->clearSuggestion()`.
4) Cleanup unused/unimplemented declarations:
   - In [GameRenderer.hpp](cci:7://file:///home/jgiampor/Desktop/Gomoku/include/UI/GameRenderer.hpp:0:0-0:0), remove unused method declarations or implement them if they’re part of your planned features. For now, keep the API tight and aligned with the subject.
5) Hardening:
   - Enable strict warnings in the Makefile:
     - `-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Wmissing-declarations -Wredundant-decls -Wunused`
   - Consider `clang-tidy`/`cppcheck` for a pass to detect dead code/inconsistencies.

## Proposed next steps
- I can implement immediately:
  - Fix capture to remove stones (Subject-critical).
  - Remove legacy [Board::draw()](cci:1://file:///home/jgiampor/Desktop/Gomoku/src/UI/GameRenderer.cpp:120:0-131:1) usage and switch to [GameRenderer](cci:1://file:///home/jgiampor/Desktop/Gomoku/include/UI/GameRenderer.hpp:154:4-154:19) only (code cleanup).
- Then:
  - Add endgame capture exception check in `Rules`/[Game](cci:1://file:///home/jgiampor/Desktop/Gomoku/src/Game/Game.cpp:26:0-70:1).
  - Implement the “move suggestion” feature in hotseat mode.

Do you want me to:
- Apply the capture fix and consolidate rendering now,
- Or first implement the endgame capture exception,
- Or prioritize the hotseat suggestion feature?



Next suggested cleanup steps:

Remove any remaining references to the old 
Board
 rendering API (there should be none now).
Optionally, remove now-unused includes in 
Board.cpp
 (e.g., <iostream> if it becomes unnecessary) and any SFML UI includes that were only needed for the legacy draw methods.
Implement capture counts in 
GameRenderer
’s side panel if you want them displayed (replacing Board::drawCaptureCount()).
Would you like me to:

Add capture counters to the 
GameRenderer
 UI panel?
Continue pruning unused includes and run a warnings pass to ensure no dead references?