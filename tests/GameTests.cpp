#include "Game.hpp"

#include <algorithm>
#include <array>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <set>
#include <stdexcept>
#include <string>
#include <tuple>

using namespace puzzle;

namespace {

void require(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

bool samePiece(const Piece& a, const Piece& b) {
    if (a.color != b.color || a.available != b.available || a.cells.size() != b.cells.size()) {
        return false;
    }
    for (std::size_t i = 0; i < a.cells.size(); ++i) {
        if (a.cells[i].x != b.cells[i].x || a.cells[i].z != b.cells[i].z) {
            return false;
        }
    }
    return true;
}

bool samePieces(const std::array<Piece, 3>& a, const std::array<Piece, 3>& b) {
    for (std::size_t i = 0; i < a.size(); ++i) {
        if (!samePiece(a[i], b[i])) {
            return false;
        }
    }
    return true;
}

int occupied(const Grid& grid) {
    int total = 0;
    for (const auto& row : grid) {
        total += static_cast<int>(std::count_if(row.begin(), row.end(), [](int value) {
            return value != 0;
        }));
    }
    return total;
}

bool directFit(const Grid& grid, const Piece& piece, int x, int z) {
    if (!piece.available) {
        return false;
    }
    for (const auto& cell : piece.cells) {
        const int cx = x + cell.x;
        const int cz = z + cell.z;
        if (cx < 0 || cx >= BoardSize || cz < 0 || cz >= BoardSize || grid[cz][cx] != 0) {
            return false;
        }
    }
    return true;
}

std::tuple<int, int, int> firstMove(const Game& game, int onlySlot = -1) {
    for (int slot = 0; slot < 3; ++slot) {
        if (onlySlot >= 0 && slot != onlySlot) {
            continue;
        }
        for (int z = 0; z < BoardSize; ++z) {
            for (int x = 0; x < BoardSize; ++x) {
                if (directFit(game.board(), game.pieces()[slot], x, z)) {
                    return {slot, x, z};
                }
            }
        }
    }
    return {-1, 0, 0};
}

void placementValidation() {
    Grid grid{};
    const Piece square{{{0, 0}, {1, 0}, {0, 1}, {1, 1}}, 3, true};
    require(canPlace(grid, square, 6, 6), "A 2x2 piece must fit in the bottom-right corner");
    require(!canPlace(grid, square, 7, 6), "A piece must not cross the right edge");
    require(!canPlace(grid, square, 6, 7), "A piece must not cross the bottom edge");
    require(!canPlace(grid, square, -1, 0), "A piece must not cross the left edge");
    require(!canPlace(grid, square, 0, -1), "A piece must not cross the top edge");
    require(!canPlace(grid, square, std::numeric_limits<int>::max(), 0), "Large coordinates must be rejected safely");
    grid[3][4] = 6;
    const auto before = grid;
    const auto invalid = placeAndClear(grid, square, 3, 2);
    require(!invalid.placed && invalid.cleared.empty() && invalid.lines == 0, "Collision must fail cleanly");
    require(grid == before, "Invalid placement must not partially write cells");
    const auto valid = placeAndClear(grid, square, 6, 6);
    require(valid.placed && valid.lines == 0 && valid.gained == 0, "Pure placement must not award score");
    require(grid[6][6] == 3 && grid[6][7] == 3 && grid[7][6] == 3 && grid[7][7] == 3,
        "All piece cells must retain their color");
    Piece used = square;
    used.available = false;
    require(!canPlace(grid, used, 0, 0), "A consumed piece must not be placeable");
    require(!canPlace(grid, Piece{}, 0, 0), "An empty piece must not be placeable");
    require(!canPlace(grid, Piece{{{0, 0}, {0, 0}}, 1, true}, 0, 0), "Duplicate cells must be rejected");
}

void simultaneousClears() {
    Grid grid{};
    for (int i = 0; i < BoardSize; ++i) {
        grid[3][i] = 2;
        grid[i][4] = 5;
    }
    grid[3][4] = 0;
    grid[0][0] = 7;
    const auto result = placeAndClear(grid, Piece{{{0, 0}}, 4, true}, 4, 3);
    require(result.placed && result.lines == 2, "Cross completion must clear both lines simultaneously");
    require(result.cleared.size() == 15, "The crossing cell must only be returned once");
    std::set<std::pair<int, int>> cleared;
    for (const auto& cell : result.cleared) {
        require(cleared.insert({cell.cell.x, cell.cell.z}).second, "Cleared cells must be unique");
        const int expectedColor = cell.cell.x == 4 ? (cell.cell.z == 3 ? 4 : 5) : 2;
        require(cell.color == expectedColor, "Clearing animation must receive the original cell color");
    }
    require(occupied(grid) == 1 && grid[0][0] == 7, "Only completed lines may be removed");

    Grid doubleRow{};
    for (int x = 0; x < BoardSize - 1; ++x) {
        doubleRow[1][x] = 1;
        doubleRow[2][x] = 2;
    }
    const auto two = placeAndClear(doubleRow, Piece{{{0, 0}, {0, 1}}, 6, true}, 7, 1);
    require(two.lines == 2 && two.cleared.size() == 16 && occupied(doubleRow) == 0,
        "One placement can complete two rows");
}

void refillAndReset() {
    Game game(12345);
    Game twin(12345);
    const auto initialPieces = game.pieces();
    require(occupied(game.board()) == 0 && !game.gameOver(), "A new game must start empty and playable");
    require(samePieces(initialPieces, twin.pieces()), "A seed must reproduce the offered pieces");
    const auto initialBoard = game.board();
    require(!game.place(-1, 0, 0).placed && !game.place(3, 0, 0).placed,
        "Invalid slot indices must be rejected");
    require(!game.place(0, -20, -20).placed && game.board() == initialBoard && game.score() == 0,
        "Rejected moves must preserve game state");
    require(samePieces(game.pieces(), initialPieces), "Rejected moves must preserve the offer");
    for (int slot = 0; slot < 3; ++slot) {
        const auto [chosen, x, z] = firstMove(game, slot);
        require(chosen == slot, "The deterministic test offer must have room for all three pieces");
        const auto result = game.place(slot, x, z);
        const auto other = twin.place(slot, x, z);
        require(result.placed && other.placed && result.gained == other.gained, "Equal moves must remain deterministic");
        require(game.board() == twin.board() && samePieces(game.pieces(), twin.pieces()), "Equal seeds and inputs must match");
        if (slot < 2) {
            for (int i = 0; i <= slot; ++i) {
                require(!game.pieces()[i].available, "Used offers must stay unavailable until all three are used");
            }
            for (int i = slot + 1; i < 3; ++i) {
                require(samePiece(game.pieces()[i], initialPieces[i]), "Unused offers must not be replaced early");
            }
            const auto snapshot = game.board();
            const int oldScore = game.score();
            require(!game.place(slot, 7, 7).placed && game.board() == snapshot && game.score() == oldScore,
                "A used slot must never change the board or score");
        }
    }
    require(std::all_of(game.pieces().begin(), game.pieces().end(), [](const Piece& p) { return p.available; }),
        "All three slots must refill together");
    game.reset();
    require(occupied(game.board()) == 0 && game.score() == 0 && game.combo() == 0 && !game.gameOver(),
        "Reset must restore an empty playable game");
    require(samePieces(game.pieces(), initialPieces), "Reset must reproduce the initial seed");
}

void reachableGames() {
    int clearMoves = 0;
    int terminalGames = 0;
    int comboMoves = 0;
    for (std::uint32_t seed = 1; seed <= 100; ++seed) {
        Game game(seed);
        int expectedScore = 0;
        int expectedCombo = 0;
        for (int turn = 0; turn < 1000; ++turn) {
            const auto [slot, x, z] = firstMove(game);
            require(game.gameOver() == (slot < 0), "Game over must exactly mean no remaining offer fits anywhere");
            if (slot < 0) {
                ++terminalGames;
                const auto snapshot = game.board();
                const int beforeScore = game.score();
                require(!game.place(0, 0, 0).placed && game.board() == snapshot && game.score() == beforeScore,
                    "Game-over input must preserve the finished game");
                break;
            }
            const int cells = static_cast<int>(game.pieces()[slot].cells.size());
            const int occupiedBefore = occupied(game.board());
            require(game.canPlace(slot, x, z), "A physically valid offer must be accepted before game over");
            const auto result = game.place(slot, x, z);
            require(result.placed, "An accepted move must place successfully");
            expectedCombo = result.lines > 0 ? expectedCombo + 1 : 0;
            const int expectedGain = 10 * cells + 100 * result.lines * expectedCombo;
            expectedScore += expectedGain;
            require(result.gained == expectedGain && game.score() == expectedScore && game.combo() == expectedCombo,
                "Scoring must follow 10/cell plus 100/line times consecutive-clear combo");
            require(occupied(game.board()) == occupiedBefore + cells - static_cast<int>(result.cleared.size()),
                "Placement and clearing must conserve the number of occupied cells");
            for (const auto& row : game.board()) {
                require(std::find(row.begin(), row.end(), 0) != row.end(), "No completed row may remain after a move");
            }
            if (result.lines > 0) {
                ++clearMoves;
            }
            if (expectedCombo > 1) {
                ++comboMoves;
            }
        }
    }
    require(clearMoves > 0 && terminalGames > 0 && comboMoves > 0,
        "Reachable gameplay must exercise clears, consecutive clears, and actual loss");
}

void demonstrationFixture() {
    Game game(12345);
    const auto originalPieces = game.pieces();
    game.startDemo();
    require(game.score() == 0 && game.combo() == 0 && !game.gameOver(),
        "Demo must start with fresh counters and legal moves");
    require(occupied(game.board()) == 18, "Demo must contain the advertised lightweight sample layout");
    require(std::all_of(game.pieces().begin(), game.pieces().end(), [](const Piece& p) { return p.available; }),
        "Demo must offer all three pieces");
    for (int x = 0; x < 6; ++x) {
        require(game.board()[7][x] != 0, "Demo bottom row must have six filled cells");
    }
    require(game.board()[7][6] == 0 && game.board()[7][7] == 0,
        "Demo bottom row must leave two adjacent empty cells");
    const auto result = game.place(0, 6, 7);
    require(result.placed && result.lines == 1 && result.cleared.size() == 8,
        "Demo first piece must complete exactly the bottom row");
    require(result.gained == 120 && game.score() == 120 && game.combo() == 1,
        "Demo must use the same normal scoring rules");
    require(occupied(game.board()) == 12 && !game.pieces()[0].available,
        "Demo clear must preserve other clusters and consume the offered piece");
    game.reset();
    require(occupied(game.board()) == 0 && game.score() == 0 && game.combo() == 0 && !game.gameOver(),
        "Normal reset after demo must restore the empty normal game");
    require(samePieces(game.pieces(), originalPieces), "Demo must not alter the normal initial seed");
}

}

int main() {
    try {
        placementValidation();
        simultaneousClears();
        refillAndReset();
        reachableGames();
        demonstrationFixture();
        std::cout << "All game tests passed.\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << "Test failure: " << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
