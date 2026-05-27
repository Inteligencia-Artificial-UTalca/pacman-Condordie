#pragma once
#include "Controller.h"

class InkyController : public Controller {

    float distToPacman(const GameState& game) const;
    float distToPoint(const GameState& game, std::pair<int,int> p) const;
    Move  getFarthestMove(const GameState& game, std::pair<int,int> target) const;
    Move  getClosestMove(const GameState& game, std::pair<int,int> target) const;

    std::pair<int,int> territory;
    bool               territorySet;

public:
    InkyController(std::shared_ptr<Character> character);
    virtual ~InkyController();
    virtual Move getMove(const GameState& game) override;
};