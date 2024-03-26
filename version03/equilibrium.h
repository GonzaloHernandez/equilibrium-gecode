#include <gecode/int.hh>
#include <gecode/minimodel.hh>
#include "table.h"
#include "list.h"

using namespace Gecode;
using namespace Int;

//=====================================================================

void equilibrium(Space&, const IntVarArgs&);
void equilibriumtable(Space&, const IntVarArgs&);
void equilibriumplus(Space&, const IntVarArgs&);

//=====================================================================

class Game : public Space {
protected:
    IntVarArray vars;
    IntVar      util;
public:
    //------------------------------------------------------------
    Game();
    //------------------------------------------------------------
    Game(Game& source) : Space(source) {
        vars.update(*this, source.vars);
        util.update(*this, source.util);
    }
    //------------------------------------------------------------
    void setEquilibriumConstraint() {
        equilibriumplus(*this, vars);
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
    virtual void constrain(const Space& current) {
        const Game& candidate = static_cast<const Game&>(current);
        rel(*this, util, IRT_GR, candidate.util);
    }
    //------------------------------------------------------------
    void print() const;
    //------------------------------------------------------------
    void printAll() const {
        std::cout << vars << " " << util << std::endl;
    }
    //------------------------------------------------------------
    void fixValue(int i,int val) {
        rel(*this, vars[i], IRT_EQ, val);
    }
    //------------------------------------------------------------
    void fixAllValues(ViewArray<IntView>& vars) {
        for (int i=0; i<vars.size(); i++) {
            rel(*this, this->vars[i], IRT_EQ, vars[i].val());
        }
    }
    //------------------------------------------------------------
    void fixAllValuesExcept(ViewArray<IntView>& vars,int ei) {
        for (int i=0; i<vars.size(); i++) {
            if (i == ei) continue;
            rel(*this, this->vars[i], IRT_EQ, vars[i].val());
        }
    }
    //------------------------------------------------------------
    void setGoal(int);    
    //------------------------------------------------------------
    void setPreference(int val) {
        rel(*this, util, IRT_GR, val);
    }
    //------------------------------------------------------------
    void findEqualUtiliy(int val) {
        rel(*this, util, IRT_EQ, val);
    }
    //------------------------------------------------------------
    int getUtility() {
        return util.val();
    }
    //------------------------------------------------------------
    int getVal(int i) {
        return vars[i].val();
    }
    //------------------------------------------------------------
};


//=====================================================================
// Searching without storing
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
            // model1->setBranchVars();
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
            // model2->setBranchVars();
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
// Storing best responses using Table and List
//=====================================================================

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
        Game* model1 = new Game();
        for (int j=0; j<n; j++) {
            if (j!=i) 
                model1->fixValue(j, vars[j].val());
        }
        model1->setGoal(i);
        model1->setBranchUtil();
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
        Game* model2 = new Game();
        for (int j=0; j<n; j++) {
            if (j!=i) 
                model2->fixValue(j, vars[j].val());
        }
        model2->setGoal(i);
        model2->findEqualUtiliy(utility);
        model2->setBranchUtil();
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

//---------------------------------------------------------------------

void equilibriumtable(Space& home, const IntVarArgs& v) {
    ViewArray<IntView> vars(home,v);
    if (EquilibriumTable::post(home,vars) != ES_OK) home.fail();
}


//=====================================================================
// Filtering by lest cost posible.
// This propagator filters looking for one better response
//=====================================================================

class EquilibriumPlus : public Propagator {
protected:
    ViewArray<IntView> vars;
    int         n;
    List<int>   where;
    Table       utils;
    int*        mins;
public :
    //-----------------------------------------------------
    EquilibriumPlus(Space& home, ViewArray<IntView> v) 
    : Propagator(home), vars(v), n(vars.size()), utils(n,1)
    {
        vars.subscribe(home, *this, PC_INT_DOM);
        int cols = 1;
        mins = new int[n];
        for (int i=n-1; i>=0; i--) {
            for (int j=0; j<cols; j++) {
                int val[] = {-999};
                utils.add(i,val);
            }
            cols *= vars[i].size();
            mins[i] = vars[i].min();
        }
    }
    //-----------------------------------------------------
    static ExecStatus post(Space& home, ViewArray<IntView> v) {
        (void) new (home) EquilibriumPlus(home, v);
        return ES_OK;
    }
    //-----------------------------------------------------
    EquilibriumPlus(Space& home, EquilibriumPlus& source) 
    : Propagator(home,source), n(source.n), utils(source.utils)
    {
        vars.update(home, source.vars);
        where = source.where;
        utils = source.utils;
        mins  = source.mins;
    }
    //-----------------------------------------------------
    virtual Propagator* copy(Space& home) {
        return new (home) EquilibriumPlus(home, *this);
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
            analyseSubspace();
            if (checkNash(vars) == false) return ES_FAILED;
        }
        return ES_NOFIX;
    }
    //-----------------------------------------------------
    int analyseSubspace() {
        int i = 0;
        for ( ; i<n; i++) {
            if ( where.len() == i) {
                where.append(vars[i].val());
            }
            else if (vars[i].val() != where[i]) {
                for (int j=i; j<n; j++) {
                    where.set(j,vars[j].val());
                    for (int k=0; k<utils.len(i); k++) {
                        int** row = utils.getRow(i);
                        int val[] = {-999};
                        row[k] = val;
                    }
                }
                break;
            }
        }
        if (i == n) return -1;

        return i;
    }
    //-----------------------------------------------------
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
            DFS<Game> engine1(model1);
            delete model1;
            if (Game* solution = engine1.next()) {
                utility = solution->getUtility();
                delete solution;
            }

            int** row = utils.getRow(i);
            int ncol;
            if ( i==(n-1) ) {
                ncol = 0;
            }
            else {
                ncol = where[i+1]-mins[i+1];
            }

            if (utility < row[ncol][0]) {
                return false;
            }
            else {
                row[ncol][0] = utility;
            }
            // Searching for at least one better response
            Game* model2 = new Game();
            for (int j=0; j<n; j++) {
                if (j!=i) 
                    model2->fixValue(j, vars[j].val());
            }
            model2->setGoal(i);
            model2->setPreference(utility);
            DFS<Game> engine2(model2);
            delete model2;
            if (Game* better = engine2.next()) {
                row[ncol][0] = better->getUtility();
                delete better;
                return false;
            }
        }

        return true;
    }
};

//---------------------------------------------------------------------

void equilibriumplus(Space& home, const IntVarArgs& v) {
    ViewArray<IntView> vars(home,v);
    if (EquilibriumPlus::post(home,vars) != ES_OK) home.fail();
}
