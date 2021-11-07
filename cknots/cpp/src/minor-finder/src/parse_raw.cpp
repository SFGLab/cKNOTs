/*
Original code by Marcin Pilipczuk
*/

#include <cstdio>
#include <iostream>
#include <set>
#include <boost/program_options.hpp>
using namespace std;

namespace po = boost::program_options;

string chromosome = "chr1";
bool flag_output_csv = false;

void parse_commandline_options(int argc, const char * const *argv){
    po::options_description desc("Supported options");
    desc.add_options()
            ("input-file,f", po::value<string>(), "file to read")
            ("help,h", "produce help message")
            ("chromosome,c", po::value<string>(), "chromosome label, e.g., chr1")
            ("output-csv,o", "output csv instead of parsed format")
            ;
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")){
        cout << desc << endl;
        exit(0);
    }
    if (vm.count("output-csv")) {
        flag_output_csv = true;
    }
    if (vm.count("input-file")) {
        if (!freopen(vm["input-file"].as<string>().c_str(), "r", stdin)){
            cout << "Error opening file: " << vm["input-file"].as<string>() << endl;
            exit(0);
        }
    }
    if (vm.count("chromosome"))
        chromosome = vm["chromosome"].as<string>();
}

struct interaction {
    int i1, i2, j1, j2, cnt;

    interaction(int _i1, int _i2, int _j1, int _j2, int _cnt) : i1(_i1), i2(_i2), j1(_j1), j2(_j2), cnt(_cnt) {};
};

struct edge {
    int u, v, a, b;

    edge(int _u, int _v, int _a, int _b) : u(_u), v(_v), a(_a), b(_b) {};
};


vector<interaction> interactions;
vector<edge> edges;
vector<string> names;

void read_input(void) {
    interactions.clear();
    string c1, c2;
    int i1, i2, j1, j2, cnt;
    while(cin >> c1 >> i1 >> i2 >> c2 >> j1 >> j2 >> cnt)
        if (c1 == chromosome && c2 == chromosome)
            interactions.push_back(interaction(i1, i2, j1, j2, cnt));
}

void interactions2edges(void) {
    map<int, int> relabel_map;
    int cnt_sum = 0;
    vector<int> endpoints;
    for(auto x : interactions) {
        endpoints.push_back(x.i1);
        endpoints.push_back(x.i2);
        endpoints.push_back(x.j1);
        endpoints.push_back(x.j2);
        cnt_sum += x.cnt;
    }
    sort(endpoints.begin(), endpoints.end());
    endpoints.erase(unique(endpoints.begin(), endpoints.end()), endpoints.end());
    for (int i = 0; i < endpoints.size(); ++i) {
        relabel_map[endpoints[i]] = i + 1;
        char *t = new char[chromosome.size() + 15];
        sprintf(t, "%s_%010d", chromosome.c_str(), endpoints[i]);
        names.push_back(string(t));
        delete[] t;
    }
    for (auto x : interactions) {
        // Check consistency: each segment should be unsplit by other segments.
        assert(relabel_map[x.i2] == relabel_map[x.i1] + 1);
        assert(relabel_map[x.j2] == relabel_map[x.j1] + 1);
        assert(relabel_map[x.i1] % 2 == 1);
        assert(relabel_map[x.j1] % 2 == 1);
        edges.push_back(edge(relabel_map[x.i1], relabel_map[x.j1], x.cnt, cnt_sum));
    }
}


void output_parsed() {
    for (auto x : names) {
        cout << "NODE " << x << endl;
    }
    for(auto e : edges) {
        cout << "EDGE " << names[min(e.u, e.v)] << " " << names[max(e.u, e.v)] << " " << e.a << " " << e.b << endl;
    }
}

void output_csv() {
    string last_name = "";
    for (auto x : names) {
        if (last_name.size() > 0) {
            cout << last_name << " " << x << " " << 1.0 << endl;
        }
        last_name = x;
    }
    for (auto e : edges) {
        cout << names[min(e.u, e.v)] << " " << names[max(e.u, e.v)] << " " << ((double)e.a/(double)e.b) << endl;
    }
}



int main(int argc, const char * const *argv){
    parse_commandline_options(argc, argv);

    read_input();
    interactions2edges();
    if (flag_output_csv) {
        output_csv();
    } else {
        output_parsed();
    }

    return 0;
}
