#include "Game/Board.hpp"
#include "Debug.hpp"
#include <string>
#include <iostream>

// Build a directional line centered on (x,y) over k in [-5..5]
static std::string buildDirectionalLine(const Board* board, int x, int y, int dx, int dy, CellState player) {
    std::string line;
    line.reserve(11);
    for (int k = -5; k <= 5; ++k) {
        int nx = x + k * dx;
        int ny = y + k * dy;
        if (!board->isValidPosition(nx, ny)) {
            line.push_back('B'); // blocker (out of bounds)
        } else if (k == 0) {
            line.push_back('P'); // simulated placement is the player
        } else {
            CellState c = board->getCell(nx, ny);
            line.push_back(c == EMPTY ? 'E' : (c == player ? 'P' : 'B'));
        }
    }
    return line;
}

// Return true if substring contains an open four pattern EPPPP E and center is inside
static bool containsOpenFour(const std::string& s, int centerIdx) {
    const int n = (int)s.size();
    for (int i = 0; i + 6 <= n; ++i) {
        if (s.compare(i, 6, "EPPPPE") == 0) {
            if (centerIdx >= i && centerIdx < i + 6) return true;
        }
    }
    return false;
}

// Return true if line contains an open-three pattern that includes the center index
static bool matchesOpenThree(const std::string& line, int centerIdx) {
    const int n = (int)line.size();
    // Direct open-three patterns
    for (int i = 0; i + 5 <= n; ++i) {
        if (line.compare(i, 5, "EPPPE") == 0) {
            if (centerIdx >= i && centerIdx < i + 5) return true;
        }
    }
    for (int i = 0; i + 6 <= n; ++i) {
        if (line.compare(i, 6, "EP.PPE") == 0 || line.compare(i, 6, "EPP.PE") == 0) {
            if (centerIdx >= i && centerIdx < i + 6) return true;
        }
    }

    // Threat-based: can we play one more P to obtain an open four EPPPPE including the center?
    // Try replacing each E with P and test for EPPPPE
    for (int j = 0; j < n; ++j) {
        if (line[j] != 'E') continue;
        std::string tmp = line;
        tmp[j] = 'P';
        if (containsOpenFour(tmp, centerIdx)) return true;
    }
    return false;
}

Board::Board(int boardSize) : size(boardSize), blackCaptures(0), whiteCaptures(0) {
    grid.resize(size, std::vector<CellState>(size, EMPTY));
}

Board::~Board() {
}

void Board::clear() {
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            grid[i][j] = EMPTY;
        }
    }
    blackCaptures = 0;
    whiteCaptures = 0;
}

bool Board::placePiece(int x, int y, CellState player) {
    if (!isValidMove(x, y)) {
        return false;
    }
    // NOTE: Double-three rule is now checked in Rules::isValidMove() with capture exception
    // No need to check again here

    // Place the stone
    grid[x][y] = player;

    // Check and execute captures after placing
    std::vector<Position> captures = checkCaptures(x, y, player);
    if (!captures.empty()) {
        int capturedCount = executeCaptures(captures);
        if (player == BLACK) {
            blackCaptures += capturedCount;
        } else {
            whiteCaptures += capturedCount;
        }
    }
    
    return true;
}

MoveUndo Board::makeMove(int x, int y, CellState player) {
    MoveUndo undo;
    undo.x = x;
    undo.y = y;
    undo.player = player;
    undo.capturedColor = getOpponent(player);
    undo.capturedCount = 0;

    // Place the stone
    grid[x][y] = player;

    // Check and execute captures
    std::vector<Position> captures = checkCaptures(x, y, player);
    if (!captures.empty()) {
        undo.capturedStones = captures;
        undo.capturedCount = captures.size();
        for (const Position& pos : captures) {
            grid[pos.x][pos.y] = EMPTY;
        }
        if (player == BLACK) {
            blackCaptures += undo.capturedCount;
        } else {
            whiteCaptures += undo.capturedCount;
        }
    }

    return undo;
}

