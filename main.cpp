//
//  File.cpp
//  MapAbstraction
//
//  Created by Nathan Sturtevant on 7/11/13.
//  Modified by Steve Rabin 12/15/14
//
//
#include "stdafx.h"
#include <vector>
#include <cstdio>
#include <cstdint>
#include <numeric>
#include <algorithm>
#include "Timer.h"
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include "Wrapper.h"
#include "Log.h"

void LoadMap(const char *fname, std::vector<bool> &map, int &w, int &h);

struct stats {
    std::vector<double> times;
    std::vector<xyLoc> path;
    std::vector<int> lengths;

    double GetTotalTime() {
        return std::accumulate(times.begin(), times.end(), 0.0);
    }

    double GetMaxTimestep() {
        return *std::max_element(times.begin(), times.end());
    }

    double Get20MoveTime() {
        for (unsigned int x = 0; x < lengths.size(); x++)
            if (lengths[x] >= 20)
                return std::accumulate(times.begin(), times.begin() + 1 + x, 0.0);
        return GetTotalTime();
    }

    double GetPathLength() {
        double len = 0;
        for (int x = 0; x < (int) path.size() - 1; x++) {
            if (path[x].x == path[x + 1].x || path[x].y == path[x + 1].y) {
                len++;
            } else {
                len += 1.4142;
            }
        }
        return len;
    }

    bool ValidatePath(int width, int height, const std::vector<bool> &mapData) {
        for (int x = 0; x < (int) path.size() - 1; x++) {
            if (abs(path[x].x - path[x + 1].x) > 1)
                return false;
            if (abs(path[x].y - path[x + 1].y) > 1)
                return false;
            if (!mapData[path[x].y * width + path[x].x])
                return false;
            if (!mapData[path[x + 1].y * width + path[x + 1].x])
                return false;
            if (path[x].x != path[x + 1].x && path[x].y != path[x + 1].y) {
                if (!mapData[path[x + 1].y * width + path[x].x])
                    return false;
                if (!mapData[path[x].y * width + path[x + 1].x])
                    return false;
            }
        }
        return true;
    }
};

int main(int argc, char *argv[]) {
    double allTestsTotalTime = 0;
    Timer *timer = new Timer();
    std::vector<bool> mapData;
    int mapSize = 100;
    int buildings[3][6] = {{2, 0, 2, 98, 0, 1},
                           {4, 1, 4, 99, 1, 1},
                           {6, 0, 6, 98, 0, 1}};

    //Construct map.
    for (int i = 0; i < mapSize; i++) {
        for (int j = 0; j < mapSize; j++) {
            mapData.push_back(true);
        }
    }

    // Init obstacles.
    for (int i = 0; i < 3; i++) {
        for (int x = buildings[i][0]; x <= buildings[i][2]; x++) {
            for (int y = buildings[i][1]; y <= buildings[i][3]; y++) {
                mapData[x + y * mapSize] = false;
            }
        }
    }

    JPSPWrapper *jpgb = new JPSPWrapper(mapData, mapSize, mapSize);
    jpgb->Preprocess();

    timer->StartTimer();

    xyLoc *start = new xyLoc(0, 0), *end = new xyLoc(0, 0);
    std::vector<xyLoc> *path = jpgb->GetPath((xyLoc &) *start, (xyLoc &) *end);

    allTestsTotalTime += timer->EndTimer();

    if (path->size() > 0) {
        printf("Path found: ");
        for (unsigned int t = 0; t < path->size(); t++) {
            if (t != path->size() - 1) {
                if (t % 10 == 0) {
                    printf("\n");
                } else {
                    printf("(%d, %d)->", (*path)[t].x, (*path)[t].y);
                }
            } else {
                printf("(%d, %d)\n", (*path)[t].x, (*path)[t].y);
            }
        }
    } else {
        printf("Path not found.\n");
    }
    (*path).clear();

    printf("All tests total time: %f", allTestsTotalTime);

    getchar();

    return 0;
}
