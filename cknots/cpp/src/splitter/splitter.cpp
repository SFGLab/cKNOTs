/*
Original code by Maciej Borodzik
Modified by Krzysztof Spalinski
*/

#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <array>

#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>
#include <filesystem>

namespace po = boost::program_options;
namespace fs = std::filesystem;

class segment {
public:
    long long int first;
    long long int second;

    segment() {
        first = 0;
        second = 0;
    }

    segment(long long int a, long long int b) {
        first = std::min(a, b);
        second = std::max(a, b);
    }

    //     bool operator<( segment s) { if (first<s.first) return true; else if (first>s.first) return false; else if (second<s.second) return true; else return false;}
    //   bool operator==( segment s) { return ((first==s.first) and (second=s.second));}
    //
    friend bool operator<(const segment &s1, const segment &s2) {
        if (s1.first < s2.first) return true;
        else if (s1.first > s2.first)return false; else if (s1.second < s2.second) return true; else return false;
    }

    friend bool operator==(const segment &s1, const segment &s2) {
        return ((s1.first == s2.first) and (s1.second == s2.second));
    }
    friend std::ostream& operator<< (std::ostream& os, const segment& seg);
};

class edge_data
{
public:
    segment first;
    segment second;
    std::string extra1;
    std::string extra2;
    long long int pos_in_file;
    edge_data(segment a, segment b, std::string c, std::string d, long long int e)
    {first=a; second=b; extra1=c; extra2=d; pos_in_file=e;}
    edge_data() {first=segment(0,0); second=segment(0,0);}
    edge_data(segment a, segment b, long long int c){first=a; second=b; pos_in_file=c;}
    edge_data(segment a, segment b, std::string c, long long int e){first=a; second=b;
        extra1=c; pos_in_file=e;}

    friend bool operator<(const edge_data &s1, const edge_data &s2);
};

bool operator<(const edge_data &s1, const edge_data &s2)
{
    if (s1.first<s2.first) return true;
    if (s2.first<s1.first) return false;
    if (s1.second<s2.first) return true;
    if (s2.second<s1.first) return false;
    if (s1.pos_in_file<s2.pos_in_file) return true; else return false;
}

typedef std::set<edge_data> all_edges;
typedef std::array<all_edges, 26> edge_array;
typedef std::array<std::set<segment>, 26> segment_array;

int chrom_from_chr(std::string st) {
    // takes a string of form 'chrXX' and translates it into
    // the chromosome number XX

    if (st.find("chrX") != std::string::npos) return 23;
    if (st.find("chrY") != std::string::npos) return 24;
    if (st.find("chrM") != std::string::npos) return 25;
    try {
        long int chrom_index = std::stol(st.substr(3, 2));
        return chrom_index;
    }
    catch (...) {
        return -1;
    }
}

std::string pad_int(int a, int padding = 4) {
    char buffer [5];
    std::sprintf(buffer, "%04d", a);
    std::string str(buffer);
    return str;
}

std::string chr_from_chrom(int chr)
{
    if (chr==23) return std::string("chrX");
    if (chr==24) return std::string("chrY");
    if (chr==25) return std::string("chrM");
    return std::string("chr")+std::to_string(chr);
}

std::ostream& operator<<(std::ostream& os, const segment & seg)
{
    os << seg.first << "\t" << seg.second;
    return os;
}

