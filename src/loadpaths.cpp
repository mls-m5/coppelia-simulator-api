
#include "loadpaths.h"
#include <fstream>
#include <iostream>
#include <sstream>

using namespace std;

Paths loadPaths(string filename) {
    ifstream file(filename);

    Paths ret;

    vector<Position> currentPath;
    //    string currentName;
    size_t currentIndex;

    if (!file.is_open()) {
        cout << "could not load paths " << filename << endl;
    }

    auto setPath = [&](size_t index, Path path) {
        if (index >= ret.size()) {
            ret.resize(index + 1);
        }
        ret.at(index) = std::move(path);
    };

    for (string line; getline(file, line);) {
        if (line.empty()) {
            continue;
        }
        if (line.front() == '#') {
            if (!currentPath.empty()) {
                setPath(currentIndex, std::move(currentPath));
            }
            currentPath.clear();

            currentIndex = stoul(line.substr(1)) - 1;
        }
        else {
            istringstream ss(line);
            Position pos;
            ss >> pos.x >> pos.y;
            currentPath.push_back(pos);
        }
    }

    if (!currentPath.empty()) {
        setPath(currentIndex, std::move(currentPath));
    }

    return ret;
}
