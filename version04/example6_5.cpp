#include <iostream>
#include "equilibrium.h"

using namespace std;

//=====================================================================

//------------------------------------------------------------
Game::Game() : vars(*this,3,1,9), util(*this,3,0,1) {    
    rel(*this,vars[2],IRT_LQ,3);

    // rel(*this,vars[0],IRT_EQ,4);
    // rel(*this,vars[1],IRT_EQ,4);

    for (int i=0; i<3; i++) setGoal(i);
    branch(*this, vars, INT_VAR_NONE(), INT_VAL_MIN());
}
//------------------------------------------------------------
void Game::setGoal(int i) {
    IntVar x = vars[0];
    IntVar y = vars[1];
    IntVar z = vars[2];
    BoolVar ux = expr(*this, util[0] == true);
    BoolVar uy = expr(*this, util[1] == true);
    BoolVar uz = expr(*this, util[2] == true);

    switch (i) {
    case 0 :
        rel(*this, ux == (x==y*z));
        break;
    case 1 :
        rel(*this, uy == (y==x*z));
        break;
    case 2 :
        rel(*this, uz == (((x*y)<=z) && (z<=(x+y)) && ((x+1)*(y+1) != (z*3))) );
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
        solution->printAll();
        delete solution;
    }
    
    return 0;
}
