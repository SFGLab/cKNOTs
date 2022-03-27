#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <array>
#include <random>
#include <ctime>
#include <cstring>
#include <map>
#include <utility>
#include <boost/algorithm/string.hpp>
#include <sys/stat.h>

typedef std::vector<std::string> filelines;

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
edge_array read_lines_from_file(std::string filename, unsigned int filter)
{
  std::ifstream myfile(filename);
  std::ofstream badfile(filename + "_bad");
  std::string handle_string;
  std::vector<std::string> my_split_string;

  edge_array myarray;
  bool is_bad;
  long long int a, b, c, d;
  int chrom;
  long long int line_number=0;
  std::cout << "+" << std::endl;
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
    if (std::stol(my_split_string[6])<filter) is_bad=true;
      }
      catch (...) { is_bad = true; }

      if (my_split_string[0].size()>5) is_bad = true;
      if (my_split_string[3].size()>5) is_bad = true;
      if (my_split_string[0].find("chr")!=0) is_bad = true;
      if (my_split_string[3].find("chr")!=0) is_bad = true;
      // we do not allow edges connecting different chromosomes.
      if (chrom!=chrom_from_chr(my_split_string[3])) is_bad=true;
    }

    if (is_bad)
      badfile << handle_string << std::endl;
    else {
      if (my_split_string.size() < 7) my_split_string.push_back("0");
      if (my_split_string.size() < 7) my_split_string.push_back("0");
      myarray.at(chrom).insert(
      edge_data(segment(a, b), segment(c, d), my_split_string[6], my_split_string[7], line_number));
    }
  }
  badfile.close();
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
  for (auto redg : raw_edges)
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
void handle_set_of_data(std::string filename, std::string ccd_file_name, unsigned int filter)
{
  all_edges working_edge;
  edge_array from_file=read_lines_from_file(filename, filter);
  std::cout << "Successfully read lines from file" << std::endl;
  segment_array from_ccd=get_ccd_from_file(ccd_file_name);
  std::cout << "Successfully read ccd from file" << std::endl;
  std::set<segment> ccd_data;
  std::vector<all_edges> split_edges;
  std::string mychrom;
  std::string savename;
  std::set<long long int> vertices;
  std::map<long long int, long long int> vertexmap;
  for (int i=1; i<26; i++)
  {
    std::cout << "Processing chromosome " << i << std::endl;
    mychrom=chr_from_chrom(i);

    std::cout << "Gluing segments" << std::endl;
    working_edge=glue_segments(from_file.at(i));

    std::cout << "Splitting into ccd's" << std::endl;
    ccd_data=from_ccd.at(i);
    split_edges=filter_in_ccd(working_edge, ccd_data);

    std::cout << "Saving file" << std::endl;
    mkdir((filename+"_"+mychrom).c_str(),0777);
    for (int j=0; j<split_edges.size(); j++)
    {
      vertices.clear();
      for (auto & edgs : split_edges.at(j))
      {
    vertices.insert(edgs.first.first);
    vertices.insert(edgs.first.second);
    vertices.insert(edgs.second.second);
    vertices.insert(edgs.second.first);
      }
      savename=filename+"_"+mychrom+"/ccd_";
      if (j<split_edges.size()-1)
      {
    savename=savename+std::to_string(j);
      }
      else
      {
    savename=savename+"extra_domain";
      }
      std::ofstream savefile(savename);
      for(auto & endpoint : vertices)
      {
    char *t = new char[mychrom.size() + 15];
    sprintf(t, "%s_%010d", mychrom.c_str(), endpoint);
    savefile << "NODE "<<std::string(t) << std::endl;
    delete [] t;
      }
      for (auto & myedge : split_edges.at(j))
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
      std::ofstream savetrfile(filename+"_"+mychrom+"/ccd_"+std::to_string(j)+".tr");
      savetrfile << "p " << vertices.size() << " " << vertices.size()+split_edges.at(j).size()-1 << std::endl;
      savetrfile << "c first come edges on the chromatine" << std::endl;
      vertexmap.clear();
      long long int jj=0;
      for (auto & endpoint : vertices)
      {
    jj++;
    {
      savetrfile << jj << " " << jj+1 << std::endl;
      vertexmap[endpoint]=jj;
    }
      }
      savetrfile << "c next we deal with interactions" << std::endl;
      for (auto & myedge : split_edges.at(j))
      {
    try
    {
      savetrfile << vertexmap[myedge.first.first] << " " << vertexmap[myedge.second.first] << std::endl;}
    catch (...)
    {
      std::cout << "Cant save map " << myedge.first.first << " " << myedge.second.first << std::endl;
    }
      }
      savetrfile.close();
    }
    std::cout << "Done" << std::endl;
  }
}


int main(int argc, char** argv) {
  std::string filename=argv[1];
  std::cout << filename << std::endl;
  std::string ccdname=argv[2];
  unsigned int filter=0;
  try
  {
    filter=std::stol(argv[3]);
  }
  catch (...) {};

  std::cout << ccdname << std::endl;
  handle_set_of_data(filename,ccdname,filter);
  return 0;
}