void Board::unmakeMove(const MoveUndo& undo) {
    // Remove the placed stone
    grid[undo.x][undo.y] = EMPTY;

    // Restore captured stones
    for (const Position& pos : undo.capturedStones) {
        grid[pos.x][pos.y] = undo.capturedColor;
    }

    // Restore capture counter
    if (undo.capturedCount > 0) {
        if (undo.player == BLACK) {
            blackCaptures -= undo.capturedCount;
        } else {
            whiteCaptures -= undo.capturedCount;
        }
    }
}

CellState Board::getCell(int x, int y) const {
    if (!isValidPosition(x, y)) {
        return EMPTY;
    }
    return grid[x][y];
}

void Board::setCell(int x, int y, CellState state) {
    if (isValidPosition(x, y)) {
        grid[x][y] = state;
    }
}

bool Board::isValidMove(int x, int y) const {
    return isValidPosition(x, y) && grid[x][y] == EMPTY;
}

bool Board::isValidPosition(int x, int y) const {
    return x >= 0 && x < size && y >= 0 && y < size;
}

CellState Board::getOpponent(CellState player) const {
    if (player == BLACK) return WHITE;
    if (player == WHITE) return BLACK;
    return EMPTY;
}

// Win condition checking
bool Board::checkWin(CellState player) const {
    return checkCaptureWin(player) || checkAlignment(-1, -1, player);
}

bool Board::checkAlignment(int x, int y, CellState player) const {
    // If specific position given, check from that position
    if (x >= 0 && y >= 0) {
        return checkDirectionAlignment(x, y, 1, 0, player) ||  // horizontal
               checkDirectionAlignment(x, y, 0, 1, player) ||  // vertical
               checkDirectionAlignment(x, y, 1, 1, player) ||  // diagonal
               checkDirectionAlignment(x, y, 1, -1, player);   // anti-diagonal
    }
    
    // Check entire board for any alignment
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            if (grid[i][j] == player) {
                if (checkDirectionAlignment(i, j, 1, 0, player) ||
                    checkDirectionAlignment(i, j, 0, 1, player) ||
                    checkDirectionAlignment(i, j, 1, 1, player) ||
                    checkDirectionAlignment(i, j, 1, -1, player)) {
                    return true;
                }
            }
        }
    }
    return false;
}

bool Board::checkCaptureWin(CellState player) const {
    return getCaptureCount(player) >= 10;
}

int Board::getCaptureCount(CellState player) const {
    return (player == BLACK) ? blackCaptures : whiteCaptures;
}

void Board::setCaptureCount(CellState player, int count) {
    if (player == BLACK) {
        blackCaptures = count;
    } else if (player == WHITE) {
        whiteCaptures = count;
    }
}

// Capture system
std::vector<Position> Board::checkCaptures(int x, int y, CellState player) const {
    std::vector<Position> allCaptures;
    
    // Check all four directions
    int directions[4][2] = {{1, 0}, {0, 1}, {1, 1}, {1, -1}};
    
    for (int i = 0; i < 4; i++) {
        int dx = directions[i][0];
        int dy = directions[i][1];
        
        // Check both directions along this axis
        std::vector<Position> captures1 = checkCaptureDirection(x, y, dx, dy, player);
        std::vector<Position> captures2 = checkCaptureDirection(x, y, -dx, -dy, player);
        
        allCaptures.insert(allCaptures.end(), captures1.begin(), captures1.end());
        allCaptures.insert(allCaptures.end(), captures2.begin(), captures2.end());
    }
    
    return allCaptures;
}

