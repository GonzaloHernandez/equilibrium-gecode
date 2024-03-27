#include <iostream>
#include "equilibrium.h"

using namespace std;

//=====================================================================

//------------------------------------------------------------
Game::Game() : vars(*this,3,1,3), util(*this,3,0,1) {
    
    for (int i=0; i<3; i++) setGoal(i);

    branch(*this, vars, INT_VAR_NONE(), INT_VAL_MIN());
}
//------------------------------------------------------------
void Game::setGoal(int i) {
    switch (i) {
    case 0 :
        rel(*this, util[0] == 0);
        break;
    case 1 :
        rel(*this, util[1] == 0);
        break;
    case 2 :
        rel(*this, util[2] == 0);
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
