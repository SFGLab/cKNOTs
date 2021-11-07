/*
Original code by Marcin Pilipczuk
*/

#include <iomanip>
#include <sstream>
#include "minorfinder.h"
#include "tools.h"

void MinorFinder::PartialMinor::remap_to_bag() {
    vector<int> perm(n, -1);
    int cnt = 0;
    for (auto bid : bag2branch) {
        if (bid >= 0 && perm[bid] == -1) {
            perm[bid] = cnt++;
        }
    }
    for (int i = 0; i < n; ++i) {
        if (perm[i] == -1) {
            perm[i] = cnt++;
        }
    }
    remap(perm);
}

void MinorFinder::PartialMinor::remap(vector<int> const &perm) {
    for (auto &bid : bag2branch) {
        if (bid >= 0) {
            bid = perm[bid];
        }
    }
    for (auto &e : graph) {
        e.br_u = perm[e.br_u];
        e.br_v = perm[e.br_v];
        if (e.br_u > e.br_v) {
            swap(e.br_u, e.br_v);
        }
    }
    sort(graph.begin(), graph.end());
#ifdef USE_BRANCHES
    assert(n == (int)branches.size());
    vector<vector<int> > branches_copy(n);
    for (int i = 0; i < n; ++i) {
        swap(branches[i], branches_copy[i]);
    }
    for (int i = 0; i < n; ++i) {
        swap(branches[perm[i]], branches_copy[i]);
    }
#endif
#ifdef USE_HOPS
    assert(n == (int)hops.size());
    vector<vector<int> > hops_copy(n);
    for (int i = 0; i < n; ++i) {
        swap(hops[i], hops_copy[i]);
    }
    for (int i = 0; i < n; ++i) {
        swap(hops[perm[i]], hops_copy[i]);
    }
#endif
}

bool MinorFinder::PartialMinor::is_connected_to_bag() const {
    FAU fau(n);
    for (auto &me : graph) {
        fau.join(me.br_u, me.br_v);
    }
    vector<bool> bag_intersection(n, false);
    for (auto x : bag2branch) {
        if (x >= 0) {
            bag_intersection[fau.find(x)] = true;
        }
    }
    for (int i = 0; i < n; ++i) {
        if (!bag_intersection[fau.find(i)]) {
            return false;
        }
    }
    return true;
}

bool MinorFinder::PartialMinor::contains_bag_vertices() const {
    for (auto bid : bag2branch) {
        if (bid != -1) {
            return true;
        }
    }
    return false;
}

void MinorFinder::canonize(PartialMinor &pm) {
    int n_in_bags = pm.n - pm.get_n();
    vector<int> perm;
    for (int i = 0; i < pm.n; ++i) {
        perm.push_back(i);
    }
    PartialMinor copy = pm;
    do{
        PartialMinor tmp = copy;
        tmp.remap(perm);
        if (tmp < pm) {
            pm = tmp;
        }
    }while(next_permutation(perm.begin() + n_in_bags, perm.end()));
}

void MinorFinder::insert_pm(const PartialMinor &pm) {
    auto it = state.find(pm);
    if (it != state.end()) {
        if (pm.is_better(*it)) {
            state.erase(it);
            state.insert(pm);
        }
    } else {
        state.insert(pm);
    }
}

vector<MinorFinder::PartialMinor> MinorFinder::dump_state() {
    vector<PartialMinor> old;
    old.insert(old.end(), make_move_iterator(state.begin()), make_move_iterator(state.end()));
    state.clear();
    return old;
}

void MinorFinder::parse_introduce_node(const string &name) {
    vector<PartialMinor> old = dump_state();
    for (const auto &old_pm : old) {
        PartialMinor pm = old_pm;
        //// New vertex nowhere
        pm.bag2branch.push_back(-1);
        insert_pm(pm);
        pm.bag2branch.pop_back();
        //// New vertex in new branch
        // Optimization: if previously was canonical, now only shift required, no call to canonize() needed
        pm.bag2branch.push_back(pm.n);
        pm.n++;
#ifdef USE_BRANCHES
        pm.branches.push_back(vector<int>());
        pm.branches.back().push_back(graph.find_node(name));
#endif
#ifdef USE_HOPS
        pm.hops.push_back(vector<int>());
#endif
        pm.remap_to_bag();
        insert_pm(pm);
    }
}

