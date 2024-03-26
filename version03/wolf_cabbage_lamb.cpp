#include <iostream>
#include "equilibrium.h"

using namespace std;

//=====================================================================

Game::Game() : vars(*this,3,0,1), util(*this,0,1) {
    branch(*this, vars, INT_VAR_NONE(), INT_VAL_MIN());
}
//-----------------------------------------------------
void Game::setGoal(int i) {
    BoolVar w = expr(*this, vars[0] == 1);  // Wolf
    BoolVar c = expr(*this, vars[1] == 1);  // Cabbage
    BoolVar l = expr(*this, vars[2] == 1);  // Lamb
    BoolVar u = expr(*this, util == 1);     // Utility
    
    switch (i) {
    case 0: 
        rel(*this, u == (w && l));
        break;
    
    case 1: 
        rel(*this, u == 0);
        break;

    case 2: 
        rel(*this, u == ((!w && c && l) || (w && !l)));
        break;

    }
}
//------------------------------------------------------------
void Game::print() const {
    std::cout << vars << " " << util << std::endl;
}

//=====================================================================

int main(int argc, char const *argv[])
{
    Game* model = new Game();
    model->setBranchVars();
    model->setGoal(2);
    model->setEquilibriumConstraint();
    DFS<Game> engine(model);
    delete model;

    while (Game* solution = engine.next()) {
        solution->print();
        delete solution;
    }
    return 0;
}
