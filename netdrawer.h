#include "network.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <map>
#include "math.h"
#include "nnode.h"
#include "link.h"
#include <string>

using namespace NEAT;

void setPosition(double &x, double &y);

void printNode(NNode* node, double x, double y, std::ofstream &myfile);

void printLink(Link* nlink, std::map<NNode*, int> npos, std::vector<double> xpos, std::vector<double> ypos, std::ofstream &myfile);

int netdrawer(const Network* network, int solution);