edge_array read_lines_from_file(std::string filename)
{
    std::ifstream myfile(filename);
    std::string handle_string;
    std::vector<std::string> my_split_string;

    edge_array myarray;
    bool is_bad;
    long long int a, b, c, d;
    int chrom;
    long long int line_number=0;
    while (std::getline(myfile, handle_string))
    {
        my_split_string.clear();
        line_number++;

        boost::split(my_split_string, handle_string, boost::is_any_of(" \t"));

        is_bad = false;
        if (my_split_string.size()<7) is_bad = true;
        else
        {
            try
            {
                chrom = chrom_from_chr(my_split_string[0]);
                a = std::stol(my_split_string[1]);
                b = std::stol(my_split_string[2]);
                c = std::stol(my_split_string[4]);
                d = std::stol(my_split_string[5]);
                if (b>10*a) is_bad = true;
                if (d>10*c) is_bad = true;
                if (a>10*b) is_bad = true;
                if (c>10*d) is_bad = true;
                if ((chrom==-1) or (chrom>25)) is_bad = true;
            }
            catch (...) { is_bad = true; }

            if (my_split_string[0].size()>5) is_bad = true;
            if (my_split_string[3].size()>5) is_bad = true;
            if (my_split_string[0].find("chr")!=0) is_bad = true;
            if (my_split_string[3].find("chr")!=0) is_bad = true;
            // we do not allow edges connecting different chromosomes.
            if (chrom!=chrom_from_chr(my_split_string[3])) is_bad=true;
        }

        if (!is_bad)
        {
            if (my_split_string.size() < 7) my_split_string.push_back("0");
            if (my_split_string.size() < 7) my_split_string.push_back("0");
            myarray.at(chrom).insert(
                    edge_data(segment(a, b), segment(c, d), my_split_string[6], my_split_string[7], line_number));
        }
    }
    return myarray;
}

all_edges glue_segments(all_edges raw_edges)
{
    // parses a segment of edges and glues intervals
    // code copied from secondparse.cpp
    std::set<segment> set_of_segments;
    all_edges return_edge;
    for (auto & redg : raw_edges)
    {
        set_of_segments.insert(redg.first);
        set_of_segments.insert(redg.second);
    }

    segment maxsegment;
    std::set<segment>::iterator upfirst;
    std::set<segment>::iterator upsecond;

    std::set<segment>::iterator it_set;
    std::set<segment> handle_set;
    long long int current_vertex;
    long long int limit_vertex;

    current_vertex=-1;
    limit_vertex=-1;
    maxsegment=*set_of_segments.rbegin(); // the maximal element
    // now handle_set will contain all the segments

    for (auto & mysegment:set_of_segments)
    {
        if (current_vertex==-1)
        {
            current_vertex=mysegment.first;
            limit_vertex=mysegment.second;
        }
        if (mysegment.first>limit_vertex)
        {
            // we start new interval
            handle_set.insert(segment(current_vertex,limit_vertex));
            current_vertex=mysegment.first;
            limit_vertex=mysegment.second;
        }
        else
            // we keep doing the next interval
        if (limit_vertex<mysegment.second) limit_vertex=mysegment.second;
        // finally we need to keep track of the last segment
        if (mysegment==maxsegment)
        {
            handle_set.insert(segment(current_vertex,limit_vertex));
        }
    }
    // we restart the loop over set_of_segments
    // this time replacing edges by larger edges.
    long long int a;
    edge_data new_edge;
    for (auto & redg : raw_edges)
    {
        a=redg.first.first;
        it_set=handle_set.upper_bound(segment(a+1,a+1));
        if (it_set!=handle_set.end())
        {
            it_set--;
            upfirst=it_set;
        }
        else
            upfirst=--(handle_set.end());
        a=redg.second.first;
        it_set=handle_set.upper_bound(segment(a+1,a+1));
        if (it_set!=handle_set.end())
        {
            it_set--;
            upsecond=it_set;
        }
        else
            upsecond=--(handle_set.end());
        new_edge=edge_data(*upfirst,*upsecond,redg.extra1,redg.extra2,redg.pos_in_file);
        // check if the new edge is already there
        // this handles validating.
        //if (std::find(return_edge.begin(),return_edge.end(),new_edge) == return_edge.end())
        return_edge.insert(new_edge);
    }
    return return_edge;
}

