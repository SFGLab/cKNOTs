/*
Original code by Marcin Pilipczuk
*/

#ifndef CHROMATYNA_K6FINDER_H
#define CHROMATYNA_K6FINDER_H

#include <cassert>
#include <set>
#include <iomanip>
#include "namesdict.h"
#include "pathdecomp.h"
#include "graph.h"

using namespace std;

class BridgeEdge {
public:
    int id;
    bool solid;

    BridgeEdge() : id(-1), solid(false) {};
    BridgeEdge(int _id, bool _solid) : id(_id), solid(_solid) {};
    bool operator<(const BridgeEdge &be) const {
        return make_pair(id, solid) < make_pair(be.id, be.solid);
    }
    bool operator==(const BridgeEdge &be) const {
        return id == be.id && solid == be.solid;
    }
};

class BranchPath {
public:
    unsigned color;  // intended endpoints
    int v1, v2; // real endpoints being indices in a bag; -1 means corresponding intended endpoint
    bool solid;

    BranchPath(unsigned br, unsigned u1, unsigned u2, int _v1, int _v2, bool _solid) : color(params2color(br, u1, u2)), v1(_v1), v2(_v2), solid(_solid) {}

    bool operator<(const BranchPath &bp) const {
        return make_pair(make_pair(color, solid), make_pair(v1, v2)) < make_pair(make_pair(bp.color, solid), make_pair(bp.v1, bp.v2));
    }

    bool operator==(const BranchPath &bp) const {
        return color == bp.color && solid == bp.solid && bp.v1 == bp.v1 && bp.v2 == bp.v2;
    }

    static inline unsigned color2branch(unsigned c) { return c >> 6u; }
    static inline unsigned color2u1(unsigned c) { return (c >> 3u) & 7u; }
    static inline unsigned color2u2(unsigned c) { return c & 7u; }
    static inline unsigned params2color(unsigned br, unsigned u1, unsigned u2) {
        return (br << 6u) | (min(u1, u2) << 3u) | max(u1, u2);
    }

    inline unsigned get_branch() { return color2branch(color); }
    inline unsigned get_u1() { return color2u1(color); }
    inline unsigned get_u2() { return color2u2(color); }
};

class PartialK6 {
public:
    vector<bool> forgotten;
    vector<int> bag2branch;
    vector<BridgeEdge> bridge_edges;
    vector<BranchPath> paths;
    vector<int> bridge_endpoints;
    set<int> solid_edges;
    vector<vector<int> > branch_sets;
    int total_score;

    PartialK6() : forgotten(6, false), bridge_edges(15), bridge_endpoints(6, -1), branch_sets(6) { total_score = 0; }

    static inline int bridgeEdgeIndex(int a, int b) {
        if (a > b) {
            swap(a, b);
        }
        assert(a < b && a >= 0 && b <= 5);
        return b*(b-1)/2 + a;
    }
    BridgeEdge getBridgeEdge(int a, int b) const {
        return bridge_edges[bridgeEdgeIndex(a, b)];
    }
    void setBridgeEdge(int a, int b, int id, bool solid) {
        bridge_edges[bridgeEdgeIndex(a, b)] = BridgeEdge(id, solid);
        if (solid) {
            solid_edges.insert(id);
        }
    }

    bool operator<(const PartialK6 &pk) const {
        if (bag2branch != pk.bag2branch) {
            return bag2branch < pk.bag2branch;
        }
        if (forgotten != pk.forgotten) {
            return forgotten < pk.forgotten;
        }
        if (paths != pk.paths) {
            return paths < pk.paths;
        }
        if (bridge_edges != pk.bridge_edges) {
            return bridge_edges < pk.bridge_edges;
        }
        return solid_edges < pk.solid_edges;
    }

    bool is_better(const PartialK6 &pk) const {
        return total_score < pk.total_score;
    }

    bool add_bridge_endpoint(unsigned a, unsigned b, int v) {
        bool ret = true;
        for (BranchPath &bp : paths) {
            if (bp.get_branch() == a) {
                if (bp.get_u1() == b && bp.v1 == v) {
                    bp.v1 = -1;
                    if (bp.v2 == -1 && count_paths_by_color(a, b, bp.get_u2()) > 1) {
                        ret = false;
                    }
                }
                if (bp.get_u2() == b && bp.v2 == v) {
                    bp.v2 = -1;
                    if (bp.v1 == -1 && count_paths_by_color(a, bp.get_u1(), b) > 1) {
                        ret = false;
                    }
                }
            }
        }
        return ret;
    }

