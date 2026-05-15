#include "InkyController.h"

Inky_Info* Inky_Info::info=nullptr;

InkyController::InkyController(std::shared_ptr<Character> character):Controller(character),root(std::make_shared<Selector>()){
	auto filter = std::make_shared<Filter>();
	filter->addCondition(std::make_shared<Powerpill_Inky>());
	filter->addAction(std::make_shared<Frightened_Inky>());
	root->addChild(filter);
	auto filter2 = std::make_shared<Filter>();
	filter2->addCondition(std::make_shared<TimeOut_Inky>());
	filter2->addAction(std::make_shared<Scatter_Inky>());
	root->addChild(filter2);
	root->addChild(std::make_shared<Chase_Inky>());
}

InkyController::~InkyController() {

}

Move
InkyController::getMove(const GameState& gs){
	Inky_Info::getInfo()->in_character=character;
	Inky_Info::getInfo()->in_gamestate=&gs;
	root->tick();

	return Inky_Info::getInfo()->out_move;		
}

TimeOut_Inky::TimeOut_Inky() : Behavior() {
	lastTime = std::chrono::high_resolution_clock::now();

}

Status TimeOut_Inky::update(){
	std::chrono::duration<float> timeStamp = std::chrono::high_resolution_clock::now() - lastTime;
	if( (int)timeStamp.count()%27 < 7){
		return BH_SUCCESS;
	}else{
		return BH_FAILURE;
	}

}

Status Chase_Inky::update(){
	//std::cerr << " Chase \n" ;
	auto character = Inky_Info::getInfo()->in_character;
	auto gs = Inky_Info::getInfo()->in_gamestate;
	auto target= gs->getMaze().getNodePos(gs->getPacmanPos());
	float min=1000000000;
	Move minMove=PASS;
	std::vector<Move> moves;
	if(character->getDirection()==PASS) {
		moves=gs->getMaze().getPossibleMoves(character->getPos());
	} else {
		moves = gs->getMaze().getGhostLegalMoves(character->getPos(), character->getDirection());
	}

	for(auto move:moves) {
		if(move==PASS) {
			break;
		}
		float dist = euclid2(target,gs->getMaze().getNodePos(gs->getMaze().getNeighbour(character->getPos(),move)));
		if(dist<min) {
			min=dist;
			minMove=move;
		}
	}
	Inky_Info::getInfo()->out_move = minMove;
	return BH_SUCCESS;
}

Status Powerpill_Inky::update(){
	auto character = Inky_Info::getInfo()->in_character;
	auto ghost = dynamic_cast<Ghost*>(character.get());

	if( ghost!=nullptr && ghost->isEdible()){
		return BH_SUCCESS;
	}else{
		return BH_FAILURE;
	}

}

Frightened_Inky::Frightened_Inky() : Behavior(), e(rand()), uniform_dist(0,3){

}

Status Frightened_Inky::update(){
	//std::cerr << " Frightened \n" ;
	auto character = Inky_Info::getInfo()->in_character;
	auto gs = Inky_Info::getInfo()->in_gamestate;
	std::vector<Move> moves;
	if(character->getDirection()==PASS) {
		moves=gs->getMaze().getPossibleMoves(character->getPos());
	} else {
		moves = gs->getMaze().getGhostLegalMoves(character->getPos(), character->getDirection());
	}
	Move m = moves[rand()%moves.size()];
	Inky_Info::getInfo()->out_move = m;
	return BH_SUCCESS; //NO es as� pero por ahora
}

Scatter_Inky :: Scatter_Inky() : Behavior(){
	target = std::make_pair(-1,-1);

}

Status Scatter_Inky::update(){
	//std::cerr << " Scatter \n" ;
	if(target.first == -1){
		target = Inky_Info::getInfo()->in_gamestate->getMaze().getPowerPillPositions()[0];
	}

	auto character = Inky_Info::getInfo()->in_character;
	auto gs = Inky_Info::getInfo()->in_gamestate;

	Move minMove=PASS;
	std::vector<Move> moves;
	if(character->getDirection()==PASS) {
		moves=gs->getMaze().getPossibleMoves(character->getPos());
	} else {
		moves = gs->getMaze().getGhostLegalMoves(character->getPos(), character->getDirection());
	}

	float min=100000000;
	for(auto move:moves) {
		if(move==PASS) {
			break;
		}
		float dist = euclid2(target,gs->getMaze().getNodePos(gs->getMaze().getNeighbour(character->getPos(),move)));
		if(dist<min) {
			min=dist;
			minMove=move;
		}
	}
	Inky_Info::getInfo()->out_move = minMove;
	return BH_SUCCESS;

}
