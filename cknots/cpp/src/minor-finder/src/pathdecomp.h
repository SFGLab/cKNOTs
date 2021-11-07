/*
Original code by Marcin Pilipczuk
*/

#ifndef PATHDECOMP_H

#include <string>
#include <vector>
#include <iostream>
#include <algorithm>

enum DecompNodeType {
    INTRODUCE_NODE = 1,
    FORGET_NODE = 2,
    INTRODUCE_EDGE = 3
};

struct PathDecompNode {
    DecompNodeType type;
    std::string name;
    int u, v, a, b;

    PathDecompNode(DecompNodeType _type, std::string _name) : type(_type), name(_name) {};
    PathDecompNode(int _u, int _v, int _a, int _b) : type(INTRODUCE_EDGE), u(_u), v(_v), a(_a), b(_b) {};
};

struct PathDecompBag {
    std::vector<std::string> bag;

    PathDecompBag() {};
    void clear() {
        bag.clear();
    }
    void step(const PathDecompNode &n) {
        switch(n.type) {
            case INTRODUCE_NODE:
                bag.push_back(n.name);
                break;
            case INTRODUCE_EDGE:
                break;
            case FORGET_NODE:
                auto it = std::find(bag.begin(), bag.end(), n.name);
                assert(it != bag.end());
                bag.erase(it);
                break;
        }
    }
    int find(std::string const &name) {
        auto r = std::find(bag.begin(), bag.end(), name);
        if (r == bag.end()) {
            assert(false);
            return -1;
        } else {
            return (int)(r - bag.begin());
        }
    }
    std::string operator[](int i) {
        return bag[i];
    }

    int size() {
        return (int)bag.size();
    }

};

class PathDecomp {
public:
    std::vector<PathDecompNode> decomposition;
    PathDecomp() {};

    void read() {
        std::string tmp;
        while (std::cin >> tmp) {
            if (tmp == "INTRODUCE_NODE") {
                std::cin >> tmp;
                decomposition.push_back(PathDecompNode(INTRODUCE_NODE, tmp));
            } else if (tmp == "INTRODUCE_EDGE") {
                int u, v, p, q;
                std::cin >> u >> v >> p >> q;
                decomposition.push_back(PathDecompNode(u, v, p, q));
            } else if (tmp == "FORGET_NODE") {
                std::cin >> tmp;
                decomposition.push_back(PathDecompNode(FORGET_NODE, tmp));
            }
        }
    }

    void print_decomposition() {
        PathDecompBag bag;
        for (auto n : decomposition) {
            switch(n.type) {
                case INTRODUCE_NODE:
                    std::cout << "INTRODUCE_NODE " << n.name << std::endl;
                    break;
                case INTRODUCE_EDGE:
                    std::cout << "INTRODUCE_EDGE " << n.u << " " << n.v << " " << n.a << " " << n.b
                              << std::endl;
                    break;
                case FORGET_NODE:
                    std::cout << "FORGET_NODE " << n.name << std::endl;
                    break;
            }
            bag.step(n);
        }
    }

    int width() {
        PathDecompBag bag;
        int res = bag.size();
        for (auto n : decomposition) {
            bag.step(n);
            if (res < bag.size()) {
                res = bag.size();
            }
        }
        assert(bag.size() == 0);
        return res;
    }
};

#define PATHDECOMP_H
#endif // PATHDECOMP_H