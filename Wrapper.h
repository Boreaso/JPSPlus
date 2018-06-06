#include <iostream>
#include <vector>
#include "Definition.h"
#include "JPSPlus.h"

class JPSPWrapper {
private:
	JPSPlus * jpsPlus;
	int w = 0;
	int h = 0;
	std::vector<bool> bits;
public:
	JPSPWrapper() {};
	JPSPWrapper(std::vector<bool> &bits, int w, int h);
	~JPSPWrapper();
	void Preprocess();
	std::vector<xyLoc> *GetPath(xyLoc &s, xyLoc &g);
};