    int find_path_by_endpoint(unsigned a, unsigned b, unsigned c, int v) {
        unsigned color = BranchPath::params2color(a, b, c);
        for (unsigned i = 0; i < paths.size(); ++i) {
           if (paths[i].color == color && (paths[i].v1 == v || paths[i].v2 == v)) {
               return (int)i;
           }
        }
        return -1;
    }
    int count_paths_by_color(unsigned a, unsigned b, unsigned c) {
        unsigned color = BranchPath::params2color(a, b, c);
        int cnt = 0;
        for (unsigned i = 0; i < paths.size(); ++i) {
            if (paths[i].color == color) {
                cnt++;
            }
        }
        return cnt;
    }

    bool try_merge_paths(unsigned a, unsigned b, unsigned c, int u, int v, int id, bool solid) {
        int iu = find_path_by_endpoint(a, b, c, u);
        if (iu < 0) {
            return false;
        }
        int iv = find_path_by_endpoint(a, b, c, v);
        if (iv < 0 || iu == iv) {
            return false;
        }
        if (paths[iu].v1 == u && paths[iv].v1 == v) {
            return false;
        }
        if (paths[iu].v2 == u && paths[iv].v2 == v) {
            return false;
        }
        if (paths[iu].v1 == u) {
            swap(iu, iv);
            swap(u, v);
        }
        assert(paths[iu].v2 == u);
        assert(paths[iv].v1 == v);
        // Check if there are other paths with the same colors if we end up a path
        if (paths[iu].v1 == -1 && paths[iv].v2 == -1 && count_paths_by_color(a, b, c) > 2) {
            return false;
        }
        paths[iu].v2 = paths[iv].v2;
        paths[iu].solid = (paths[iu].solid || solid || paths[iv].solid);
        paths.erase(paths.begin() + iv);
        sort(paths.begin(), paths.end());
        if (solid) {
            solid_edges.insert(id);
        }
        return true;
    }

    int find_finished_path(unsigned a, unsigned b, unsigned c) const {
        unsigned color = BranchPath::params2color(a, b, c);
        for (unsigned i = 0; i < paths.size(); ++i) {
            const BranchPath &bp = paths[i];
            if (bp.color == color && bp.v1 == -1 && bp.v2 == -1) {
                return (int)i;
            }
        }
        return -1;
    }

    bool is_path_finished(unsigned a, unsigned b, unsigned c) const {
        return find_finished_path(a, b, c) >= 0;
    }

    bool check_triangles(unsigned a) const {
        for (unsigned b = 0; b < 5u; ++b) {
            if (a != b) {
                for (unsigned c = b + 1u; c < 6u; ++c) {
                    if (a != c) {
                        int ia = find_finished_path(a, b, c);
                        int ib = find_finished_path(b, c, a);
                        int ic = find_finished_path(c, a, b);
                        if (ia >= 0 && ib >= 0 && ic >= 0) {
                            if (!(paths[ia].solid || paths[ib].solid || paths[ic].solid || getBridgeEdge(a, b).solid ||
                                    getBridgeEdge(b, c).solid || getBridgeEdge(c, a).solid)) {
                                return false;
                            }
                        }
                    }
                }
            }
        }
        return true;
    }
};

class K6Finder {
    set<PartialK6> state;
    PathDecompBag bag;

    vector<PartialK6> dump_state() {
        vector<PartialK6> old;
        old.insert(old.end(), make_move_iterator(state.begin()), make_move_iterator(state.end()));
        state.clear();
        return old;
    }

    void insert_pk(const PartialK6 &pk) {
        auto it = state.find(pk);
        if (it != state.end()) {
            if (pk.is_better(*it)) {
                state.erase(it);
                state.insert(pk);
            }
        } else {
            state.insert(pk);
        }
    }

    void parse_node(const PathDecompNode &node) {
        int edge_id;
        switch(node.type) {
            case INTRODUCE_NODE:
                graph.find_node(node.name);
                bag.step(node);
                parse_introduce_node(node.name);
                break;
            case INTRODUCE_EDGE:
                edge_id = graph.add_edge(bag[node.u], bag[node.v], node.a, node.b);
                parse_introduce_edge(node.u, node.v, edge_id);
                bag.step(node);
                break;
            case FORGET_NODE:
                parse_forget_node(node.name);
                bag.step(node);
                break;
        }
    }

