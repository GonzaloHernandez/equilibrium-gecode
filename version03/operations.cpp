#include <iostream>
#include "equilibrium.h"

using namespace std;

//=====================================================================

Game::Game() : vars(*this,3,1,3), util(*this,-30,30) {
    branch(*this, vars, INT_VAR_NONE(), INT_VAL_MIN());
}
//-----------------------------------------------------
void Game::setGoal(int i) {
    IntVar x = vars[0];
    IntVar y = vars[1];
    IntVar z = vars[2];
    IntVar u = util;

    switch (i) {
    case 0: 
        rel(*this, u == x + y + z);
        break;
    
    case 1: 
        rel(*this, u == x * y * z);
        break;

    case 2: 
        rel(*this, u == x - y - z);
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
    model->setBranchVars();
    // model->setGoal(0);
    model->setEquilibriumConstraint();
    DFS<Game> engine(model);
    delete model;

    while (Game* solution = engine.next()) {
        solution->print();
        delete solution;
    }
    return 0;
}
