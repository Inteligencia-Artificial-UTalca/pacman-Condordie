#include "PacmanController.h"
#include <SDL2/SDL.h>
#include <cmath>

PacmanController::PacmanController(std::shared_ptr<Character> character): Controller(character) {}
PacmanController::~PacmanController() {}

static Move oppositeOf(Move m) {
    if (m == UP)    return DOWN;
    if (m == DOWN)  return UP;
    if (m == LEFT)  return RIGHT;
    if (m == RIGHT) return LEFT;
    return PASS;
}

Move PacmanController::getClosestMove(const GameState& game, std::pair<int,int> target) const {
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

Move PacmanController::getFarthestMove(const GameState& game,std::pair<int,int> target) const {
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

Move PacmanController::getEscapeMoveFromAll(
        const GameState& game,
        const std::vector<std::pair<int,int>>& threats) const {

    Move currentDir = character->getDirection();
    Move opposite   = oppositeOf(currentDir);
    auto moves      = game.getMaze().getPossibleMoves(character->getPos());

    Move bestMove  = currentDir;
    int  bestScore = -1;

    for (Move m : moves) {
        if (m == opposite && moves.size() > 1) continue;
        int v = game.getMaze().getNeighbour(character->getPos(), m);
        if (v < 0) continue;
        auto vc = game.getMaze().getNodePos(v);
        int total = 0;
        for (auto& t : threats) total += euclid2(vc, t);
        if (total > bestScore) { bestScore = total; bestMove = m; }
    }
    return bestMove;
}

Move PacmanController::getExploreMove(const GameState& game) const {
    int  pacNode  = character->getPos();
    auto pacCoord = game.getMaze().getNodePos(pacNode);
    Move opposite = oppositeOf(character->getDirection());

    auto pills      = game.getMaze().getPillPositions();
    auto powerPills = game.getMaze().getPowerPillPositions();
    pills.insert(pills.end(), powerPills.begin(), powerPills.end());
    if (pills.empty()) return PASS;

    // Pill mas cercana
    auto  closest = pills[0];
    float minD    = euclid2(pacCoord, pills[0]);
    for (auto& p : pills) {
        float d = euclid2(pacCoord, p);
        if (d < minD) { minD = d; closest = p; }
    }

    Move desired = getClosestMove(game, closest);
    if (desired == opposite) {
        for (Move m : game.getMaze().getPossibleMoves(pacNode)) {
            if (m != opposite && game.getMaze().getNeighbour(pacNode, m) >= 0)
                return m;
        }
    }
    return desired;
}

float PacmanController::getDistanceToGhost(const GameState& game, int g) const {
    return std::sqrt(euclid2(
        game.getMaze().getNodePos(character->getPos()),
        game.getMaze().getNodePos(game.getGhostsPos(g))));
}

Move PacmanController::getMove(const GameState& game) {

    // Cierre de ventana
    SDL_Event e;
    if (SDL_PollEvent(&e) != 0) {
        if (e.type == SDL_QUIT ||
            (e.type == SDL_KEYDOWN && (e.key.keysym.sym == SDLK_ESCAPE || e.key.keysym.sym == SDLK_q))) {
            SDL_Quit();
            exit(0);
        }
    }

    int pacNode = character->getPos();
    Move opposite = oppositeOf(character->getDirection());

    std::vector<std::pair<int,int>> ghostPos;
    std::vector<bool>               edible;
    for (int i = 0; i < 4; i++) {
        ghostPos.push_back(game.getMaze().getNodePos(game.getGhostsPos(i)));
        edible.push_back(game.isGhostEdible(i));
    }

    float fear         = 0.0f;
    float hunger       = 0.0f;
    Move  eatGhostMove = PASS;
    std::vector<std::pair<int,int>> threats;

    // Miedo: funcion logistica, umbral de amenaza activa = 0.15
    for (int i = 0; i < 4; i++) {
        if (!edible[i]) {
            float d = getDistanceToGhost(game, i);
            float u = 1.0f - 1.0f / (1.0f + std::pow(2.718f * 0.45f, -d + 20.0f));
            if (u > 0.15f) {
                threats.push_back(ghostPos[i]);
                if (u > fear) fear = u;
            }
        }
    }

    // Hambre: funcion cuadratica con factor 1.5
    for (int i = 0; i < 4; i++) {
        if (edible[i]) {
            float d = getDistanceToGhost(game, i);
            float u = 1.5f * std::pow((100.0f - d) / 100.0f, 2);
            if (u > hunger) {
                hunger       = u;
                eatGhostMove = getClosestMove(game, ghostPos[i]);
            }
        }
    }

    Move finalMove = PASS;

    if (fear > 0.3f && !threats.empty()) {
        finalMove = getEscapeMoveFromAll(game, threats);

    } else if (hunger > 0.1f) {
        // Persecucion de fantasma comestible (evitar retroceder)
        if (eatGhostMove == opposite) {
            bool changed = false;
            for (Move m : game.getMaze().getPossibleMoves(pacNode)) {
                if (m != opposite && game.getMaze().getNeighbour(pacNode, m) >= 0) {
                    finalMove = m;
                    changed   = true;
                    break;
                }
            }
            if (!changed) finalMove = eatGhostMove;
        } else {
            finalMove = eatGhostMove;
        }

    } else {
        // Exploracion hacia la pill mas cercana
        finalMove = getExploreMove(game);
    }

    if (game.getMaze().getNeighbour(pacNode, finalMove) < 0) {
        auto possible = game.getMaze().getPossibleMoves(pacNode);
        if (!possible.empty()) finalMove = possible[0];
    }

    return finalMove;
}