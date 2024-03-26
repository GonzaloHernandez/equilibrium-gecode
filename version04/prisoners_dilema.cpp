#include <iostream>
#include "equilibrium.h"

using namespace std;

//=====================================================================

Game::Game() : vars(*this,2,0,1), util(*this,2,0,3) {
    setGoal(0);
    setGoal(1);
    branch(*this, vars, INT_VAR_NONE(), INT_VAL_MIN());
}
//-----------------------------------------------------
void Game::setGoal(int i) {
    IntVar x = vars[0];
    IntVar y = vars[1];

    switch (i) {
    case 0: 
        rel(*this, util[0] == 3-((y*2)-x+1) );
        break;
    
    case 1: 
        rel(*this, util[1] == 3-((x*2)-y+1) );
        break;
    }
}
//------------------------------------------------------------
void Game::print() const {
    std::cout << vars << std::endl;
}

//=====================================================================

int main(int argc, char const *argv[])
{
    Game* model = new Game();
    model->setEquilibriumConstraint();
    DFS<Game> engine(model);
    delete model;

    while (Game* solution = engine.next()) {
        solution->print();
        delete solution;
    }
    return 0;
}
