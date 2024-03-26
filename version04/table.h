#include <cstdlib>
#include <assert.h>
#include <iostream>

class Table {
private:
    int     nrows;      // Number of rows
    int     size;       // Size of the tuples
    int     *lens;      // Lengths of each row
    int     ***info;    // Information / Payload
    bool    copy;       // Is this a copy? (or is the original)
public:
    Table(int nrows,int size) : nrows(nrows), size(size) {
        info    = (int***)malloc(nrows*sizeof(int**));
        // info    = new int**[nrows];
        lens   = (int*)malloc(nrows*sizeof(int));
        // lens   = new int[nrows];
        for (int i=0; i<nrows; i++) {
            info[i]     = nullptr;
            lens[i]    = 0;
        }
        copy = false;
        toStr(0);
    }
    //--------------------------------------------------
    Table(Table& source) {
        nrows   = source.nrows;
        size    = source.size;
        lens    = source.lens;
        info    = source.info;
        copy    = true;
    }
    //--------------------------------------------------
    ~Table() {
        if (copy) return;
        for (int i=0; i<nrows; i++) {
            empty(i);
        }
        free(info);
        // delete info;
        free(lens);
        // delete lens;
    }
    //--------------------------------------------------
    void add(int row,int* data) {
        assert (row >= 0 && row < nrows);

        int s = lens[row]+1;
        int**r;
        // r = new int*[s];
        if (lens[row] == 0) {
            r  = (int**)malloc(s*sizeof(int*));
        }
        else {
            r = (int**)realloc(info[row],s*sizeof(int*));
            // for(int i=0; i<s-1; i++) {
            //     r[i] = info[row][i];
            // }
        }
        info[row] = r;

        int* t = (int*)malloc(size*sizeof(int));
        // int* t = new int[size];

        info[row][s-1] = t;
        for (int i=0; i<size; i++) {
            info[row][s-1][i] = data[i];
        }
        lens[row] = s;
    }
    //--------------------------------------------------
    void empty(int row) {
        assert (row >= 0 && row < nrows);
        if (lens[row] > 0) {
            for(int i=0; i<lens[row]; i++) {
                free(info[row][i]);
                // delete info[row][i];
            }
            free(info[row]);
            // delete info[row];
            lens[row] = 0;
        }
    }
    //--------------------------------------------------
    int*** getInfo() {
        return info;
    }
    //--------------------------------------------------
    int** getRow(int r) {
        return info[r];
    }
    //--------------------------------------------------
    bool exists(int row, int* data) {
        assert (row >= 0 && row < nrows);
        for (int d=0; d<lens[row]; d++) {
            int i;
            for (i=0; i<size; i++) {
                if (data[i] != info[row][d][i]) {
                    break;
                }
            }
            if (i==size) return true;
        }
        return false;
    }
    //--------------------------------------------------
    int len(int row) {
        assert (row >= 0 && row < nrows);
        return lens[row];
    }
    //--------------------------------------------------
    std::string toStr(int row) {
        int l = lens[row];
        std::string text = "{";
        for (int d=0; d<l; d++) {
            for (int i=0; i<size; i++) {
                std::stringstream ss;
                ss << info[row][d][i];
                text += ss.str() + " ";
            }
            text += ",";
        }
        text += "}";
        return text;
    }
};

//=======================================================


std::string dataToStr(int* data,int n) {
    std::string text = "{";
    for (int i=0; i<n; i++) {
        std::stringstream ss;
        ss << data[i];
        text += ss.str() + " ";
    }
    text += "}";
    return text;
}