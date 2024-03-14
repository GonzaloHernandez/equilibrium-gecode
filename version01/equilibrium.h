#include <gecode/int.hh>
#include "table.h"
#include "list.h"

using namespace Gecode;
using namespace Int;

//==============================================================

class Game : public Space {
protected:
    IntVarArray vars;
    IntVar      util;
public:
    //-----------------------------------------------------
    Game(int n) 
    : vars(*this, n, 0, n-1), util(*this, 1, n) 
    {
        branch(*this, vars, INT_VAR_NONE(), INT_VAL_MIN());
        branch(*this, util, INT_VAL_MIN());
    }
    //-----------------------------------------------------
    Game(Game& source) : Space(source) {
        vars.update(*this, source.vars);
        util.update(*this, source.util);
    }
    //-----------------------------------------------------
    virtual Space* copy() {
        return new Game(*this);
    }
    //-----------------------------------------------------
    virtual void constrain(const Space& current) {
        const Game& candidate = static_cast<const Game&>(current);
        rel(*this, util, IRT_GR, candidate.util);
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
        count(*this, vars, vars[i], IRT_EQ, util);
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
    void print() const {
        std::cout << vars << " " << util << std::endl;
    }
};


//==============================================================
// Filtering by lest cost posible.
// This propagator filters looking for one better response
//============================================================== 

class Equilibrium : public Propagator {
protected:
    ViewArray<IntView> vars;
public :
    //-----------------------------------------------------
    Equilibrium(Space& home, ViewArray<IntView> v) 
    : Propagator(home), vars(v) 
    {
        vars.subscribe(home, *this, PC_INT_DOM);
    }
    //-----------------------------------------------------
    static ExecStatus post(Space& home, ViewArray<IntView> v) {
        (void) new (home) Equilibrium(home, v);
        return ES_OK;
    }
    //-----------------------------------------------------
    Equilibrium(Space& home, Equilibrium& source) 
    : Propagator(home,source)
    {
        vars.update(home, source.vars);
    }
    //-----------------------------------------------------
    virtual Propagator* copy(Space& home) {
        return new (home) Equilibrium(home, *this);
    }
    //-----------------------------------------------------
    virtual void reschedule(Space& home) {
        vars.reschedule(home, *this, PC_INT_DOM);
    }
    //-----------------------------------------------------
    virtual PropCost cost(const Space&, const ModEventDelta&) const {
        return PropCost::binary(PropCost::LO);
    } 
    //-----------------------------------------------------
    virtual ExecStatus propagate(Space& home, const ModEventDelta&) {
        if (vars.assigned()) {
            if (checkNash(vars) == false) return ES_FAILED;
        }
        return ES_NOFIX;
    }
    //-----------------------------------------------------
    bool checkNash(ViewArray<IntView> vars) {
        int n = vars.size();
        for (int i=0; i<n; i++) {
            int utility;

            // Utility calculation
            Game* model1 = new Game(n);
            for (int j=0; j<n; j++) {
                model1->fixValue(j, vars[j].val());
            }
            model1->setGoal(i);
            DFS<Game> engine1(model1);
            delete model1;
            if (Game* solution = engine1.next()) {
                utility = solution->getUtility();
                delete solution;
            }

            // Searching for at least one better response
            Game* model2 = new Game(n);
            for (int j=0; j<n; j++) {
                if (j!=i) 
                    model2->fixValue(j, vars[j].val());
            }
            model2->setGoal(i);
            model2->setPreference(utility);
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

//==============================================================

void equilibrium(Space& home, const IntVarArgs& v) {
    ViewArray<IntView> vars(home,v);
    if (Equilibrium::post(home,vars) != ES_OK) home.fail();
}


//==============================================================
// Storing best responses using Table and List
//==============================================================

class EquilibriumTable : public Propagator {
protected:
    ViewArray<IntView>  vars;
    int         n;
    Table       brs;   // best responses
    List<int>   cnt;   // counters
    List<int>   spc;   // current subspace
public :
    //-----------------------------------------------------
    EquilibriumTable(Space& home, ViewArray<IntView> v) 
    : Propagator(home), vars(v), n(vars.size()), brs(n,n)
    {
        vars.subscribe(home, *this, PC_INT_DOM);
        for (int i=0; i<n; i++) {
            cnt.append(-999);
        }
        cnt.toStr();
        dataToStr(nullptr,0);
    }
    //-----------------------------------------------------
    static ExecStatus post(Space& home, ViewArray<IntView> v) {
        (void) new (home) EquilibriumTable(home, v);
        return ES_OK;
    }
    //-----------------------------------------------------
    EquilibriumTable(Space& home, EquilibriumTable& source) 
    : Propagator(home,source), n(source.n), brs(source.brs)
    {
        vars.update(home, source.vars);
        cnt = source.cnt;
        spc = source.spc;
    }
    //-----------------------------------------------------
    virtual Propagator* copy(Space& home) {
        return new (home) EquilibriumTable(home, *this);
    }
    //-----------------------------------------------------
    virtual void reschedule(Space& home) {
        vars.reschedule(home, *this, PC_INT_DOM);
    }
    //-----------------------------------------------------
    virtual PropCost cost(const Space&, const ModEventDelta&) const {
        return PropCost::binary(PropCost::LO);
    } 
    //-----------------------------------------------------
    virtual ExecStatus propagate(Space& home, const ModEventDelta&) {
        List<int> nsp;
        for (int i=0; i<n; i++) {
            if (vars[i].assigned())
                nsp.append(vars[i].val());
        }

        analyseSubscpace(nsp);

        if (vars.assigned()) {

            int t[n];
            for (int i=0; i<n; i++) {
                t[i] = vars[i].val();
            }

            if (! checkNash(t)) 
                return ES_FAILED;
        }
        return ES_NOFIX;
    }
    //-----------------------------------------------------
    void analyseSubscpace(const List<int>& nsp) {
        int i = 0;

        while (i < nsp.len() && i < spc.len()) {
            int nv = nsp[i];
            int cv = spc[i];
            if (nv != cv) break;
            i++;
        }
        if (i == nsp.len() || i == n) return;

        if (cnt[i] == -999) {
            brs.empty(i);
            cnt.set(i,1);
            for (int j=i+1; j<n; j++) {
                cnt.set(i, cnt[i]*n);
                brs.empty(j);
                cnt.set(j,-999);
            }
        }
        else {
            for (int j=i+1; j<n; j++) {
                brs.empty(j);
                cnt.set(j,-999);
            }
        }

        spc.copy(nsp);
    }
    //-----------------------------------------------------
    bool checkNash(int* t) {
        for (int i=n-1; i>=0; i--) {
            if (! isBestResponseInTable(t,i)) {
                if (cnt[i] <= 0) return false;
                if (! isBestResponseNew(t,i)) return false;
            }
        }
        return true;
    }
    //-----------------------------------------------------
    bool isBestResponseInTable(int* t, int i) {
        if (brs.exists(i,t)) return true;
        return false;
    }
    //-----------------------------------------------------
    bool isBestResponseNew(int* t, int i) {

        // Looking for a best response
        Game* model1 = new Game(n);
        for (int j=0; j<n; j++) {
            if (j!=i) 
                model1->fixValue(j, vars[j].val());
        }
        model1->setGoal(i);
        BAB<Game> engine(model1);
        delete model1;
        Game* best = nullptr;
        while (Game* solution = engine.next()) {
            if (best) delete best;
            best = solution;
        }

        int utility = best->getUtility();
        delete best;
        bool isBestResponse = false;

        // Looking for all best responses
        Game* model2 = new Game(n);
        for (int j=0; j<n; j++) {
            if (j!=i) 
                model2->fixValue(j, vars[j].val());
        }
        model2->setGoal(i);
        model2->findEqualUtiliy(utility);
        DFS<Game> engine2(model2);
        delete model2;
        while (Game* solution = engine2.next()) {
            int r[n];
            bool areEquals = true;
            for (int j=0; j<n; j++) {
                r[j] = solution->getVal(j);
                if (r[j] != t[j]) {
                    areEquals = false;
                }
            }
            if (areEquals) isBestResponse = true;
            delete solution;

            saveResponseInTable(r,i);
        }

        cnt.set(i, cnt[i]-1);
        
        return isBestResponse;
    }
    //-----------------------------------------------------
    void saveResponseInTable(int* r, int i) {
        brs.add(i,r);
    }
};

//==============================================================

void equilibriumTable(Space& home, const IntVarArgs& v) {
    ViewArray<IntView> vars(home,v);
    if (EquilibriumTable::post(home,vars) != ES_OK) home.fail();
}