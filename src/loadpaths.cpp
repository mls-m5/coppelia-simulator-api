
#include "loadpaths.h"
#include <fstream>
#include <iostream>
#include <sstream>

using namespace std;

Paths loadPaths(string filename) {
    ifstream file(filename);

    Paths ret;

    vector<Position> current;
    string currentName;

    if (!file.is_open()) {
        cout << "could not load paths " << filename << endl;
    }

    for (string line; getline(file, line);) {
        if (line.empty()) {
            continue;
        }
        if (line.front() == '#') {
            ret[currentName] = current;
            if (!current.empty()) {
                ret[currentName] = current;
            }
            current.clear();

            currentName = line.substr(1);
        }
        else {
            istringstream ss(line);
            Position pos;
            ss >> pos.x >> pos.y;
            current.push_back(pos);
        }
    }

    if (!current.empty()) {
        ret[currentName] = current;
    }

    return ret;
}
