/*
Original code by Marcin Pilipczuk
*/

#ifndef MINORFINDER_H

#include <cassert>
#include <set>
#include "namesdict.h"
#include "pathdecomp.h"
#include "graph.h"

using namespace std;

// #define USE_HOPS
#define USE_BRANCHES

class MinorFinder {
private:
    struct PartialMinor {
        int n;
        int arcs_edges, arcs_in_branches;
        vector<int> bag2branch;
        vector<MinorEdge> graph;
#ifdef USE_BRANCHES
        vector<vector<int> > branches;
#endif
#ifdef USE_HOPS
        vector<vector<int> > hops;
#endif
        int jump_edges() const {
            return arcs_edges + arcs_in_branches;
        }
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

        bool is_better(const PartialMinor &s) const {
            if (jump_edges() != s.jump_edges()) {
                return jump_edges() < s.jump_edges();
            }
            if (max_branch_set() != s.max_branch_set()) {
                return max_branch_set() < s.max_branch_set();
            }
            if (sum_branch_sets() != s.sum_branch_sets()) {
                return sum_branch_sets() < s.sum_branch_sets();
            }
            return false;
        }

        bool operator<(const PartialMinor &s) const {
            if (n != s.n) {
                return n < s.n;
            }
            if (bag2branch != s.bag2branch) {
                return bag2branch < s.bag2branch;
            }
#ifdef USE_HOPS
            if (graph != s.graph) {
                return graph < s.graph;
            }
            return hops < s.hops;
#else
            return graph < s.graph;
#endif
        }

        int get_cost() {
            int res = (int)graph.size();
#ifdef USE_HOPS
            for (const auto &v : hops) {
                res += (int)v.size();
            }
#endif
            return res;
        }

        int get_n() {
            int m = -1;
            for (auto i : bag2branch) {
                m = max(i, m);
            }
            return n - m - 1;
        }

        bool has_edge(int u, int v) const {
            for (const auto &me : graph) {
                if ((me.br_u == u && me.br_v == v) || (me.br_u == v && me.br_v == u)) {
                    return true;
                }
            }
            return false;
        }

        void add_edge(int u, int v, int edge_id) {
            if (u > v) {
                swap(u, v);
            }
            MinorEdge me(edge_id, u, v);
            graph.insert(std::upper_bound(graph.begin(), graph.end(), me), me);
        }

        void remap(vector<int> const &perm);
        void remap_to_bag();
        bool is_connected_to_bag() const;
        bool contains_bag_vertices() const;
    };
    set<PartialMinor> state;
    PathDecompBag bag;

    vector<PartialMinor> dump_state();
    void parse_node(const PathDecompNode &node);
    void parse_introduce_node(const string &name);
    void parse_forget_node(const string &name);
    void parse_introduce_edge(int u, int v, int edge_id);

    void canonize(PartialMinor &pm);
    void line_report(int node_id);
    void insert_pm(const PartialMinor &pm);
public:
    Graph graph;
    const PathDecomp &pd;
    const int max_cost, max_n, min_n, max_nonedges, min_deg;
    vector<Minor> found;

    MinorFinder(PathDecomp &_pd, int _max_cost, int _max_n, int _min_n, int _max_nonedges, int _min_deg) : pd(_pd), max_cost(_max_cost), max_n(_max_n), min_n(_min_n), max_nonedges(_max_nonedges), min_deg(_min_deg) {};
    void find();

    string print_minor(Minor &m) const;
};

#define MINORFINDER_H
#endif // MINORFINDER_H