    void parse_introduce_node(const string &name) {
        vector<PartialK6> old = dump_state();
        for (const auto &old_pk : old) {
            PartialK6 pk = old_pk;
            //// New vertex nowhere
            pk.bag2branch.push_back(-1);
            insert_pk(pk);
            pk.bag2branch.pop_back();
            //// New vertex in one of the branches
            int v = (int)pk.bag2branch.size();
            int offset = pk.total_score - (int)pk.paths.size();
            for (unsigned a = 0; a < 6u; ++a) {
                if (!pk.forgotten[a]) {
                    pk.bag2branch.push_back((int)a);
                    pk.branch_sets[a].push_back(graph.find_node(name));
                    vector<vector<BranchPath> > tmp1, tmp2;
                    for (unsigned b = 0; b < 5u; ++b) {
                        if (b != a) {
                            for (unsigned c = b + 1u; c < 6u; ++c) {
                                if (c != a) {
                                    if (!pk.is_path_finished(a, b, c)) {
                                        for (vector<BranchPath> &vbp : tmp1) {
                                            tmp2.push_back(vbp);
                                            vector<BranchPath> vbp2 = vbp;
                                            vbp2.push_back(BranchPath(a, b, c, v, v, false));
                                            tmp2.push_back(vbp2);
                                        }
                                        swap(tmp1, tmp2);
                                        tmp2.clear();
                                    }
                                }
                            }
                        }
                    }
                    for (vector<BranchPath> &vbp : tmp1) {
                        pk.paths = vbp;
                        sort(pk.paths.begin(), pk.paths.end());
                        pk.total_score = (int)pk.paths.size() + offset;
                        insert_pk(pk);
                    }
                    pk.bag2branch.pop_back();
                    pk.branch_sets[a].pop_back();
                }
            }
        }
    }

    void parse_forget_node(const string &name) {
        int bag_id = bag.find(name);
        vector<PartialK6> old = dump_state();
        for (const auto &old_pk : old) {
            PartialK6 pk = old_pk;
            int bid = pk.bag2branch[bag_id];
            pk.bag2branch.erase(pk.bag2branch.begin() + bag_id);
            // If forget a not used vertex, just pass it on
            if (bid == -1) {
                insert_pk(pk);
                continue;
            }
            // The required checks are:
            // (1) the forgotten vertex is not an endpoint of any path in paths
            // (2) if the forgotten vertex is an endpoint of a connection edge, make sure there is a path starting
            //     in it in paths
            // (3) if we forget a last vertex of some branch, check if all paths there are constructed, and
            //     mark the branch forgotten
            // (4) if we forget a last vertex of some branch, check if every triangle has a solid edge,
            // (5) if all branches are forgotten, put it as a found minor
            bool ok = true;

            // Check (1)
            for (BranchPath &bp : pk.paths) {
                if (bp.v1 == bag_id || bp.v2 == bag_id) {
                    ok = false;
                    break;
                }
            }
            if (!ok) {
                continue;
            }

            // Check (2)
            for (int u = 0; u < 6; ++u) {
                if (u != bid && pk.bridge_endpoints[u] == bid) {
                    unsigned mask = ((1u << (unsigned)bid) | (1u << (unsigned)u));
                    for (BranchPath &bp : pk.paths) {
                        if (bp.get_branch() == bid && (bp.get_u1() == u || bp.get_u2() == u)) {
                            mask |= (1u << (bp.get_u1() == u ? bp.get_u2() : bp.get_u1()));
                        }
                    }
                    if (mask < ((1u << 6u) - 1u)) {
                        ok = false;
                        break;
                    }
                }
            }
            if (!ok) {
                continue;
            }

            // Check (3)
            bool is_branch_forgotten = true;
            for (auto a : pk.bag2branch) {
                if (a == bid) {
                    is_branch_forgotten = false;
                    break;
                }
            }
            if (is_branch_forgotten) {
                pk.forgotten[bid] = true;
                int cnt = 0;
                for (BranchPath &bp : pk.paths) {
                    if (bp.get_branch() == bid) {
                        cnt++;
                    }
                }
                assert(cnt <= 10); // There are 5*4/2 = 10 pairs to connect through this branch
                if (cnt < 10) {
                    continue;
                }
                // Check (4)
                if (!pk.check_triangles((unsigned)bid)) {
                    continue;
                }
                // Check (5)
                bool all_branches_forgotten = true;
                for (int i = 0; i < 6; ++i) {
                    if (!pk.forgotten[i]) {
                        all_branches_forgotten = false;
                        break;
                    }
                }
                if (all_branches_forgotten) {
                    Minor m(graph);
                    for (unsigned a = 0; a < 5u; ++a) {
                        for (unsigned b = a + 1u; b < 6u; ++b) {
                            m.edges.push_back(MinorEdge(pk.getBridgeEdge(a, b).id, a, b, pk.getBridgeEdge(a, b).solid));
                        }
                    }
                    m.branches = pk.branch_sets;
                    m.arcs_edges = (int)pk.solid_edges.size();
                    // TODO: better statistics
                    found.push_back(m);
                    continue;
                }
            }
            // Reached here means ready to put into next queue
            // But first need to update bridge_endpoints
            for (int u = 0; u < 6; ++u) {
                if (pk.bridge_endpoints[u] == bid){
                    pk.bridge_endpoints[u] = -1;
                } else if (pk.bridge_endpoints[u] > bid) {
                    pk.bridge_endpoints[u]--;
                }
            }
            insert_pk(pk);
        }
    }
    void parse_introduce_edge(int u, int v, int edge_id) {
        vector<PartialK6> old = dump_state();
        for (const auto &old_pk : old) {
            int br_u = old_pk.bag2branch[u], br_v = old_pk.bag2branch[v];
            // Case A: ignore the new edge if it has one endpoint in no branch or connects two distinct branches
            if (br_u == -1 || br_v == -1 || br_u != br_v) {
                insert_pk(old_pk);
            }
            // Nothing more to do if one endpoint is in no branch
            if (br_u == -1 || br_v == -1) {
                continue;
            }
            // Case B: if the new edge connects two distinct branches, take it as a bridge
            if (br_u != br_v && old_pk.getBridgeEdge(br_u, br_v).id < 0) {
                PartialK6 pk = old_pk;
                pk.setBridgeEdge(br_u, br_v, edge_id, is_solid(edge_id));
                bool ok = pk.add_bridge_endpoint(br_u, br_v, u);
                ok = (ok || pk.add_bridge_endpoint(br_v, br_u, v));
                if (ok) {
                    sort(pk.paths.begin(), pk.paths.end());
                    insert_pk(pk);
                }
            }
            // Case C: if the new edge is internal to some branch, try to use it in paths
            if (br_u == br_v) {
                vector<PartialK6> tmp1, tmp2;
                tmp1.push_back(old_pk);
                for (unsigned b = 0; b < 5u; ++b) {
                    if (b != br_u) {
                        for (unsigned c = b + 1u; c < 6u; ++c) {
                            if (c != br_u) {
                                for(PartialK6 &pk : tmp1) {
                                    tmp2.push_back(pk);
                                    if (pk.try_merge_paths((unsigned)br_u, b, c, u, v, edge_id, is_solid(edge_id))) {
                                        tmp2.push_back(pk);
                                    }
                                }
                                swap(tmp1, tmp2);
                                tmp2.clear();
                            }
                        }
                    }
                }
                for (PartialK6 &pk : tmp1) {
                    insert_pk(pk);
                }
            }

        }
    }

