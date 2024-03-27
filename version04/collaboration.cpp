#include <iostream>
#include "equilibrium.h"
#include <chrono>

using namespace std::chrono;
using namespace std;

//=====================================================================

uint64_t gettime() {
  using namespace std::chrono;
  return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

//=====================================================================

int n = 4;
int s = 2;

//------------------------------------------------------------
Game::Game() : vars(*this,n,0,s-1), util(*this,n,1,s) {
    for (int i=0; i<n; i++) setGoal(i);
    branch(*this, vars, INT_VAR_NONE(), INT_VAL_MIN());
}
//------------------------------------------------------------
void Game::setGoal(int i) {
    count(*this, vars, vars[i], IRT_EQ, util[i]);
}
//------------------------------------------------------------
void Game::print() const {
    std::cout << vars << std::endl;
}

//=====================================================================

int main(int argc, char const *argv[])
{
    // n = atoi(argv[1]);
    // s = atoi(argv[2]);
    
    Game* model = new Game();
    // model->setEquilibriumConstraint();
    DFS<Game> engine(model);
    delete model;

    uint64_t t1 = gettime();

    while (Game* solution = engine.next()) {
        solution->printAll();
        delete solution;
    }
    uint64_t t2 = gettime();

    cout << "Gecode Equilibrium++" << endl;
    cout << n << " " << s << " " << t2-t1 << endl;
    
    return 0;
}
