#include <iostream>
#include <gecode/int.hh>
#include "equilibrium.h"
#include <chrono>

using namespace std::chrono;
using namespace std;
using namespace Gecode;
using namespace Int;

//==============================================================

class Collaboration : public Space {
protected :
    IntVarArray vars;
public :
    //-----------------------------------------------------
    Collaboration(int n) : vars(*this, n, 0, n-1) {
        equilibriumTable(*this, vars);
        // equilibrium(*this, vars);
        branch(*this, vars, INT_VAR_NONE(), INT_VAL_MIN());
    }
    //-----------------------------------------------------
    Collaboration(Collaboration& source) : Space(source) {
        vars.update(*this, source.vars);
    }
    //-----------------------------------------------------
    virtual Space* copy() {
        return new Collaboration(*this);
    }
    //-----------------------------------------------------
    void print() const {
        cout << vars << endl;
    }
};

//==============================================================

uint64_t gettime() {
  using namespace std::chrono;
  return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

//==============================================================

int main(int argc, char const *argv[])
{
    int n = 7; //atoi(argv[1]);
    // cout << " n = " << n << endl;
    Collaboration* model = new Collaboration(n);
    DFS<Collaboration> engine(model);
    delete model;

    uint64_t t1 = gettime();

    while (Collaboration* solution = engine.next()) {
        solution->print();
        delete solution;
    }
    uint64_t t2 = gettime();

    cout << t2-t1 << endl;
    return 0;
}