std::vector<all_edges> filter_in_ccd(all_edges my_edges, std::set<segment> ccd_data)
{
    // the procedure runs through all edges in a vector all_edges
    // we split edges according to ccd
    // all_edges contains edges of the same chromosome
    all_edges parsed_edge;
    std::vector<all_edges> return_vector;
    all_edges current;
    auto it = my_edges.begin();
    for (auto & ccd  : ccd_data)
    {
        current.clear();
        it=my_edges.begin();
        while (it!=my_edges.end())
        {
            if (((it->first).second>=ccd.first) and ((it->second).first<=ccd.second))
            {
                current.insert(*it);
                it=my_edges.erase(it);
            }
            else
            {
                ++it;
            }
            if (((it->first).first)>ccd.second) // if this happens, we know that we are past the ccd, we set the iterator to the end
                it=my_edges.end();
        }
        return_vector.push_back(current);
    }
    // last vector contains edges not fitting any ccd.
    return_vector.push_back(my_edges);
    //
    // now we use remove edges with the same beginning and end
    std::cout << "Removing double edges." << std::endl;
    all_edges::iterator j;
    for (auto & test_vector : return_vector)
        for (all_edges::iterator it=test_vector.begin(); it!=test_vector.end(); ++it)
        {
            j=it;
            j++;
            while (j!=test_vector.end())
            {
                if (((j->first) == (it->first)) and ((j->second) == (it->second)))
                {
                    j=test_vector.erase(j);
                }
                else
                    j=test_vector.end();
            }
        }
    return return_vector;
}

segment_array get_ccd_from_file(std::string ccd_file_name)
{
    std::ifstream ccd_file(ccd_file_name);
    std::string handle_string;
    std::vector<std::string> my_split_string;
    segment_array ccd_array;
    int chrom;
    long long int a,b;
    while (std::getline(ccd_file,handle_string))
    {
        my_split_string.clear();
        boost::split(my_split_string, handle_string, boost::is_any_of(" \t"));
        if (my_split_string.size()>2)
        {
            try
            {
                chrom=chrom_from_chr(my_split_string[0]);
                a = std::stol(my_split_string[1]);
                b = std::stol(my_split_string[2]);
                if ((chrom>0) and (chrom<26)) ccd_array.at(chrom).insert(segment(a,b));
            }
            catch (...) {}
        }
    }
    ccd_file.close();
    return ccd_array;
}

void export_to_file(all_edges my_edg, std::string savename, int chrom=-1)
{
    std::set<long long int> vertices;
    std::string mychrom=chr_from_chrom(chrom);
    for (auto & edgs : my_edg)
    {
        vertices.insert(edgs.first.first);
        vertices.insert(edgs.first.second);
        vertices.insert(edgs.second.second);
        vertices.insert(edgs.second.first);
    }
    std::ofstream savefile(savename);
    for(auto & endpoint : vertices)
    {
        char *t = new char[mychrom.size() + 15];
        sprintf(t, "%s_%010d", mychrom.c_str(), endpoint);
        savefile << "NODE "<<std::string(t) << std::endl;
        delete [] t;
    }
    for (auto & myedge : my_edg)
    {
        char *t1 = new char[mychrom.size() + 15];
        sprintf(t1, "%s_%010d", mychrom.c_str(), myedge.first.first);

        char *t2 = new char[mychrom.size() + 15];
        sprintf(t2, "%s_%010d", mychrom.c_str(), myedge.second.first);
        savefile << "EDGE " << t1 << " " << t2 << " " << myedge.extra1 << " " << myedge.pos_in_file << std::endl;
    }
    savefile.close();
    // we are now save in the tr format for jdrasil
    // the file name has "tr" extension.
}

