/*
Original code by Marcin Pilipczuk
*/

#ifndef TOOLS_H
#define TOOLS_H

#include <vector>

using namespace std;

class FAU {
private:
    vector<int> fau;

public:
    explicit FAU(int n) : fau(n, -1) {};

    int find(int x) {
        return fau[x] < 0 ? x : fau[x] = find(fau[x]);
    }

    bool join(int x, int y) {
        x = find(x);
        y = find(y);
        if (x == y) {
            return false;
        }
        if (fau[x] > fau[y]) {
            swap(x, y);
        } else if (fau[x] == fau[y]) {
            fau[x]--;
        }
        fau[y] = x;
        return true;
    }
};

#endif