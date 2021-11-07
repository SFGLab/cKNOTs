/*
Original code by Marcin Pilipczuk
*/

#include <cstdio>
#include <iostream>
#include <set>
#include <boost/program_options.hpp>
#include <fstream>
#include "minorfinder.h"
using namespace std;

namespace po = boost::program_options;

int max_n = 7, max_cost = 30, min_n = 1, max_nonedges = 1000, min_deg = 3;
string output_filename = "";

void parse_commandline_options(int argc, const char * const *argv){
    po::options_description desc("Supported options");
    desc.add_options()
            ("input-file,f", po::value<string>(), "file to read")
            ("output-file,o", po::value<string>(), "file to output minors")
            ("max-n,N", po::value<int>(), "max number of nodes of a minor")
            ("min-n,n", po::value<int>(), "min number of nodes of a minor")
            ("max-cost,c", po::value<int>(), "max number of arcs in a minor model")
            ("max-nonedges,e", po::value<int>(), "max number of nonedges of a minor")
            ("min-deg,d", po::value<int>(), "min degree of a minor")
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
    if (vm.count("max-cost")) {
        max_cost = vm["max-cost"].as<int>();
    }
    if (vm.count("max-n")) {
        max_n = vm["max-n"].as<int>();
    }
    if (vm.count("min-n")) {
        min_n = vm["min-n"].as<int>();
    }
    if (vm.count("min-deg")) {
        min_deg = vm["min-deg"].as<int>();
    }
    if (vm.count("max-nonedges")) {
        max_nonedges = vm["max-nonedges"].as<int>();
    }
    if (vm.count("output-file")) {
        output_filename = vm["output-file"].as<string>();
    }
}

int main(int argc, const char * const *argv){
    parse_commandline_options(argc, argv);

    PathDecomp pd;
    pd.read();

    cout << "Read decomposition, width=" << pd.width() << endl;

    MinorFinder mf(pd, max_cost, max_n, min_n, max_nonedges, min_deg);
    mf.find();
    cout << "Found " << mf.found.size() << endl;

    if (output_filename != "") {
        fstream cf;
        cf.open(output_filename, fstream::out);
        for (auto &m : mf.found) {
            cf << mf.print_minor(m);
        }
        cf.close();
    }
    return 0;
}