void MinorFinder::parse_forget_node(const string &name) {
    int bag_id = bag.find(name);
    vector<PartialMinor> old = dump_state();
    for (const auto &old_pm : old) {
        PartialMinor pm = old_pm;
        int bid = pm.bag2branch[bag_id];
        pm.bag2branch.erase(pm.bag2branch.begin() + bag_id);
        // Check if branch still present in bag
        bool bid_present_in_bag = false;
        for (auto i : pm.bag2branch) {
            if (i == bid) {
                bid_present_in_bag = true;
                break;
            }
        }
        // Compute degree of forgotten branch
        int bid_degree = 0;
        if (!bid_present_in_bag && min_deg > 0) {
            for (auto &me : pm.graph) {
                if (me.br_u == bid || me.br_v == bid) {
                    bid_degree++;
                }
            }
        }
        // Optimization: if previously was canonical, now only shift required, no call to canonize() needed
        pm.remap_to_bag();
        int n = pm.get_n();
        // Compute edges with both endpoints already forgotten
        int forgotten_edges = 0;
        int half_forgotten_edges = 0;
        if (!bid_present_in_bag) {
            for (auto &me : pm.graph) {
                if (me.br_u >= pm.n - n && me.br_v >= pm.n - n) {
                    forgotten_edges++;
                } else if (me.br_u >= pm.n - n || me.br_v >= pm.n - n) {
                    half_forgotten_edges++;
                }
            }
        }
        int sure_nonedges = (n*(n-1)/2 - forgotten_edges) + max(0, (n * max(0, min_n - n) - half_forgotten_edges));
        if (n <= max_n && (bid_present_in_bag || (bid_degree >= min_deg && sure_nonedges <= max_nonedges))) {
            if (pm.is_connected_to_bag()) {
                if (n < max_n) {
                    insert_pm(pm);
                }
            } else if (!pm.contains_bag_vertices() && n >= min_n) {
                assert(!bid_present_in_bag);
                Minor m(graph);
                m.edges = pm.graph;
                m.branches = pm.branches;
                m.arcs_edges = pm.arcs_edges;
                m.arcs_in_branches = pm.arcs_in_branches;
                found.push_back(m);
            }
        }
    }
}

