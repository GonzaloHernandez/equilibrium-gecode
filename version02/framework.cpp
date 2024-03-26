#include <iostream>
#include <gecode/int.hh>
#include <gecode/minimodel.hh>

using namespace Gecode;
using namespace Int;
using namespace std;

//=====================================================================

void equilibrium(Space& , const IntVarArgs& );

//=====================================================================

class Game : public Space {
protected:
    IntVarArray vars;
    IntVar      util;
public:
    //------------------------------------------------------------
    Game() : vars(*this,3,0,1), util(*this,0,1) {
        setHardConstraints();
    }
    //------------------------------------------------------------
    Game(Game& source) : Space(source) {
        vars.update(*this, source.vars);
        util.update(*this, source.util);
    }
    //-----------------------------------------------------
    void setHardConstraints() {
    }
    //-----------------------------------------------------
    void setEquilibriumConstraint() {
        equilibrium(*this, vars);
    }
    //------------------------------------------------------------
    void setBranchVars() {
        branch(*this, vars, INT_VAR_NONE(), INT_VAL_MIN());
    }
    //------------------------------------------------------------
    void setBranchUtil() {
        branch(*this, util, INT_VAL_MIN());
    }
    //------------------------------------------------------------
    virtual Space* copy() {
        return new Game(*this);
    }
    //------------------------------------------------------------
    void print() const {
        cout << vars << endl;
    }
    //------------------------------------------------------------
    void printAll() const {
        cout << vars << " " << util << endl;
    }
    //-----------------------------------------------------
    void fixValue(int i,int val) {
        rel(*this, vars[i], IRT_EQ, val);
    }
    //-----------------------------------------------------
    void fixAllValues(ViewArray<IntView>& vars) {
        for (int i=0; i<vars.size(); i++) {
            rel(*this, this->vars[i], IRT_EQ, vars[i].val());
        }
    }
    //-----------------------------------------------------
    void fixAllValuesExcept(ViewArray<IntView>& vars,int ei) {
        for (int i=0; i<vars.size(); i++) {
            if (i == ei) continue;
            rel(*this, this->vars[i], IRT_EQ, vars[i].val());
        }
    }
    //-----------------------------------------------------
    void setGoal(int i) {
        BoolVar w = expr(*this, vars[0] == 1);
        BoolVar c = expr(*this, vars[1] == 1);
        BoolVar l = expr(*this, vars[2] == 1);
        BoolVar u = expr(*this, util == 1);
        
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
    //-----------------------------------------------------
    void setPreference(int val) {
        rel(*this, util, IRT_GR, val);
    }
    //-----------------------------------------------------
    void findEqualUtiliy(int val) {
        rel(*this, util, IRT_EQ, val);
    }
    //-----------------------------------------------------
    int getUtility() {
        return util.val();
    }
    //-----------------------------------------------------
    int getVal(int i) {
        return vars[i].val();
    }
    //-----------------------------------------------------
};

//=====================================================================

class Equilibrium : public Propagator {
protected:
    ViewArray<IntView> vars;
public :
    //------------------------------------------------------------
    Equilibrium(Space& home, ViewArray<IntView> v) 
    : Propagator(home), vars(v) 
    {
        vars.subscribe(home, *this, PC_INT_DOM);
    }
    //------------------------------------------------------------
    static ExecStatus post(Space& home, ViewArray<IntView> v) {
        (void) new (home) Equilibrium(home, v);
        return ES_OK;
    }
    //------------------------------------------------------------
    Equilibrium(Space& home, Equilibrium& source) 
    : Propagator(home,source)
    {
        vars.update(home, source.vars);
    }
    //------------------------------------------------------------
    virtual Propagator* copy(Space& home) {
        return new (home) Equilibrium(home, *this);
    }
    //------------------------------------------------------------
    virtual void reschedule(Space& home) {
        vars.reschedule(home, *this, PC_INT_DOM);
    }
    //------------------------------------------------------------
    virtual PropCost cost(const Space&, const ModEventDelta&) const {
        return PropCost::binary(PropCost::LO);
    } 
    //------------------------------------------------------------
    virtual ExecStatus propagate(Space& home, const ModEventDelta&) {
        if (vars.assigned()) {
            if (checkNash(vars) == false) return ES_FAILED;
        }
        return ES_NOFIX;
    }
    //------------------------------------------------------------
    bool checkNash(ViewArray<IntView> vars) {
        int n = vars.size();
        for (int i=0; i<n; i++) {
            int utility;

            // Utility calculation
            Game* model1 = new Game();
            for (int j=0; j<n; j++) {
                model1->fixValue(j, vars[j].val());
            }
            model1->setGoal(i);
            model1->setBranchVars();
            model1->setBranchUtil();
            DFS<Game> engine1(model1);
            delete model1;
            if (Game* solution = engine1.next()) {
                utility = solution->getUtility();
                delete solution;
            }

            // Searching for at least one better response
            Game* model2 = new Game();
            for (int j=0; j<n; j++) {
                if (j!=i) 
                    model2->fixValue(j, vars[j].val());
            }
            model2->setGoal(i);
            model2->setPreference(utility);
            model2->setBranchVars();
            model2->setBranchUtil();
            DFS<Game> engine2(model2);
            delete model2;
            if (Game* better = engine2.next()) {
                delete better;
                return false;
            }
        }

        return true;
    }
};

//---------------------------------------------------------------------

void equilibrium(Space& home, const IntVarArgs& v) {
    ViewArray<IntView> vars(home,v);
    if (Equilibrium::post(home,vars) != ES_OK) home.fail();
}

//=====================================================================

int main(int argc, char const *argv[])
{
    Game* model = new Game();
    model->setBranchVars();
    // model->setBranchUtil();
    // model->setGoal(2);
    // model->setEquilibriumConstraint();
    DFS<Game> engine(model);
    delete model;

    while (Game* solution = engine.next()) {
        // solution->printAll();
        solution->print();
        delete solution;
    }
    return 0;
}