std::vector<Position> Board::checkCaptureDirection(int x, int y, int dx, int dy, CellState player) const {
    std::vector<Position> captures;
    CellState opponent = getOpponent(player);

    // CAPTURE RULE: Playing at (x,y) completes pattern X-O-O-X
    // We capture the 2 opponent stones in the middle
    // Pattern positions: (x,y) | +1 | +2 | +3
    //                     X(new)| O  | O  | X(existing)
    //
    // Subject rule (appendix line 7): "One can only capture PAIRS, not single stones,
    // and not more than 2 stones in a row"
    //
    // This means: If playing at (x,y) completes X-O-O-X, we capture ONLY if:
    // 1. Positions +1 and +2 are opponent stones
    // 2. Position +3 is our stone
    // 3. There is NO 3rd opponent stone continuing the chain
    //
    // We DON'T care what's BEFORE position (x,y)

    if (isValidPosition(x + dx, y + dy) &&
        isValidPosition(x + 2*dx, y + 2*dy) &&
        isValidPosition(x + 3*dx, y + 3*dy)) {

        if (grid[x + dx][y + dy] == opponent &&
            grid[x + 2*dx][y + 2*dy] == opponent &&
            grid[x + 3*dx][y + 3*dy] == player) {

            // Check for exactly 2 opponent stones (not 3+)
            // We need to verify there's no opponent stone between +2 and +3
            // But wait - +3 is already our player stone!
            // So we can't have an opponent between +2 and +3.
            //
            // The issue is: what if the pattern is X-O-O-O-O-X?
            // In this case, when we play at position 0, we have:
            // +1=O, +2=O, +3=O (NOT player!) - so this won't match anyway.
            //
            // So actually, the basic pattern check ALREADY ensures exactly 2!
            // Because +3 MUST be player, there can't be 3 consecutive opponents.

            LOG_DEBUG("[CaptureCheck] Found valid capture at (" << x << "," << y
                      << ") dir=(" << dx << "," << dy << ") capturing ("
                      << (x+dx) << "," << (y+dy) << ") and ("
                      << (x+2*dx) << "," << (y+2*dy) << ")");
            captures.push_back(Position(x + dx, y + dy));
            captures.push_back(Position(x + 2*dx, y + 2*dy));
        }
    }

    return captures;
}

int Board::executeCaptures(const std::vector<Position>& captures) {
    // Verbose logging for captures (useful during debugging/analysis)
    if (!captures.empty()) {
        LOG_DEBUG("[Capture] Removing " << captures.size() << " stones");
    }

    for (const Position& pos : captures) {
        if (isValidPosition(pos.x, pos.y)) {
            grid[pos.x][pos.y] = EMPTY;
        }
    }
    return captures.size();
}

// Double-three rule
bool Board::isDoubleThree(int x, int y, CellState player) const {
    std::vector<Position> freeThrees = findFreeThrees(x, y, player);
    return freeThrees.size() >= 2;
}

std::vector<Position> Board::findFreeThrees(int x, int y, CellState player) const {
    std::vector<Position> freeThreeDirections;
    
    // Temporarily place the piece
    const_cast<Board*>(this)->grid[x][y] = player;
    
    // Check all four directions for free-threes
    int directions[4][2] = {{1, 0}, {0, 1}, {1, 1}, {1, -1}};
    
    for (int i = 0; i < 4; i++) {
        int dx = directions[i][0];
        int dy = directions[i][1];
        
        // Si le placement crée déjà un alignement de 4 ou plus dans cette direction,
        // ce n'est pas un "trois libre" et ne doit pas compter pour un double-trois
        int contiguous = countConsecutive(x, y, dx, dy, player);
        if (contiguous >= 4) {
            continue; // Skip this direction
        }
        
        if (isFreeThree(x, y, dx, dy, player)) {
            // Store the direction vector instead of just the position
            freeThreeDirections.push_back(Position(dx, dy));
        }
    }
    
    // Remove the temporary piece
    const_cast<Board*>(this)->grid[x][y] = EMPTY;
    
    return freeThreeDirections;
}

bool Board::isFreeThree(int x, int y, int dx, int dy, CellState player) const {
    // Un "trois libre" peut être :
    // 1. Exactement 3 pierres consécutives avec extrémités libres (_XXX_)
    // 2. Un pattern avec gap qui forme 3 pierres après placement (_X_XX_, _XX_X_)
    
    // Simuler le placement de la pierre
    const_cast<Board*>(this)->grid[x][y] = player;
    
    // Utiliser l'ancienne logique robuste qui gère tous les patterns
    bool result = canCreateUnstoppableFour(x, y, dx, dy, player);
    
    // Mais ajouter une vérification supplémentaire : on ne veut que les "trois libres"
    // pas les alignements de 4 ou plus
    if (result) {
        int consecutive = countConsecutive(x, y, dx, dy, player);
        if (consecutive >= 4) {
            result = false; // Ce n'est pas un "trois libre" mais un alignement plus long
        }
    }
    
    // Retirer la pierre temporaire
    const_cast<Board*>(this)->grid[x][y] = EMPTY;
    
    return result;
}

