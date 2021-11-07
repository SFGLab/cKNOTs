/*
Original code by Marcin Pilipczuk
*/

#include <cstdio>
#include <iostream>
#include <set>
#include <boost/program_options.hpp>
#include <fstream>
#include "k6finder.h"
using namespace std;

namespace po = boost::program_options;

int minor_size = 6;
string output_filename = "";
bool allow_common_endpoints = true;

void parse_commandline_options(int argc, const char * const *argv){
    po::options_description desc("Supported options");
    desc.add_options()
            ("input-file,f", po::value<string>(), "file to read")
            ("output-file,o", po::value<string>(), "file to output minors")
            ("num-vertices,n", po::value<int>(), "number of vertices of the clique minor to find, default 6")
            ("no-common-endpoints,c", "disallow common endpoints of jump edges")
            ("help,h", "produce help message")
            ;
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")){
        cout << desc << endl;
        exit(0);
    }
    if (vm.count("input-file")) {
        if (!freopen(vm["input-file"].as<string>().c_str(), "r", stdin)){
            cout << "Error opening file: " << vm["input-file"].as<string>() << endl;
            exit(0);
        }
    }
    if (vm.count("output-file")) {
        output_filename = vm["output-file"].as<string>();
    }
    if (vm.count("no-common-endpoints")) {
        allow_common_endpoints = false;
    }
    if (vm.count("num-vertices")) {
        minor_size = vm["num-vertices"].as<int>();
    }
}

struct LinearMinor {
    vector<int> last_vertices; // Implicitly says which is the current segment
    vector<int> chosen_edges;
    unsigned current_edges_mask;
    unsigned current_vertex_mask;
    Graph &g;

    explicit LinearMinor(Graph &_g) : current_edges_mask(0u), current_vertex_mask(0u), g(_g) {};

    // Maybe consider exact cost comparison, but I'm not sure it's worth it.
    // Makes sense only for comparison with the same number of chosen edges.
    double cost() const {
        double res = 0.0;
        for (auto eid : chosen_edges) {
            res += log((double)g.edge_list[eid].p);
        }
        return res;
    }

    int vertex2segment(int v) const {
        int i = 0;
        while (i < (int)last_vertices.size() && last_vertices[i] < v) {
            i++;
        }
        return i;
    }

    bool operator<(const LinearMinor &lm) const {
        if (last_vertices.size() != lm.last_vertices.size()) {
            return last_vertices.size() < lm.last_vertices.size();
        }
        if (current_edges_mask != lm.current_edges_mask) {
            return current_edges_mask < lm.current_edges_mask;
        }
        return current_vertex_mask < lm.current_vertex_mask;
    }

    bool has_max_edges() const {
        int b = (int)last_vertices.size();
        return (chosen_edges.size() >= (minor_size - 1) * (minor_size - 2) / 2 + max(minor_size - 2 - b, 0) - (minor_size - 1 - b) * (minor_size - 2 - b) / 2);
    }

    bool can_take_new_edge(int eid) const {
        // first check if not too many edges
        if (has_max_edges()) {
            return false;
        }
        // check if not bad triangle
        Edge &e = g.edge_list[eid];
        for (unsigned a = 0; a < chosen_edges.size(); ++a) {
            Edge &ea = g.edge_list[chosen_edges[a]];
            if (!allow_common_endpoints) {
                if (ea.first() == e.first() || ea.second() == e.first() || ea.first() == e.second() || ea.second() == e.second()) {
                    return false;
                }
            }
            for (unsigned b = 0; b < a; ++b) {
                Edge &eb = g.edge_list[chosen_edges[b]];
                if (ea.first() == eb.first() && ((ea.second() == e.first() && eb.second() == e.second()) || (eb.second() == e.first() && ea.second() == e.second()))) {
                    return false;
                }
            }
        }
        return true;
    }

    void debug_check_correctness() const {
        assert(last_vertices.size() == (unsigned)minor_size);
        // Check all edges
        if (chosen_edges.size() != (unsigned)((minor_size - 1) * (minor_size - 2) / 2)) {
            assert(!"Wrong number of chosen edges!");
        }
        set<pair<int, int> > connections;
        for (auto eid : chosen_edges) {
            int a = vertex2segment(g.edge_list[eid].first());
            int b = vertex2segment(g.edge_list[eid].second());
            if (a + 1 >= b) {
                assert(!"Two consecutive segments connected by an edge!");
            }
            if (connections.count({a, b})) {
                assert(!"Two segments already connected!");
            }
            connections.insert({a, b});
        }
        if (connections.size() != (unsigned)((minor_size - 1) * (minor_size - 2) / 2)) {
            assert(!"Wrong number of connections");
        }
        // Check triangles
        for (auto e1 : chosen_edges) {
            for (auto e2: chosen_edges) {
                if (!allow_common_endpoints && e1 != e2) {
                    if (g.edge_list[e1].first() == g.edge_list[e2].first() ||
                        g.edge_list[e1].first() == g.edge_list[e2].second() ||
                        g.edge_list[e1].second() == g.edge_list[e2].first() ||
                        g.edge_list[e1].second() == g.edge_list[e2].second()) {
                        assert(!"Common endpoint of two jump edges");
                    }
                }
                for (auto e3 : chosen_edges) {
                    if (e1 != e2 && e2 != e3 && e1 != e3 && g.edge_list[e1].first() == g.edge_list[e2].first() &&
                        g.edge_list[e1].second() == g.edge_list[e3].first() && g.edge_list[e2].second() == g.edge_list[e3].second()) {
                        assert(!"Wrong triangle!");
                    }
                }
            }
        }
    }
};

class LinearMinorFinder {
private:
    Graph &g;