void MinorFinder::parse_introduce_edge(int u, int v, int edge_id) {
    vector<PartialMinor> old = dump_state();
    for (const auto &old_pm : old) {
        //// Ignore the edge
        insert_pm(old_pm);
        // Remaining cases valid only for edge connecting two distinct branches
        if (old_pm.bag2branch[u] == -1 || old_pm.bag2branch[v] == -1 || old_pm.bag2branch[u] == old_pm.bag2branch[v]) {
            continue;
        }
        int br_u = old_pm.bag2branch[u], br_v = old_pm.bag2branch[v];
        if (br_u > br_v) {
            swap(br_u, br_v);
        }
        if (!old_pm.has_edge(br_u, br_v)) {
            //// Connect branches via edge
            PartialMinor pm = old_pm;
            pm.add_edge(br_u, br_v, edge_id);
            if (graph.edge_list[edge_id].q != 1) {
                pm.arcs_edges++;
            }
            if (pm.get_cost() <= max_cost) {
                insert_pm(pm);
            }
            //// Merge branches via edge
            pm = old_pm;
            for (auto &x : pm.bag2branch) {
                if (x == br_v) {
                    x = br_u;
                } else if (x > br_v) {
                    x--;
                }
            }
            for (auto &e : pm.graph) {
                if (e.br_u == br_v) {
                    e.br_u = br_u;
                } else if (e.br_u > br_v) {
                    e.br_u--;
                }
                if (e.br_v == br_v) {
                    e.br_v = br_u;
                } else if (e.br_v > br_v) {
                    e.br_v--;
                }
                if (e.br_u > e.br_v) {
                    swap(e.br_u, e.br_v);
                }
            }
            sort(pm.graph.begin(), pm.graph.end());
            bool repeated_edge = false;
            for (unsigned i = 1; i < pm.graph.size(); ++i) {
                if (pm.graph[i].br_u == pm.graph[i-1].br_u && pm.graph[i].br_v == pm.graph[i-1].br_v) {
                    repeated_edge = true;
                    break;
                }
            }
            if (!repeated_edge) {
#ifdef USE_BRANCHES
                pm.branches[br_u].insert(pm.branches[br_u].end(), make_move_iterator(pm.branches[br_v].begin()),
                                         make_move_iterator(pm.branches[br_v].end()));
                for (int i = br_v + 1; i < pm.n; ++i) {
                    swap(pm.branches[i], pm.branches[i - 1]);
                }
                pm.branches.pop_back();
#endif
#ifdef USE_HOPS
                pm.hops[br_u].insert(pm.hops[br_u].end(), make_move_iterator(pm.hops[br_v].begin()),
                                     make_move_iterator(pm.hops[br_v].end()));
                if (p != 1 || q != 1) {
                    pm.hops[br_u].push_back(edge_id);
                }
                sort(pm.hops[br_u].begin(), pm.hops[br_u].end());
                for (int i = br_v + 1; i < pm.n; ++i) {
                    swap(pm.hops[i], pm.hops[i - 1]);
                }
                pm.hops.pop_back();
#endif
                pm.n--;
                pm.remap_to_bag();
                if (graph.edge_list[edge_id].q != 1) {
                    pm.arcs_in_branches++;
                }
                if (pm.get_cost() <= max_cost) {
                    //canonize(pm);
                    insert_pm(pm);
                }
            }
        }
    }
}

void MinorFinder::parse_node(const PathDecompNode &node) {
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

void MinorFinder::line_report(int node_id) {
    cout << "Parsing node " << setw(6) << (node_id+1) << "/" << pd.decomposition.size();
    cout << " (type: " << pd.decomposition[node_id].type << ")";
    cout << " number of partial minors: " << setw(10) << state.size();
    cout << " minors found so far: " << setw(10) << found.size();
    cout << flush;
}

void MinorFinder::find() {
    bag.clear();
    state.clear();

    // Initial empty state
    PartialMinor pm;
    pm.n = 0;
    pm.arcs_in_branches = pm.arcs_edges = 0;
    insert_pm(pm);

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

string MinorFinder::print_minor(Minor &m) const {
    stringstream cf;
    cf << "MINOR (jump_edges=" << m.arcs_edges << "+" << m.arcs_in_branches << ", max_branch_set=" << m.max_branch_set() << ", sum_branch_sets=" << m.sum_branch_sets() << "): ";
    for (auto x : m.edges) {
        cf << " edge(" << x.br_u << " " << x.br_v << ")=" << x.edge_id << "=(" << graph.namesdict.id2name(graph.edge_list[x.edge_id].u) << " " << graph.namesdict.id2name(graph.edge_list[x.edge_id].v) << ")";
    }
#ifdef USE_BRANCHES
    for (const auto &x : m.branches) {
        cf << " branch(";
        bool space = false;
        for (auto i : x) {
            if (space) {
                cf << " ";
            }
            space = true;
            cf << graph.namesdict.id2name(i);
        }
        cf << ")";
    }
#endif
#ifdef USE_HOPS
    for (const auto &x : m.hops) {
        cf << "hops(";
        for (auto i : x) {
            cf << i << " ";
        }
        cf << ") ";
    }
#endif
    cf << endl;
    return cf.str();
}