bool Board::canFormUnstoppableFour(int x, int y, int dx, int dy, CellState player) const {
    // Find the bounds of the three consecutive stones
    int start = 0, end = 0;
    
    // Find start of sequence
    while (isValidPosition(x + (start-1)*dx, y + (start-1)*dy) && 
           grid[x + (start-1)*dx][y + (start-1)*dy] == player) {
        start--;
    }
    
    // Find end of sequence
    while (isValidPosition(x + (end+1)*dx, y + (end+1)*dy) && 
           grid[x + (end+1)*dx][y + (end+1)*dy] == player) {
        end++;
    }
    
    // Check if we can extend to form an unstoppable four
    bool canExtendBefore = isValidPosition(x + (start-1)*dx, y + (start-1)*dy) && 
                          grid[x + (start-1)*dx][y + (start-1)*dy] == EMPTY;
    bool canExtendAfter = isValidPosition(x + (end+1)*dx, y + (end+1)*dy) && 
                         grid[x + (end+1)*dx][y + (end+1)*dy] == EMPTY;
    
    return canExtendBefore && canExtendAfter;
}

bool Board::canCreateUnstoppableFour(int x, int y, int dx, int dy, CellState player) const {
    std::string line = buildDirectionalLine(this, x, y, dx, dy, player);
    const int centerIdx = 5; // k = 0 maps to index 5 in [-5..5]
    return matchesOpenThree(line, centerIdx);
}

// Helper methods
int Board::countDirection(int x, int y, int dx, int dy, CellState player) const {
    int count = 0;
    int nx = x, ny = y;
    
    while (isValidPosition(nx, ny) && grid[nx][ny] == player) {
        count++;
        nx += dx;
        ny += dy;
    }
    
    return count;
}

bool Board::checkDirectionAlignment(int x, int y, int dx, int dy, CellState player) const {
    int count = 1; // Count the current piece
    
    // Count in positive direction
    count += countDirection(x + dx, y + dy, dx, dy, player);
    
    // Count in negative direction
    count += countDirection(x - dx, y - dy, -dx, -dy, player);
    
    return count >= 5;
}

int Board::countConsecutive(int x, int y, int dx, int dy, CellState player) const {
    int count = 0;
    
    if (grid[x][y] == player) {
        count = 1;
        
        // Count forward
        int nx = x + dx, ny = y + dy;
        while (isValidPosition(nx, ny) && grid[nx][ny] == player) {
            count++;
            nx += dx;
            ny += dy;
        }
        
        // Count backward
        nx = x - dx;
        ny = y - dy;
        while (isValidPosition(nx, ny) && grid[nx][ny] == player) {
            count++;
            nx -= dx;
            ny -= dy;
        }
    }
    
    return count;
}

bool Board::hasOpenEnds(int x, int y, int dx, int dy, int count, CellState player) const {
    (void)count;
    // Find the bounds of the sequence
    int startX = x, startY = y, endX = x, endY = y;
    
    // Find actual start and end of the sequence
    while (isValidPosition(startX - dx, startY - dy) && grid[startX - dx][startY - dy] == player) {
        startX -= dx;
        startY -= dy;
    }
    
    while (isValidPosition(endX + dx, endY + dy) && grid[endX + dx][endY + dy] == player) {
        endX += dx;
        endY += dy;
    }
    
    // Check if both ends are open (empty)
    bool startOpen = isValidPosition(startX - dx, startY - dy) && 
                     grid[startX - dx][startY - dy] == EMPTY;
    bool endOpen = isValidPosition(endX + dx, endY + dy) && 
                   grid[endX + dx][endY + dy] == EMPTY;
    
    return startOpen && endOpen;
}