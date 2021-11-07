/*
Original code by Marcin Pilipczuk
*/

#ifndef NAMESDICT_H

#include <string>
#include <map>
#include <vector>

class NamesDict {
public:
    std::map<std::string, int> names_map;
    std::vector<std::string> names;

    NamesDict() {};

    int name2id(const std::string &name) {
        if (names_map.count(name) == 0) {
            names_map[name] = (int)names.size();
            names.push_back(name);
        }
        return names_map[name];
    }

    const std::string &id2name(int id) const {
        return names[id];
    }

    void clear() {
        names_map.clear();
        names.clear();
    }

    int size() const {
        return (int)names.size();
    }
};

#define NAMESDICT_H
#endif // NAMESDICT_H