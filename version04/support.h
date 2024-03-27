#include <cstdlib>
#include <assert.h>
#include <iostream>

class Table {
private:
    int     nrows;  // Number of rows
    int     *lens;  // Lengths of each row
    int     **info; // Information / Payload
    bool    copy;   // Is this a copy? (or is the original)
public:
    Table(int nrows) : nrows(nrows) {
        info    = (int**)malloc(nrows*sizeof(int*));
        lens   = (int*)malloc(nrows*sizeof(int));
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
        free(lens);
    }
    //--------------------------------------------------
    void add(int row,int data) {
        assert (row >= 0 && row < nrows);

        int s = lens[row];
        int*r;
        if (lens[row] == 0) {
            r  = (int*)malloc((s+1)*sizeof(int));
        }
        else {
            r = (int*)realloc(info[row],(s+1)*sizeof(int));
        }
        r[s]        = data;

        info[row]   = r;
        lens[row]   = s+1;
    }
    //--------------------------------------------------
    void empty(int row) {
        assert (row >= 0 && row < nrows);
        if (lens[row] > 0) {
            free(info[row]);
            lens[row] = 0;
        }
    }
    //--------------------------------------------------
    int** getInfo() {
        return info;
    }
    //--------------------------------------------------
    int* getRow(int r) {
        return info[r];
    }
    //--------------------------------------------------
    bool exists(int row, int data) {
        assert (row >= 0 && row < nrows);
        for (int d=0; d<lens[row]; d++) {
            if (data == info[row][d]) {
                return true;
            }
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
            std::stringstream ss;
            ss << info[row][d];
            text += ss.str() + " ";
        }
        text += "}";
        return text;
    }
};

//=======================================================

class Vector {
private:
    bool    copy;
    int     len;
    int**   info;
public:
    Vector() : copy(false), len(0)  {
        info = (int**)malloc(sizeof(int*));
    }
    //--------------------------------------------------
    Vector(Vector& source) {
        len  = source.len;
        info = source.info;
        copy = true;
    }
    //--------------------------------------------------
    ~Vector() {
        if (copy) return;
        empty();
        free(info);
    }
    //--------------------------------------------------
    void append(int data) {
        int s = len;
        int* r;
        if (len == 0) {
            r = (int*)malloc((s+1)*sizeof(int));
        }
        else {
            r = (int*)realloc((*info),(s+1)*sizeof(int));            
        }
        r[s]    = data;
        (*info) = r;
        len     = s+1;
    }
    //--------------------------------------------------
    void empty() {
        if (len > 0) {
            free(*info);
            len = 0;
            (*info) = nullptr;
        }
    }
    //--------------------------------------------------
    int* getInfo() {
        return (*info);
    }
    //--------------------------------------------------
    void set(int i, int data) {
        assert( i >= 0 && i < len );
        (*info)[i] = data;
    }
    //--------------------------------------------------
    std::string toStr() {
        int l = len;
        std::string text = "[";
        for (int d=0; d<l; d++) {
            std::stringstream ss;
            ss << (*info)[d];
            text += ss.str() + " ";
        }
        text += "]";
        return text;
    }
    //--------------------------------------------------
    int lenght() {
        return len;
    }
    //--------------------------------------------------
    int operator[](int i) {
        assert( i >= 0 && i < len );
        return (*info)[i];
    }
    //--------------------------------------------------
};