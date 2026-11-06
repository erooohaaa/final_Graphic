#pragma once

#include <array>
#include <cstdint>
#include <random>
#include <vector>

namespace puzzle {

constexpr int BoardSize = 8;

struct Cell {
    int x;
    int z;
};

struct Piece {
    std::vector<Cell> cells;
    int color = 1;
    bool available = true;

    int width() const;
    int height() const;
};

using Grid = std::array<std::array<int, BoardSize>, BoardSize>;

struct ClearedCell {
    Cell cell;
    int color;
};

struct MoveResult {
    bool placed = false;
    std::vector<ClearedCell> cleared;
    int lines = 0;
    int gained = 0;
};

bool canPlace(const Grid& grid, const Piece& piece, int x, int z);
MoveResult placeAndClear(Grid& grid, const Piece& piece, int x, int z);

class Game {
public:
    explicit Game(std::uint32_t seed = 12345);
    void reset();
    void startDemo();
    const Grid& board() const;
    const std::array<Piece, 3>& pieces() const;
    int score() const;
    int combo() const;
    bool gameOver() const;
    bool canPlace(int slot, int x, int z) const;
    MoveResult place(int slot, int x, int z);
    static std::vector<Piece> shapes();

private:
    void refill();
    void updateGameOver();

    std::uint32_t seed_;
    std::mt19937 random_;
    Grid board_{};
    std::array<Piece, 3> pieces_{};
    int score_ = 0;
    int combo_ = 0;
    bool gameOver_ = false;
};

}
