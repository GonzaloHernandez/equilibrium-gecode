#include <iostream>
#include "equilibrium.h"

using namespace std;

//=====================================================================

Game::Game() : vars(*this,3,0,1), util(*this,3,0,1) {
    setGoal(0);
    setGoal(1);
    setGoal(2);
    branch(*this, vars, INT_VAR_NONE(), INT_VAL_MIN());
}
//-----------------------------------------------------
void Game::setGoal(int i) {
    BoolVar w  = expr(*this, vars[0] == 1);  // Wolf
    BoolVar c  = expr(*this, vars[1] == 1);  // Cabbage
    BoolVar l  = expr(*this, vars[2] == 1);  // Lamb

    BoolVar uw = expr(*this, util[0] == 1);  // Utility
    BoolVar uc = expr(*this, util[1] == 1);  // Utility
    BoolVar ul = expr(*this, util[2] == 1);  // Utility
    
    switch (i) {
    case 0: 
        rel(*this, uw == (w && l));
        break;
    
    case 1: 
        rel(*this, uc == 0);
        break;

    case 2: 
        rel(*this, ul == ((!w && c && l) || (w && !l)));
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
    model->setEquilibriumConstraint();
    DFS<Game> engine(model);
    delete model;

    while (Game* solution = engine.next()) {
        solution->print();
        delete solution;
    }
    return 0;
}
