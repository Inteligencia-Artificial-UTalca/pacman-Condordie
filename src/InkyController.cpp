#include "InkyController.h"
#include "Ghost.h"
#include <SDL2/SDL.h>
#include <cmath>
#include <cstdlib>

InkyController::InkyController(std::shared_ptr<Character> character): Controller(character), territory({-1, -1}), territorySet(false) {}
InkyController::~InkyController() {}
float InkyController::distToPacman(const GameState& game) const {
    return std::sqrt(euclid2(
        game.getMaze().getNodePos(character->getPos()),
        game.getMaze().getNodePos(game.getPacmanPos())));
}

float InkyController::distToPoint(const GameState& game,std::pair<int,int> p) const {
    return std::sqrt(static_cast<float>(euclid2(game.getMaze().getNodePos(character->getPos()), p)));
}

Move InkyController::getFarthestMove(const GameState& game,std::pair<int,int> target) const {
    int  maxDist = -1;
    Move maxMove = character->getDirection();
    for (Move m : game.getMaze().getPossibleMoves(character->getPos())) {
        int v = game.getMaze().getNeighbour(character->getPos(), m);
        if (v < 0) continue;
        int d = euclid2(game.getMaze().getNodePos(v), target);
        if (d > maxDist) { maxDist = d; maxMove = m; }
    }
    return maxMove;
}

Move InkyController::getClosestMove(const GameState& game,std::pair<int,int> target) const {
    int  minDist = 10000000;
    Move minMove = character->getDirection();
    for (Move m : game.getMaze().getPossibleMoves(character->getPos())) {
        int v = game.getMaze().getNeighbour(character->getPos(), m);
        if (v < 0) continue;
        int d = euclid2(game.getMaze().getNodePos(v), target);
        if (d < minDist) { minDist = d; minMove = m; }
    }
    return minMove;
}

Move InkyController::getMove(const GameState& game) {

    SDL_Event e;
    if (SDL_PollEvent(&e) != 0) {
        if (e.type == SDL_QUIT || (e.type == SDL_KEYDOWN &&(e.key.keysym.sym == SDLK_ESCAPE || e.key.keysym.sym == SDLK_q))) {
            SDL_Quit();
            exit(0);
        }
    }

    if (!territorySet) {
        auto startCoord = game.getMaze().getNodePos(character->getPos());
        territory = { startCoord.first - 60, startCoord.second + 80 };
        territorySet = true;
    }

    auto ghost = std::dynamic_pointer_cast<Ghost>(character);
    bool isEdible = (ghost != nullptr) && ghost->isEdible();

    auto pacmanCoords = game.getMaze().getNodePos(game.getPacmanPos());
    float dPacman = distToPacman(game);

    if (isEdible) {
        float uPanic = 1.0f - 1.0f /
            (1.0f + std::pow(2.718f * 0.45f, -dPacman + 5.0f));
        (void)uPanic; // siempre huye cuando es comestible
        return getFarthestMove(game, pacmanCoords);
    }

    constexpr float FEAR_RADIUS   = 35.0f;
    constexpr float COWARD_THRESH = 0.4f;

    float uCoward = 1.0f - 1.0f /
        (1.0f + std::pow(2.718f * 0.3f, -dPacman + FEAR_RADIUS));

    if (uCoward > COWARD_THRESH) {
        // Huir de Pac-Man
        return getFarthestMove(game, pacmanCoords);
    }

    constexpr float MAX_DIST = 120.0f;
    float dTerr  = std::min(distToPoint(game, territory), MAX_DIST);
    float uTerr  = std::pow((MAX_DIST - dTerr) / MAX_DIST, 2);

    if (uTerr > 0.7f) {
        auto moves = game.getMaze().getPossibleMoves(character->getPos());
        if (!moves.empty())
            return moves[std::rand() % moves.size()];
    }

    // Moverse hacia el centro
    return getClosestMove(game, territory);
}