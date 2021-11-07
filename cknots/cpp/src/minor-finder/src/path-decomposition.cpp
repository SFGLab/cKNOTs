/*
Original code by Marcin Pilipczuk
*/

#include <cstdio>
#include <iostream>
#include <set>
#include <boost/program_options.hpp>
#include <fstream>
#include "namesdict.h"
#include "pathdecomp.h"
using namespace std;

namespace po = boost::program_options;

string test_name = "input";

void parse_commandline_options(int argc, const char * const *argv){
    po::options_description desc("Supported options");
    desc.add_options()
            ("input-file,f", po::value<string>(), "file to read")
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
        test_name = vm["input-file"].as<string>();
    }
}

struct edge {
    int u, v, a, b;

    edge(int _u, int _v, int _a, int _b) : u(_u), v(_v), a(_a), b(_b) {};
};

vector<edge> edges;
NamesDict namesdict;

void read_input() {
    edges.clear();
    namesdict.clear();
    string tmp;
    while (cin >> tmp) {
        if (tmp == "NODE") {
            cin >> tmp;
            namesdict.name2id(tmp);
        } else if (tmp == "EDGE") {
            string nu, nv;
            int u, v, a, b;
            cin >> nu >> nv >> a >> b;
            u = namesdict.name2id(nu);
            v = namesdict.name2id(nv);
            if (u > v) {
                swap(u, v);
            }
            edges.push_back(edge(u, v, a, b));
        }
    }
}

PathDecomp compute_path_decomposition() {
    int n = (int)namesdict.names.size();
    vector<int> cnt(n, 0);
    vector<int> max_right(n, 0);
    vector<int> state(n, 0);
    vector<vector<edge> > inc_list(n);

    for (auto e : edges) {
        inc_list[e.v].push_back(e);
        inc_list[e.u].push_back(e);
    }
    // Jump over deg-2 vertices
    int last = 0;
    for (int i = 1; i < n; ++i) {
        if (i == n-1 || !inc_list[i].empty()) {
            inc_list[last].push_back(edge(last, i, 1, 1));
            inc_list[i].push_back(edge(last, i, 1, 1));
            max_right[last] = i;
            last = i;
        }
    }
    max_right[n-1] = n-1;
    for (auto e : edges) {
        max_right[e.u] = max(max_right[e.u], e.v);
    }
    set<pair<int, int> > to_forget;
    PathDecomp pd;
    PathDecompBag bag;

    for (int i = 0; i < n; ++i) {
        if (i == 0 || i == n-1 || !inc_list[i].empty()) {
            pd.decomposition.push_back(PathDecompNode(INTRODUCE_NODE, namesdict.names[i]));
            bag.step(pd.decomposition.back());
            state[i] = 1;
            to_forget.insert({max_right[i], i});
            for (auto it = to_forget.begin(); it != to_forget.end(); ++it) {
                if (it->first == i) {
                    for (auto e : inc_list[it->second]) {
                        if (state[e.u] == 1 && state[e.v] == 1) {
                            pd.decomposition.push_back(
                                    PathDecompNode(bag.find(namesdict.id2name(e.u)), bag.find(namesdict.id2name(e.v)),
                                                   e.a,
                                                   e.b));
                            bag.step(pd.decomposition.back());
                        }
                    }
                    pd.decomposition.push_back(PathDecompNode(FORGET_NODE, namesdict.id2name(it->second)));
                    bag.step(pd.decomposition.back());
                    state[it->second] = 2;
                } else {
                    to_forget.erase(to_forget.begin(), it);
                    break;
                }
            }
        }
    }
    return pd;
}

int main(int argc, const char * const *argv){
    parse_commandline_options(argc, argv);

    read_input();
    PathDecomp pd = compute_path_decomposition();
    pd.print_decomposition();
    cerr << "Computed path decomposition for " << test_name << ", nodes " << pd.decomposition.size() << " max bag size " << pd.width() << endl;

    return 0;
}