    vector<LinearMinor> found;
    set<LinearMinor> partial_minors;
    vector<int> current_edges;
    map<int, int> current_edges_map;

    void process_single_vertex(int v) {
        set<int> new_current_edges_set(current_edges.begin(), current_edges.end());
        for (auto eid : g.inc_list[v]) {
            int u = (g.edge_list[eid].u == v ? g.edge_list[eid].v : g.edge_list[eid].u);
            if (u < v) {
                new_current_edges_set.erase(eid);
            } else {
                new_current_edges_set.insert(eid);
            }
        }
        vector<int> new_current_edges;
        std::copy(new_current_edges_set.begin(), new_current_edges_set.end(), std::back_inserter(new_current_edges));
        map<int, int> new_current_edges_map;
        for (int i = 0; i < (int)new_current_edges.size(); ++i) {
            new_current_edges_map[new_current_edges[i]] = i;
        }

        set<LinearMinor> new_partial_minors;
        for (auto &opm : partial_minors) {
            vector<LinearMinor> tmp_list, tmp_list2;
            tmp_list.push_back(opm);
            if (v > 0) {
                // Check if all previous edges present
                unsigned b = (int)opm.last_vertices.size();
                unsigned expected_cvm = (1u << (max(b, 1u) - 1)) - 1;
                if (expected_cvm == opm.current_vertex_mask && opm.has_max_edges()) {
                    LinearMinor pm = opm;
                    pm.last_vertices.push_back(v - 1);
                    pm.current_vertex_mask = 0u;
                    if (pm.last_vertices.size() == minor_size) {
                        found.push_back(pm);
                    } else {
                        tmp_list.push_back(pm);
                    }
                }
            }
            for (auto eid : g.inc_list[v]) {
                for (auto &pm : tmp_list) {
                    int u = (g.edge_list[eid].u == v ? g.edge_list[eid].v : g.edge_list[eid].u);
                    if (u < v) {
                        // old edge
                        assert(current_edges_map.count(eid) > 0);
                        if (pm.current_edges_mask & (1u << current_edges_map[eid])) {
                            int i = pm.vertex2segment(g.edge_list[eid].first());
                            if (i + 1 < (int)pm.last_vertices.size() && !(pm.current_vertex_mask & (1u << i))) {
                                pm.current_vertex_mask |= (1u << i);
                                tmp_list2.push_back(pm);
                            }
                        } else {
                            tmp_list2.push_back(pm);
                        }
                    } else {
                        // new edge
                        // not take it
                        tmp_list2.push_back(pm);
                        // take it
                        if (pm.can_take_new_edge(eid)) {
                            pm.chosen_edges.push_back(eid);
                            tmp_list2.push_back(pm);
                        }
                    }
                }
                swap(tmp_list, tmp_list2);
                tmp_list2.clear();
            }
            for (auto &pm : tmp_list) {
                pm.current_edges_mask = 0;
                for (auto eid : pm.chosen_edges) {
                    if (new_current_edges_map.count(eid)) {
                        pm.current_edges_mask |= 1u << new_current_edges_map[eid];
                    }
                }
                new_partial_minors.insert(pm);
            }
        }
        swap(partial_minors, new_partial_minors);
        swap(current_edges, new_current_edges);
        swap(current_edges_map, new_current_edges_map);
        cout << "\33[2K\r";
        cout << "Processed vertex " << setw(6) << v  << "/" << (g.get_n()-1);
        cout << " number of partial minors: " << setw(10) << partial_minors.size();
        cout << " minors found so far: " << setw(10) << found.size();
        cout << flush;
    }
public:
    // Assumes graph read by read_parsed_txt()
    explicit LinearMinorFinder(Graph &_g) : g(_g) {};

    void solve() {
        g.find_node("dummy");
        current_edges.clear();
        found.clear();
        partial_minors.clear();
        // Initial empty minor
        partial_minors.insert(LinearMinor(g));
        for (int v = 0; v < g.get_n(); ++v) {
            process_single_vertex(v);
        }
        cout << endl;
    }

    void output_minors(const string &output_filename) {
        if (!output_filename.empty()) {
            fstream cf;
            cf.open(output_filename, fstream::out);
            for (auto &m : found) {
                m.debug_check_correctness();
                cf << "MINOR { " << endl;
                cf << "  endpoints=[" << endl;
                for (int i = 0; i < minor_size; ++i) {
                    int vs = (i ? m.last_vertices[i-1] + 1 : 0);
                    int ve = m.last_vertices[i];
                    cf << "    segment=" << i << " start=(" << vs << "=" << g.namesdict.id2name(vs) << ") ";
                    cf << "end=(" << ve << "=" << g.namesdict.id2name(ve) << ") ";
                    cf << endl;
                }
                cf <<"  ]" << endl;
                cf <<"  edges=[" << endl;
                for (auto eid : m.chosen_edges) {
                    cf << "  from " << m.vertex2segment(g.edge_list[eid].first()) << " to " << m.vertex2segment(g.edge_list[eid].second()) << ", ";
                    cf << "eid=" << eid << ", ";
                    cf << "left=(" << g.edge_list[eid].first() << "=" << g.namesdict.id2name(g.edge_list[eid].first()) << "), ";
                    cf << "right=(" << g.edge_list[eid].second() << "=" << g.namesdict.id2name(g.edge_list[eid].second()) << ")" << endl;
                }
                cf <<"  ]" << endl << "}" << endl;
            }
            cf.close();
        }
    }
};

int main(int argc, const char * const *argv){
    parse_commandline_options(argc, argv);

    Graph g;
    g.read_parsed_txt();

    cout << "Read graph n=" << g.get_n() << " m=" << g.get_m() << endl;

    LinearMinorFinder finder(g);

    finder.solve();

    finder.output_minors(output_filename);
    return 0;
}
