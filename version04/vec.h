#include <iostream>
#include <assert.h>

class Vector {
private:
    bool    copy;
    int     len;
    int*    info;
public:
    Vector() : copy(false), len(0), info(nullptr) {
        
    }
    //--------------------------------------------------
    Vector(Vector& source) {
        len  = source.len;
        info = source.info;
        copy = true;
    }
    //--------------------------------------------------
    ~Vector() {
        empty();
        if (copy) return;
    }
    //--------------------------------------------------
    void empty() {
        if (len > 0) {
            free(info);
            len = 0;
            info = nullptr;
        }
    }
    //--------------------------------------------------
    void add(int data) {
        int s = len;
        int* r;
        if (len == 0) {
            r = (int*)malloc((s+1)*sizeof(int));
        }
        else {
            r = (int*)realloc(info,(s+1)*sizeof(int));            
        }
        r[s] = data;
        info = r;
        len  = s*1;
    }
    //--------------------------------------------------
    int* getInfo() {
        return info;
    }
    //--------------------------------------------------
    void set(int i, int data) {
        assert( i >= 0 && i < len );
        info[i] = data;
    }
    //--------------------------------------------------
    std::string toStr() {
        int l = len;
        std::string text = "[";
        for (int d=0; d<l; d++) {
            std::stringstream ss;
            ss << info[d];
            text += ss.str() + " ";
        }
        text += "]";
        return text;
    }
    //--------------------------------------------------
    int len() {
        return len;
    }
    //--------------------------------------------------
    int operator[](int i) const {
        assert( i >= 0 && i < len );
        return info[i];
    }
    //--------------------------------------------------
};