    bool is_solid(int edge_id) {
        return graph.edge_list[edge_id].q == 1;
    }

public:
    Graph graph;
    vector<Minor> found;
    const PathDecomp &pd;

    explicit K6Finder(PathDecomp &_pd) : pd(_pd) {}

    void line_report(int node_id) {
        cout << "Parsing node " << setw(6) << (node_id+1) << "/" << pd.decomposition.size();
        cout << " (type: " << pd.decomposition[node_id].type << ")";
        cout << " number of partial minors: " << setw(10) << state.size();
        cout << " minors found so far: " << setw(10) << found.size();
        cout << flush;
    }

    void find() {
        bag.clear();
        state.clear();

        // Initial empty state
        PartialK6 pk;
        insert_pk(pk);

        line_report(0);
        int cnt = 0;
        for (const auto &node : pd.decomposition) {
            cout << "\33[2K\r";
            line_report(cnt++);
            parse_node(node);
        }
        cout << "\33[2K\r";
        line_report(cnt-1);
        cout << "  FINISHED" << endl;
    }

    string print_minor(Minor &m) const {
        stringstream cf;
        cf << "MINOR (jump_edges=" << m.arcs_edges << "+" << m.arcs_in_branches << ", max_branch_set=" << m.max_branch_set() << ", sum_branch_sets=" << m.sum_branch_sets() << "): ";
        for (auto x : m.edges) {
            cf << " edge(" << x.br_u << " " << x.br_v << ")=" << x.edge_id << "=(" << graph.namesdict.id2name(graph.edge_list[x.edge_id].u) << " " << graph.namesdict.id2name(graph.edge_list[x.edge_id].v) << ")";
        }
        cf << endl;
        return cf.str();
    }
};
#endif //CHROMATYNA_K6FINDER_H
