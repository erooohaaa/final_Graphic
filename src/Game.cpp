#include "Game.hpp"

#include <algorithm>
#include <limits>

namespace puzzle {

int Piece::width() const {
    int result = 0;
    for (const auto& cell : cells) {
        result = std::max(result, cell.x + 1);
    }
    return result;
}

int Piece::height() const {
    int result = 0;
    for (const auto& cell : cells) {
        result = std::max(result, cell.z + 1);
    }
    return result;
}

bool canPlace(const Grid& grid, const Piece& piece, int x, int z) {
    if (!piece.available || piece.cells.empty() || piece.color < 1 || piece.color > 7) {
        return false;
    }
    std::array<std::array<bool, BoardSize>, BoardSize> visited{};
    for (const auto& cell : piece.cells) {
        const auto targetX = static_cast<long long>(x) + cell.x;
        const auto targetZ = static_cast<long long>(z) + cell.z;
        if (targetX < 0 || targetX >= BoardSize || targetZ < 0 || targetZ >= BoardSize) {
            return false;
        }
        if (grid[targetZ][targetX] != 0 || visited[targetZ][targetX]) {
            return false;
        }
        visited[targetZ][targetX] = true;
    }
    return true;
}

MoveResult placeAndClear(Grid& grid, const Piece& piece, int x, int z) {
    MoveResult result;
    if (!canPlace(grid, piece, x, z)) {
        return result;
    }
    result.placed = true;
    for (const auto& cell : piece.cells) {
        grid[z + cell.z][x + cell.x] = piece.color;
    }
    std::array<bool, BoardSize> fullRows{};
    std::array<bool, BoardSize> fullColumns{};
    for (int i = 0; i < BoardSize; ++i) {
        fullRows[i] = true;
        fullColumns[i] = true;
        for (int j = 0; j < BoardSize; ++j) {
            fullRows[i] = fullRows[i] && grid[i][j] != 0;
            fullColumns[i] = fullColumns[i] && grid[j][i] != 0;
        }
        result.lines += static_cast<int>(fullRows[i]) + static_cast<int>(fullColumns[i]);
    }
    for (int row = 0; row < BoardSize; ++row) {
        for (int column = 0; column < BoardSize; ++column) {
            if (fullRows[row] || fullColumns[column]) {
                result.cleared.push_back({{column, row}, grid[row][column]});
                grid[row][column] = 0;
            }
        }
    }
    return result;
}

Game::Game(std::uint32_t seed) : seed_(seed), random_(seed) {
    reset();
}

void Game::reset() {
    random_.seed(seed_);
    board_ = {};
    score_ = 0;
    combo_ = 0;
    gameOver_ = false;
    refill();
}

void Game::startDemo() {
    reset();
    const Piece purpleL{{{0, 0}, {0, 1}, {0, 2}, {1, 2}}, 6, true};
    const Piece blueSquare{{{0, 0}, {1, 0}, {0, 1}, {1, 1}}, 2, true};
    const Piece greenT{{{0, 0}, {1, 0}, {2, 0}, {1, 1}}, 3, true};
    const Piece orangeLine{{{0, 0}, {1, 0}, {2, 0}}, 4, true};
    const Piece redLine{{{0, 0}, {1, 0}, {2, 0}}, 1, true};
    placeAndClear(board_, purpleL, 1, 1);
    placeAndClear(board_, blueSquare, 5, 1);
    placeAndClear(board_, greenT, 3, 4);
    placeAndClear(board_, orangeLine, 0, 7);
    placeAndClear(board_, redLine, 3, 7);
    pieces_[0] = {{{0, 0}, {1, 0}}, 5, true};
    pieces_[1] = {{{0, 0}, {0, 1}, {0, 2}}, 3, true};
    pieces_[2] = {{{0, 0}, {1, 0}, {0, 1}, {1, 1}}, 7, true};
    updateGameOver();
}

const Grid& Game::board() const {
    return board_;
}

const std::array<Piece, 3>& Game::pieces() const {
    return pieces_;
}

int Game::score() const {
    return score_;
}

int Game::combo() const {
    return combo_;
}

bool Game::gameOver() const {
    return gameOver_;
}

bool Game::canPlace(int slot, int x, int z) const {
    return !gameOver_ && slot >= 0 && slot < static_cast<int>(pieces_.size())
        && puzzle::canPlace(board_, pieces_[slot], x, z);
}

MoveResult Game::place(int slot, int x, int z) {
    if (!canPlace(slot, x, z)) {
        return {};
    }
    const int cellCount = static_cast<int>(pieces_[slot].cells.size());
    MoveResult result = placeAndClear(board_, pieces_[slot], x, z);
    pieces_[slot].available = false;
    combo_ = result.lines > 0 ? std::min(combo_ + 1, 1000000) : 0;
    const long long points = 10LL * cellCount + 100LL * result.lines * combo_;
    result.gained = static_cast<int>(std::min(points,
        static_cast<long long>(std::numeric_limits<int>::max()) - score_));
    score_ += result.gained;
    const bool consumed = std::none_of(pieces_.begin(), pieces_.end(),
        [](const Piece& piece) { return piece.available; });
    if (consumed) {
        refill();
    }
    updateGameOver();
    return result;
}

std::vector<Piece> Game::shapes() {
    std::vector<Piece> result = {{{{0, 0}}, 1, true}};
    for (int length = 2; length <= 5; ++length) {
        Piece horizontal;
        Piece vertical;
        for (int i = 0; i < length; ++i) {
            horizontal.cells.push_back({i, 0});
            vertical.cells.push_back({0, i});
        }
        result.push_back(horizontal);
        result.push_back(vertical);
    }
    result.push_back({{{0, 0}, {1, 0}, {0, 1}, {1, 1}}, 1, true});
    result.push_back({{{0, 0}, {1, 0}, {2, 0}, {0, 1}, {1, 1}, {2, 1},
                       {0, 2}, {1, 2}, {2, 2}}, 1, true});
    result.push_back({{{0, 0}, {0, 1}, {0, 2}, {1, 2}}, 1, true});
    result.push_back({{{0, 0}, {1, 0}, {2, 0}, {0, 1}}, 1, true});
    result.push_back({{{0, 0}, {1, 0}, {1, 1}, {1, 2}}, 1, true});
    result.push_back({{{2, 0}, {0, 1}, {1, 1}, {2, 1}}, 1, true});
    result.push_back({{{0, 0}, {1, 0}, {2, 0}, {1, 1}}, 1, true});
    result.push_back({{{1, 0}, {0, 1}, {1, 1}, {1, 2}}, 1, true});
    result.push_back({{{1, 0}, {0, 1}, {1, 1}, {2, 1}}, 1, true});
    result.push_back({{{0, 0}, {0, 1}, {1, 1}, {0, 2}}, 1, true});
    result.push_back({{{1, 0}, {2, 0}, {0, 1}, {1, 1}}, 1, true});
    result.push_back({{{0, 0}, {0, 1}, {1, 1}, {1, 2}}, 1, true});
    result.push_back({{{0, 0}, {1, 0}, {1, 1}, {2, 1}}, 1, true});
    result.push_back({{{1, 0}, {0, 1}, {1, 1}, {0, 2}}, 1, true});
    return result;
}

void Game::refill() {
    const auto candidates = shapes();
    std::uniform_int_distribution<std::size_t> selectShape(0, candidates.size() - 1);
    std::uniform_int_distribution<int> selectColor(1, 7);
    for (auto& piece : pieces_) {
        piece = candidates[selectShape(random_)];
        piece.color = selectColor(random_);
    }
}

void Game::updateGameOver() {
    for (const auto& piece : pieces_) {
        if (!piece.available) {
            continue;
        }
        for (int z = 0; z < BoardSize; ++z) {
            for (int x = 0; x < BoardSize; ++x) {
                if (puzzle::canPlace(board_, piece, x, z)) {
                    gameOver_ = false;
                    return;
                }
            }
        }
    }
    gameOver_ = true;
}

}
