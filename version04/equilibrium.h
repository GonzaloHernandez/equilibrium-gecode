#include <gecode/int.hh>
#include <gecode/minimodel.hh>
#include "table.h"
#include "list.h"

using namespace Gecode;
using namespace Int;

//=====================================================================

void equilibriumplus(Space&, const IntVarArgs&, const IntVarArgs&);

//=====================================================================

class Game : public Space {
protected:
    IntVarArray vars;
    IntVarArray util;
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
        equilibriumplus(*this, vars, util);
    }
    //------------------------------------------------------------
    void setBranchVars() {
        branch(*this, vars, INT_VAR_NONE(), INT_VAL_MIN());
    }
    //------------------------------------------------------------
    void setBranchUtil() {
        branch(*this, util, INT_VAR_NONE(), INT_VAL_MIN());
    }
    //------------------------------------------------------------
    virtual Space* copy() {
        return new Game(*this);
    }
    //------------------------------------------------------------
    virtual void constrain(int i, const Space& current) {
        const Game& candidate = static_cast<const Game&>(current);
        rel(*this, util[i], IRT_GR, candidate.util[i]);
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
    void setPreference(int i, int val) {
        rel(*this, util[i], IRT_GR, val);
    }
    //------------------------------------------------------------
    void findEqualUtiliy(int i, int val) {
        rel(*this, util[i], IRT_EQ, val);
    }
    //------------------------------------------------------------
    int getUtility(int i) {
        return util[i].val();
    }
    //------------------------------------------------------------
    int getVal(int i) {
        return vars[i].val();
    }
    //------------------------------------------------------------
};

//=====================================================================
// Filtering by lest cost posible.
// This propagator filters looking for one better response
//=====================================================================

class EquilibriumPlus : public Propagator {
protected:
    ViewArray<IntView> vars;
    ViewArray<IntView> util;
    int         n;
    List<int>   where;
    Table       bests;
    int*        delta;
public :
    //-----------------------------------------------------
    EquilibriumPlus(Space& home, ViewArray<IntView> v, ViewArray<IntView> u) 
    : Propagator(home), vars(v), util(u), n(vars.size()), bests(n)
    {
        vars.subscribe(home, *this, PC_INT_DOM);
        util.subscribe(home, *this, PC_INT_DOM);
        delta = new int[n];
        int cols = 1;
        for (int i=n-1; i>=0; i--) {
            for (int j=0; j<cols; j++) {
                bests.add(i,-999);
            }
            cols *= vars[i].size();
            delta[i] = vars[i].min();
        }
        varsToStr();
    }
    //-----------------------------------------------------
    static ExecStatus post(Space& home, ViewArray<IntView> v, ViewArray<IntView> u) {
        (void) new (home) EquilibriumPlus(home, v, u);
        return ES_OK;
    }
    //-----------------------------------------------------
    EquilibriumPlus(Space& home, EquilibriumPlus& source) 
    : Propagator(home,source), n(source.n), bests(source.bests)
    {
        vars.update(home, source.vars);
        util.update(home, source.util);
        where = source.where;
        bests = source.bests;
        delta = source.delta;
    }
    //-----------------------------------------------------
    virtual Propagator* copy(Space& home) {
        return new (home) EquilibriumPlus(home, *this);
    }
    //-----------------------------------------------------
    virtual void reschedule(Space& home) {
        vars.reschedule(home, *this, PC_INT_DOM);
        util.reschedule(home, *this, PC_INT_DOM);
    }
    //-----------------------------------------------------
    virtual PropCost cost(const Space&, const ModEventDelta&) const {
        return PropCost::binary(PropCost::LO);
    } 
    //-----------------------------------------------------
    virtual ExecStatus propagate(Space& home, const ModEventDelta&) {
        for (int i=0; i<n; i++) {
            if (util[i].gq(home, minBest(i)) == Int::ME_INT_FAILED)
                return ES_FAILED;
        }

        if (vars.assigned() && util.assigned()) {
            analyseSubspace();
            if (checkNash() == false) return ES_FAILED;
        }
        return ES_NOFIX;
    }
    //-----------------------------------------------------
    int minBest(int i) {
        int* row = bests.getRow(i);
        int found = row[0];
        for (int j=1; j<bests.len(i); j++) {
            if (row[j] < found) {
                found = row[j];
            }
        }
        return found;
    }
    //-----------------------------------------------------
    std::string varsToStr() {
        std::string text = "[";
        for (int i=0; i<n; i++) {
            std::stringstream ss;
            if (vars[i].assigned()) {
                ss << vars[i].min();
            }
            else {
                ss << vars[i].min()<<".."<<vars[i].max();
            }
            text += ss.str() + " ";
        }
        text += "] [";

        for (int i=0; i<n; i++) {
            std::stringstream ss;
            if (vars[i].assigned()) {
                ss << util[i].min();
            }
            else {
                ss << util[i].min()<<".."<<util[i].max();
            }
            text += ss.str() + " ";
        }
        text += "]";

        return text;
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
                    for (int k=0; k<bests.len(i); k++) {
                        int* row = bests.getRow(i);
                        row[k] = -999;
                    }
                }
                break;
            }
        }
        if (i == n) return -1;

        return i;
    }
    //-----------------------------------------------------
    bool checkNash() {
        for (int i=0; i<n; i++) {
            int utility = util[i].val();

            int* row = bests.getRow(i);
            int ncol;

            if ( i==(n-1) ) ncol = 0;
            else            ncol = where[i+1] - delta[i+1];

            if (utility < row[ncol]) {
                return false;
            }
            else {
                row[ncol] = utility;
            }

            // Searching for at least one better response
            Game* model = new Game();
            for (int j=0; j<n; j++) {
                if (j!=i) 
                    model->fixValue(j, vars[j].val());
            }
            model->setPreference(i,utility);
            DFS<Game> engine(model);
            delete model;
            if (Game* better = engine.next()) {
                row[ncol] = better->getUtility(i);
                delete better;
                return false;
            }
        }

        return true;
    }
};

//---------------------------------------------------------------------

void equilibriumplus(Space& home, const IntVarArgs& v, const IntVarArgs& u) {
    ViewArray<IntView> vars(home,v);
    ViewArray<IntView> util(home,u);
    if (EquilibriumPlus::post(home,vars,util) != ES_OK) home.fail();
}
