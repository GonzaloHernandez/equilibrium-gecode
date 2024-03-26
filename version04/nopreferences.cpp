#include <iostream>
#include "equilibrium.h"

using namespace std;

//=====================================================================


int n = 6;
int s = 6;


Game::Game() : vars(*this,n,0,s-1), util(*this,n,1,s) {
    for (int i=0; i<n; i++) setGoal(i);
    branch(*this, vars, INT_VAR_NONE(), INT_VAL_MIN());
}
//-----------------------------------------------------
void Game::setGoal(int i) {
        rel(*this, util[i] == 1);
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
