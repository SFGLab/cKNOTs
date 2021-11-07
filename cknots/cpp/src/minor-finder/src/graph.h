/*
Original code by Marcin Pilipczuk
*/

#ifndef GRAPH_H
#define GRAPH_H

#include <iostream>
#include "namesdict.h"

// #define COMPARE_EDGE_ID

using namespace std;

class Edge {
public:
    int u, v, p, q;
    Edge(int _u, int _v, int _p, int _q) : u(_u), v(_v), p(_p), q(_q) {};

    bool operator<(const Edge &e) const {
        if (u != e.u) {
            return u < e.u;
        }
        if (v != e.v) {
            return v < e.v;
        }
        if (p != e.p) {
            return p < e.p;
        }
        return q < e.q;
    }

    int first() const {
        return min(u, v);
    }
    int second() const {
        return max(u, v);
    }
};

class Graph {
public:
    NamesDict namesdict;
    vector<Edge> edge_list;
    vector<vector<int> > inc_list;

    Graph() {};

    int find_node(string const &name) {
        int id = namesdict.name2id(name);
        while (id >= (int)inc_list.size()) {
            inc_list.push_back(vector<int>());
        }
        return id;
    }

    int add_edge(string const &u, string const &v, int p, int q) {
        int uid = find_node(u);
        int vid = find_node(v);
        int eid = (int)edge_list.size();
        edge_list.push_back(Edge(uid, vid, p, q));
        inc_list[uid].push_back(eid);
        inc_list[vid].push_back(eid);
        return eid;
    }

    void read_parsed_txt() {
        // Reads .txt file from parsed/ directory
        // Assumes NODE commands are in the correct order and does NOT add the edges of the main strand
        // The main strand consists of nodes exactly in the order of their indices
        string tmp;
        while(std::cin >> tmp) {
            if (tmp == "NODE") {
                std::cin >> tmp;
                find_node(tmp);
            } else if (tmp == "EDGE") {
                string a, b; int p, q;
                std::cin >> a >> b >> p >> q;
                add_edge(a, b, p, q);
            }
        }
    }

    int get_n() {
        return (int)inc_list.size();
    }

    int get_m() {
        return (int)edge_list.size();
    }
};


struct MinorEdge {
    int edge_id;
    int br_u, br_v;
    bool solid;

    MinorEdge(int _edge_id, int _br_u, int _br_v, bool _solid = false) : edge_id(_edge_id), br_u(_br_u), br_v(_br_v), solid(_solid) {};

    bool operator<(const MinorEdge &me) const {
        if (br_u != me.br_u) {
            return br_u < me.br_u;
        }
        if (br_v != me.br_v) {
            return br_v < me.br_v;
        }
        if (solid != me.solid) {
            return solid < me.solid;
        }
        if (solid && (edge_id != me.edge_id)) {
            return edge_id < me.edge_id;
        }
#ifdef COMPARE_EDGE_ID
        return edge_id < me.edge_id;
#else
        return false;
#endif
    }

    bool operator==(const MinorEdge &me) const {
#ifdef COMPARE_EDGE_ID
        return solid == me.solid && br_u == me.br_u && br_v == me.br_v && edge_id == me.edge_id;
#else
        return solid == me.solid && br_u == me.br_u && br_v == me.br_v && (!solid || (edge_id == me.edge_id));
#endif
    }
};

class Minor {
public:
    Graph &graph;
    vector<vector<int> > branches;
    vector<MinorEdge> edges;
    int arcs_edges, arcs_in_branches;

    explicit Minor(Graph &_graph) : graph(_graph) {}

    int sum_branch_sets() const {
        int res = 0;
        for (auto &x : branches) {
            res += (int)x.size();
        }
        return res;
    }

    int max_branch_set() const {
        int res = 0;
        for (auto &x : branches) {
            res = max(res, (int)x.size());
        }
        return res;
    }
};
#endif // GRAPH_H