int pop_for_chromosome(std::string filename)
{
    // the
    std::ifstream myfile(filename);
    int chrom;
    bool is_bad;
    std::string handle_string;
    std::vector<std::string> my_split_string;
    long long int a,b,c,d;
    while (std::getline(myfile,handle_string))
    {
        std::cout << "+";
        my_split_string.clear();
        is_bad = false;
        boost::split(my_split_string, handle_string, boost::is_any_of(" \t"));
        if (my_split_string.size()<7) is_bad = true;
        else
        {
            try
            {
                chrom = chrom_from_chr(my_split_string[0]);
                a = std::stol(my_split_string[1]);
                b = std::stol(my_split_string[2]);
                c = std::stol(my_split_string[4]);
                d = std::stol(my_split_string[5]);
                if (b>10*a) is_bad = true;
                if (d>10*c) is_bad = true;
                if (a>10*b) is_bad = true;
                if (c>10*d) is_bad = true;
                if ((chrom==-1) or (chrom>25)) is_bad = true;
            }
            catch (...) { is_bad = true; }

            if (my_split_string[0].size()>5) is_bad = true;
            if (my_split_string[3].size()>5) is_bad = true;
            if (my_split_string[0].find("chr")!=0) is_bad = true;
            if (my_split_string[3].find("chr")!=0) is_bad = true;
            // we do not allow edges connecting different chromosomes.
            if (chrom!=chrom_from_chr(my_split_string[3])) is_bad=true;
        }
        if (!is_bad)
        {
            myfile.close();
            return chrom;
        }
    }
    myfile.close();
    return -1;
}

void handle_file(std::string filename, int chromosome, bool split_ccd, std::string ccd_filename = std::string(""))
{
    std::cout << "Popping for chrom in " << filename << std::endl;
    int chroms = pop_for_chromosome(filename);
    if (chroms == -1) {
        // this means all vertices are bad
        // we do not want to work with this file
        return;
    }
    if (chromosome < 1)
        chromosome=chroms;
    std::cout << "Handling chromosome " << chromosome << std::endl;

    edge_array from_file= read_lines_from_file(filename);
    all_edges work_with_this = glue_segments(from_file.at(chromosome));

    if (!split_ccd)
    {
        export_to_file(work_with_this, filename+std::string(".mp"),chromosome);
        return;
    } else {
        segment_array from_ccd = get_ccd_from_file(ccd_filename);
        std::set<segment> ccd_data = from_ccd.at(chromosome);

        std::vector<all_edges> split_edges = filter_in_ccd(work_with_this, ccd_data);
        int ccd_no = 1;

        // now we start to write all the edges using savefile_procedure

        for (auto &myedge :split_edges) {
            std::string fileNumber = pad_int(ccd_no);
            std::string chrNumber = pad_int(chromosome);
            export_to_file(myedge, filename + std::string(".") + fileNumber + std::string(".chr") + chrNumber + std::string(".mp"), chromosome);
            ccd_no++;
        }
    }
}


int main(int argc, char** argv) {

    int which_chrom;
    std::string ccdFileame;
    std::string filename;
    try {
        po::options_description description("Options");
        description.add_options()
                ("help,h", "show help")
                ("ccd_split,s", "split into ccd's")
                ("chromosome,c", po::value<int>(&which_chrom)->implicit_value(-1), "which chromosome")
                ("ccdfile,d",po::value<std::string>(&ccdFileame),"path to file with CCD info (.bed)")
                ("file,f",po::value<std::string>(&filename),"name of file to parse");

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, description), vm);
        po::notify(vm);

        if (vm.count("help")) {
            std::cout << description << "\n";
            return 0;
        }
        bool do_ccd = false;
        if (vm.count("ccd_split")) do_ccd=true;

        fs::path filepath(filename);
        fs::path ccdFilepath(ccdFileame);


        if (fs::is_regular_file(filepath))
        {
            if (do_ccd && !fs::is_regular_file(ccdFilepath)) {
                throw "File ccdfile is not correct.";
            }
            std::cout << "Handling file" << std::endl;
            handle_file(filename, which_chrom, do_ccd, ccdFileame);
            return 0;
        }

        return 0;
    }
    catch (...) {};

}
