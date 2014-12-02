#include "network.h"
#include "nnode.h"
#include "link.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <map>
#include "math.h"

using namespace NEAT;

//void setPosition(double &x, double &y, double Min, double Max){
void setPosition(double &x, double &y){
	//randomly generate position for a node;
	double Min = 0.0, Max = 1000.0;
	double p = (double)rand() / RAND_MAX;
	x = Min + p * (Max - Min);
	x = (floor(x * 10))/10; //only keep one digit after the decimal point
	
	double q = (double)rand() / RAND_MAX;
	y = Min + q * (Max - Min);
	y = (floor(y * 10))/10;
	
	return;
}

void printNode(NNode* node, double x, double y, std::ofstream &myfile){

	int type = node->type;
	//nodetype is 1, which is transition
	if(type){
		myfile << "<transition id=\"" << node->node_id << "\">\n";
		myfile << "<graphics>\n";
		myfile << "<position x=\"" << x << "\" y=\"" << y << "\"/>\n";
		myfile << "</graphics>\n";
		myfile << "<name>\n";
		myfile << "<value>" << node->node_id << "</value>\n";
		myfile << "<graphics/>\n";
		myfile << "</name>\n";
		myfile << "<orientation>\n";
		myfile << "<value>90</value>\n";
		myfile << "</orientation>\n";
		myfile << "<rate>\n";
		myfile << "<value>1.0</value>\n";
		myfile << "</rate>\n";
		myfile << "<timed>\n";
		myfile << "<value>false</value>\n";
		myfile << "</timed>\n";
		myfile << "</transition>\n";				
	}
	else{ ////nodetype is 0, which is place
		myfile << "<place id=\"" << node->node_id << "\">\n";
		myfile << "<graphics>\n";
		myfile << "<position x=\"" << x << "\" y=\"" << y << "\"/>\n";
		myfile << "</graphics>\n";
		myfile << "<name>\n";
		myfile << "<value>" << node->node_id << "</value>\n";
		myfile << "<graphics/>\n";
		myfile << "</name>\n";
		myfile << "<initialMarking>\n";
		myfile << "<value>" << node->tok_count << "</value>\n";
		myfile << "<graphics/>\n";
		myfile << "</initialMarking>\n";
		myfile << "</place>\n";
	}
		 
}

void printLink(Link* nlink, std::map<NNode*, int> npos, std::vector<double> xpos, std::vector<double> ypos, std::ofstream &myfile){
	NNode* in = nlink->in_node;
	NNode* out = nlink->out_node;
	int in_ptr = npos[in];
	int out_ptr = npos[out];
	
	myfile << "<arc id=\"" << in->node_id << " to " << out->node_id << "\" ";
	myfile << "source=\"" << in->node_id << "\" target=\"" << out->node_id <<"\">\n";
	myfile << "<graphics/>\n";
	myfile << "<inscription>\n";
	myfile << "<value>" << (int)nlink->weight << "</value>\n";
	myfile << "<graphics/>\n";
	myfile << "</inscription>\n";
	myfile << "<arcpath id=\"000\" x=\"" << ((int)xpos[in_ptr]) + 11;
	myfile << "\" y=\"" << ((int)ypos[in_ptr]) + 5 << "\" curvePoint=\"false\"/>\n";
	myfile << "<arcpath id=\"001\" x=\"" << ((int)xpos[out_ptr]) + 11;
	myfile << "\" y=\"" << ((int)ypos[out_ptr]) + 5 << "\" curvePoint=\"false\"/>\n";
	myfile << "</arc>\n";
}


int main(){

/*	
	//test setPosition
	double x = 0.0, y = 0.0;
	for(int i = 0; i < 10; i++){
		setPosition(x, y);
		std::cout << " x = " << x << " y = " << y << std::endl;
	}
*/
/*	//testing printNode	
	std::ofstream myfile;
	myfile.open( "net.xml");
	myfile <<"<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>\n<pnml>\n<net id=\"Net-One\" type=\"P/T net\">";
	
	NNode* nodep = new NNode(TRANSITION, 1111);
	NNode* nodet = new NNode(PLACE, 2222);
	
	double x = 0.0, y = 0.0;
	setPosition(x, y);
	printNode(nodep, x, y, myfile);
	
	setPosition(x, y);
	printNode(nodet, x, y, myfile);
	
	//closing xml file
	myfile <<"</net>\n</pnml>";
	myfile.close();
*/
	//testing printLink
	std::ofstream myfile;
	myfile.open( "net.xml");
	myfile <<"<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>\n<pnml>\n<net id=\"Net-One\" type=\"P/T net\">";
	
	//a hashmap to record posiitons of nodes using node pointer (NNode*) and position number(int)
	std::map<NNode*, int> nodepos; 
	//two vectors to record positions of the nodes
	std::vector<double> xpos; 
	std::vector<double> ypos;
	NNode* nodep = new NNode(TRANSITION, 1111);
	NNode* nodet = new NNode(PLACE, 2222);
	
	double x = 0.0, y = 0.0;
	int count = 0;

	setPosition(x, y);
	nodepos[nodep] = count;
	xpos.push_back(x);
	ypos.push_back(y);
	printNode(nodep, x, y, myfile);

	count++;
	setPosition(x, y);
	nodepos[nodet] = count;
	xpos.push_back(x);
	ypos.push_back(y);
	printNode(nodet, x, y, myfile);
	
	Link* nlink = new Link(2, nodep, nodet);
	printLink(nlink, nodepos, xpos, ypos, myfile);
	
	//closing xml file
	myfile <<"</net>\n</pnml>";
	myfile.close();
	
	return 0;
}


