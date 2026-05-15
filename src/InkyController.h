#pragma once

#include "Controller.h"
#include "BehaviorTree.h"
#include <random>


class Inky_Info{
    static Inky_Info *info;
    Inky_Info(){}

public:
    static Inky_Info* getInfo(){
        if(info==nullptr)info = new Inky_Info();
        return info;
    }
    const GameState* in_gamestate;
    Move out_move;
    std::shared_ptr<Character> in_character;
};



class InkyController: public Controller {

private:
	std::shared_ptr<Composite> root;
public:
	InkyController(std::shared_ptr<Character> character);
	virtual ~InkyController();
	virtual Move getMove(const GameState& gs)override;
};


class Chase_Inky : public Behavior{
public:
    virtual Status update() override;

};

class Frightened_Inky : public Behavior{
private:
    std::mt19937 e;
    std::uniform_int_distribution<int> uniform_dist;
public:
    virtual Status update() override;
    Frightened_Inky();

};

class Scatter_Inky : public Behavior{
private:
    std::pair<int,int> target;

public:
    virtual Status update() override;
    Scatter_Inky();

};

class Powerpill_Inky : public Behavior{
public:
    virtual Status update() override;
};

class TimeOut_Inky	 : public Behavior{
private:
    std::chrono::time_point<std::chrono::high_resolution_clock> lastTime;
public:
    virtual Status update() override;
    TimeOut_Inky();
};

