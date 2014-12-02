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


int main(){
	
	//test setPosition
	double x = 0.0, y = 0.0;
	
	for(int i = 0; i < 10; i++){
		setPosition(x, y);
		std::cout << " x = " << x << " y = " << y << std::endl;
	}
	
	return 0;
}


