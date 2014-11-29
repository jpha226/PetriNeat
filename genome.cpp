/*
 Copyright 2001 The University of Texas at Austin

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/
#include "genome.h"

#include <iostream>
#include <cmath>
#include <sstream>
using namespace NEAT;

Genome::Genome(int id, std::vector<NNode*> n, std::vector<Gene*> g) {
	genome_id=id;
	//traits=t;
	nodes=n; 
	genes=g;
}


Genome::Genome(int id, std::vector<NNode*> n, std::vector<Link*> links) {
	std::vector<Link*>::iterator curlink;
	Gene *tempgene;
	
	nodes=n;

	genome_id=id;

	//We go through the links and turn them into original genes
	for(curlink=links.begin();curlink!=links.end();++curlink) {
		//Create genes one at a time
		tempgene=new Gene((*curlink)->weight,(*curlink)->in_node,(*curlink)->out_node,1.0,0.0);
		genes.push_back(tempgene);
	}

}

Genome::Genome(const Genome& genome)
{
	genome_id = genome.genome_id;

	std::vector<NNode*>::const_iterator curnode;
	std::vector<Gene*>::const_iterator curgene;



	//Duplicate NNodes
	for(curnode=genome.nodes.begin();curnode!=genome.nodes.end();++curnode) {
		//First, find the trait that this node points to

		NNode* newnode=new NNode(*curnode);

		(*curnode)->dup=newnode;  //Remember this node's old copy
		//    (*curnode)->activation_count=55;
		nodes.push_back(newnode);    
	}

	NNode *inode; //For forming a gene 
	NNode *onode; //For forming a gene

	//Duplicate Genes
	for(curgene=genome.genes.begin(); curgene!=genome.genes.end(); ++curgene) {
		//First find the nodes connected by the gene's link

		inode=(((*curgene)->lnk)->in_node)->dup;
		onode=(((*curgene)->lnk)->out_node)->dup;

		Gene* newgene=new Gene(*curgene,inode,onode);
		genes.push_back(newgene);

	}
}

Genome::Genome(int id, std::ifstream &iFile) {

	char curword[128];  //max word size of 128 characters
	char curline[1024]; //max line size of 1024 characters
	char delimiters[] = " \n";

	int done=0;

	//int pause;

	genome_id=id;

	iFile.getline(curline, sizeof(curline));
	int wordcount = NEAT::getUnitCount(curline, delimiters);
	int curwordnum = 0;

	//Loop until file is finished, parsing each line
	while (!done) {

        //std::cout << curline << std::endl;

		if (curwordnum > wordcount || wordcount == 0) {
			iFile.getline(curline, sizeof(curline));
			wordcount = NEAT::getUnitCount(curline, delimiters);
			curwordnum = 0;
		}
        
        std::stringstream ss(curline);
		//strcpy(curword, NEAT::getUnit(curline, curwordnum++, delimiters));
        ss >> curword;

		//printf(curword);
		//printf(" test\n");
		//Check for end of Genome
		if (strcmp(curword,"genomeend")==0) {
			//strcpy(curword, NEAT::getUnit(curline, curwordnum++, delimiters));
            ss >> curword;
			int idcheck = atoi(curword);
			//iFile>>idcheck;
			if (idcheck!=genome_id) printf("ERROR: id mismatch in genome");
			done=1;
		}

		//Ignore genomestart if it hasn't been gobbled yet
		else if (strcmp(curword,"genomestart")==0) {
			++curwordnum;
			//cout<<"genomestart"<<endl;
		}

		//Ignore comments surrounded by - they get printed to screen
		else if (strcmp(curword,"/*")==0) {
			//strcpy(curword, NEAT::getUnit(curline, curwordnum++, delimiters));
            ss >> curword;
			while (strcmp(curword,"*/")!=0) {
				//cout<<curword<<" ";
				//strcpy(curword, NEAT::getUnit(curline, curwordnum++, delimiters));
                ss >> curword;
			}
			//cout<<endl;
		}

		//Read in a trait
		else if (strcmp(curword,"trait")==0) {
			

			char argline[1024];
			//strcpy(argline, NEAT::getUnits(curline, curwordnum, wordcount, delimiters));

			curwordnum = wordcount + 1;

            ss.getline(argline, 1024);
			//Allocate the new trait
			

			//Add trait to vector of traits
			//traits.push_back(newtrait);
		}

		//Read in a node
		else if (strcmp(curword,"node")==0) {
			NNode *newnode;

			char argline[1024];
			//strcpy(argline, NEAT::getUnits(curline, curwordnum, wordcount, delimiters));
			curwordnum = wordcount + 1;
            
            ss.getline(argline, 1024);
			//Allocate the new node
			newnode=new NNode(argline);

			//Add the node to the list of nodes
			nodes.push_back(newnode);
		}

		//Read in a Gene
		else if (strcmp(curword,"gene")==0) {
			Gene *newgene;

			char argline[1024];
			//strcpy(argline, NEAT::getUnits(curline, curwordnum, wordcount, delimiters));
			curwordnum = wordcount + 1;

            ss.getline(argline, 1024);
            //std::cout << "New gene: " << ss.str() << std::endl;
			//Allocate the new Gene
            newgene=new Gene(argline,nodes);

			//Add the gene to the genome
			genes.push_back(newgene);

            //std::cout<<"Added gene " << newgene << std::endl;
		}

	}

}


Genome::Genome(int new_id,int p, int t, double linkprob) {
	int totalnodes;
	bool *cm; //The connection matrix which will be randomized
	bool *cmp; //Connection matrix pointer
	int matrixdim;
	int count;

	int ncount; //Node and connection counters
	//int ccount;

	int row;  //For navigating the matrix
	int col;

	double new_weight;

	int maxnode; //No nodes above this number for this genome

	int first_transition; //Number of first output node

	totalnodes=p+t;
	matrixdim=2*p*t;
	cm=new bool[matrixdim];  //Dimension the connection matrix
	maxnode=p+t;

	first_transition=totalnodes-t+1;

	//For creating the new genes
	NNode *newnode;
	Gene *newgene;
	NNode *in_node;
	NNode *out_node;

	//Retrieves the nodes pointed to by connection genes
	std::vector<NNode*>::iterator node_iter;

	//Assign the id
	genome_id=new_id;

	//cout<<"Assigned id "<<genome_id<<endl;

	//Step through the connection matrix, randomly assigning bits
	cmp=cm;
	for(count=0;count<matrixdim;count++) {
		if (randfloat()<linkprob)
			*cmp=true;
		else *cmp=false;
		cmp++;
	}

	//Build the place nodes
	for(ncount=1;ncount<=p;ncount++) {

		newnode=new NNode(PLACE,ncount);


		//Add the node to the list of nodes
		nodes.push_back(newnode);
	}

	//Build the transition nodes
	for(ncount=first_transition; ncount<=totalnodes; ncount++) {
		newnode=new NNode(TRANSITION,ncount);
		
		//Add the node to the list of nodes
		nodes.push_back(newnode);
	}

	//cout<<"Built nodes"<<endl;

	//Connect the nodes 
	//ccount=1;  //Start the connection counter

	//Step through the connection matrix, creating connection genes
	cmp=cm;
	bool* cmp2 = cmp + p * t;
	count=0;
	for(col=1;col<=p;col++)
		for(row=first_transition;row<=p+t;row++) {
			//Only try to create a link if it is in the matrix
			//and not leading into a sensor
					
			if(col <= p && row <= maxnode){
		
					if (*cmp==true) {
						//Retrieve the in_node
						node_iter=nodes.begin();
						while((*node_iter)->node_id!=row)
							node_iter++;
						in_node=(*node_iter);
						//Retrieve the out_node
						node_iter=nodes.begin();
						while((*node_iter)->node_id!=col)
							node_iter++;
						out_node=(*node_iter);
						//Create the gene
						new_weight=(int)(10*randposneg()*randfloat());
						newgene=new Gene(new_weight, in_node, out_node,count,new_weight);
						//Add the gene to the genome
						genes.push_back(newgene);
					}else if(*cmp2 == true){
						//Retrieve the in_node
                                                node_iter=nodes.begin();
                                                while((*node_iter)->node_id!=col)
                                                        node_iter++;
                                                in_node=(*node_iter);
                                                //Retrieve the out_node
                                                node_iter=nodes.begin();
                                                while((*node_iter)->node_id!=row)
                                                        node_iter++;
                                                out_node=(*node_iter);
                                                //Create the gene
                                                new_weight=(int)(10*randposneg()*randfloat());
                                                newgene=new Gene(new_weight, in_node, out_node,count,new_weight);
                                                //Add the gene to the genome
                                                genes.push_back(newgene);
					}

			}
				count++; //increment gene counter	    
				cmp++;
				cmp2++;
		}

		delete [] cm;

}

/*
Genome::Genome(int num_in,int num_out,int num_hidden,int type) {

	//Temporary lists of nodes
	std::vector<NNode*> inputs;
	std::vector<NNode*> outputs;
	std::vector<NNode*> hidden;
	NNode *bias; //Remember the bias

	std::vector<NNode*>::iterator curnode1; //Node iterator1
	std::vector<NNode*>::iterator curnode2; //Node iterator2
	std::vector<NNode*>::iterator curnode3; //Node iterator3

	//For creating the new genes
	NNode *newnode;
	Gene *newgene;


	int count;
	int ncount;


	//Assign the id 0
	genome_id=0;

	//Create a dummy trait (this is for future expansion of the system)
	
	traits.push_back(newtrait);

	//Adjust hidden number
	if (type==0) 
		num_hidden=0;
	else if (type==1)
		num_hidden=num_in*num_out;

	//Create the inputs and outputs

	//Build the input nodes
	for(ncount=1;ncount<=num_in;ncount++) {
		if (ncount<num_in)
			newnode=new NNode(SENSOR,ncount,INPUT);
		else { 
			newnode=new NNode(SENSOR,ncount,BIAS);
			bias=newnode;
		}

		//newnode->nodetrait=newtrait;

		//Add the node to the list of nodes
		nodes.push_back(newnode);
		inputs.push_back(newnode);
	}

	//Build the hidden nodes
	for(ncount=num_in+1;ncount<=num_in+num_hidden;ncount++) {
		newnode=new NNode(NEURON,ncount,HIDDEN);
		//newnode->nodetrait=newtrait;
		//Add the node to the list of nodes
		nodes.push_back(newnode);
		hidden.push_back(newnode);
	}

	//Build the output nodes
	for(ncount=num_in+num_hidden+1;ncount<=num_in+num_hidden+num_out;ncount++) {
		newnode=new NNode(NEURON,ncount,OUTPUT);
		//newnode->nodetrait=newtrait;
		//Add the node to the list of nodes
		nodes.push_back(newnode);
		outputs.push_back(newnode);
	}

	//Create the links depending on the type
	if (type==0) {
		//Just connect inputs straight to outputs

		count=1;

		//Loop over the outputs
		for(curnode1=outputs.begin();curnode1!=outputs.end();++curnode1) {
			//Loop over the inputs
			for(curnode2=inputs.begin();curnode2!=inputs.end();++curnode2) {
				//Connect each input to each output
				newgene=new Gene(newtrait,0, (*curnode2), (*curnode1),count,0);

				//Add the gene to the genome
				genes.push_back(newgene);	 

				count++;

			}

		}

	} //end type 0
	//A split link from each input to each output
	else if (type==1) {
		count=1; //Start the gene number counter

		curnode3=hidden.begin(); //One hidden for ever input-output pair
		//Loop over the outputs
		for(curnode1=outputs.begin();curnode1!=outputs.end();++curnode1) {
			//Loop over the inputs
			for(curnode2=inputs.begin();curnode2!=inputs.end();++curnode2) {

				//Connect Input to hidden
				newgene=new Gene(newtrait,0, (*curnode2), (*curnode3),count,0);
				//Add the gene to the genome
				genes.push_back(newgene);

				count++; //Next gene

				//Connect hidden to output
				newgene=new Gene(newtrait,0, (*curnode3), (*curnode1),count,0);
				//Add the gene to the genome
				genes.push_back(newgene);

				++curnode3; //Next hidden node
				count++; //Next gene

			}
		}

	}//end type 1
	//Fully connected 
	else if (type==2) {
		count=1; //Start gene counter at 1


		//Connect all inputs to all hidden nodes
		for(curnode1=hidden.begin();curnode1!=hidden.end();++curnode1) {
			//Loop over the inputs
			for(curnode2=inputs.begin();curnode2!=inputs.end();++curnode2) {
				//Connect each input to each hidden
				newgene=new Gene(newtrait,0, (*curnode2), (*curnode1),count,0);

				//Add the gene to the genome
				genes.push_back(newgene);	 

				count++;

			}
		}

		//Connect all hidden units to all outputs
		for(curnode1=outputs.begin();curnode1!=outputs.end();++curnode1) {
			//Loop over the inputs
			for(curnode2=hidden.begin();curnode2!=hidden.end();++curnode2) {
				//Connect each input to each hidden
				newgene=new Gene(newtrait,0, (*curnode2), (*curnode1),count,0);

				//Add the gene to the genome
				genes.push_back(newgene);	 

				count++;

			}
		}

		//Connect the bias to all outputs
		for(curnode1=outputs.begin();curnode1!=outputs.end();++curnode1) {
			newgene=new Gene(newtrait,0, bias, (*curnode1),count,0);

			//Add the gene to the genome
			genes.push_back(newgene);	 

			count++;
		}

		//Recurrently connect the hidden nodes
		for(curnode1=hidden.begin();curnode1!=hidden.end();++curnode1) {
			//Loop Over all Hidden
			for(curnode2=hidden.begin();curnode2!=hidden.end();++curnode2) {
				//Connect each hidden to each hidden
				newgene=new Gene(newtrait,0, (*curnode2), (*curnode1),count,0);

				//Add the gene to the genome
				genes.push_back(newgene);	 

				count++;

			}

		}

	}//end type 2

}
*/
Genome* Genome::new_Genome_load(char *filename) {
	Genome *newgenome;

	int id;

	//char curline[1024];
	char curword[20];  //max word size of 20 characters
	//char delimiters[] = " \n";
	//int curwordnum = 0;

	std::ifstream iFile(filename);

	//Make sure it worked
	//if (!iFile) {
	//	cerr<<"Can't open "<<filename<<" for input"<<endl;
	//	return 0;
	//}

	iFile>>curword;
	//iFile.getline(curline, sizeof(curline));
	//strcpy(curword, NEAT::getUnit(curline, curwordnum++, delimiters));

	//Bypass initial comment
	if (strcmp(curword,"/*")==0) {
		//strcpy(curword, NEAT::getUnit(curline, curwordnum++, delimiters));
		iFile>>curword;
		while (strcmp(curword,"*/")!=0) {
			printf("%s ",curword);
			//strcpy(curword, NEAT::getUnit(curline, curwordnum++, delimiters));
			iFile>>curword;
		}

		//cout<<endl;
		iFile>>curword;
		//strcpy(curword, NEAT::getUnit(curline, curwordnum++, delimiters));
	}

	//strcpy(curword, NEAT::getUnit(curline, curwordnum++, delimiters));
	//id = atoi(curword);
	iFile>>id;

	newgenome=new Genome(id,iFile);

	iFile.close();

	return newgenome;
}

Genome::~Genome() {
	
	std::vector<NNode*>::iterator curnode;
	std::vector<Gene*>::iterator curgene;

	for(curnode=nodes.begin();curnode!=nodes.end();++curnode) {
		delete (*curnode);
	}

	for(curgene=genes.begin();curgene!=genes.end();++curgene) {
		delete (*curgene);
	}

}

Network *Genome::genesis(int id) {
	std::vector<NNode*>::iterator curnode; 
	std::vector<Gene*>::iterator curgene;
	NNode *newnode;

	Link *curlink;
	Link *newlink;

	double maxweight=0.0; //Compute the maximum weight for adaptation purposes
	double weight_mag; //Measures absolute value of weights

	//Inputs and outputs will be collected here for the network
	//All nodes are collected in an all_list- 
	//this will be used for later safe destruction of the net
	std::vector<NNode*> places;
	std::vector<NNode*> transitions;
	std::vector<NNode*> all_list;

	//Gene translation variables
	NNode *inode;
	NNode *onode;

	//The new network
	Network *newnet;

	//Create the nodes
	for(curnode=nodes.begin();curnode!=nodes.end();++curnode) {
		newnode=new NNode((*curnode)->type,(*curnode)->node_id);


		//Check for input or output designation of node
		/*if (((*curnode)->gen_node_label)==INPUT) 
			inlist.push_back(newnode);
		if (((*curnode)->gen_node_label)==BIAS) 
			inlist.push_back(newnode);
		if (((*curnode)->gen_node_label)==OUTPUT)
			outlist.push_back(newnode);*/
		
		// Code for PetriNeat, input and output nodes
		if(((*curnode)->type)==PLACE)
			places.push_back(newnode);
		else
			transitions.push_back(newnode);
		

		//Keep track of all nodes, not just input and output
		all_list.push_back(newnode);

		//Have the node specifier point to the node it generated
		(*curnode)->analogue=newnode;

	}

	//Create the links by iterating through the genes
	for(curgene=genes.begin();curgene!=genes.end();++curgene) {
		//Only create the link if the gene is enabled
		if (((*curgene)->enable)==true) {
			curlink=(*curgene)->lnk;
			inode=(curlink->in_node)->analogue;
			onode=(curlink->out_node)->analogue;
			//NOTE: This line could be run through a recurrency check if desired
			// (no need to in the current implementation of NEAT)
			newlink=new Link(curlink->weight,inode,onode);


			// Redundant, this is already in the link constructor
			//(onode->incoming).push_back(newlink);
			//(inode->outgoing).push_back(newlink);

			

			//Keep track of maximum weight
			if (newlink->weight>0)
				weight_mag=newlink->weight;
			else weight_mag=-newlink->weight;
			if (weight_mag>maxweight)
				maxweight=weight_mag;
		}
	}

	//Create the new network
	newnet=new Network(places,transitions,all_list,id);

	//Attach genotype and phenotype together
	newnet->genotype=this;
	phenotype=newnet;

	newnet->maxweight=maxweight;

	return newnet;

}

bool Genome::verify() {
	std::vector<NNode*>::iterator curnode;
	std::vector<Gene*>::iterator curgene;
	std::vector<Gene*>::iterator curgene2;
	NNode *inode;
	NNode *onode;

	bool disab;

	int last_id;

	//int pause;

	//cout<<"Verifying Genome id: "<<this->genome_id<<endl;

	if (this==0) {
		//cout<<"ERROR GENOME EMPTY"<<endl;
		//cin>>pause;
	}

	//Check each gene's nodes
	for(curgene=genes.begin();curgene!=genes.end();++curgene) {
		inode=((*curgene)->lnk)->in_node;
		onode=((*curgene)->lnk)->out_node;

		//Look for inode
		curnode=nodes.begin();
		while((curnode!=nodes.end())&&
			((*curnode)!=inode))
			++curnode;

		if (curnode==nodes.end()) {
			//cout<<"MISSING iNODE FROM GENE NOT IN NODES OF GENOME!!"<<endl;
			//cin>>pause;
			return false;
		}

		//Look for onode
		curnode=nodes.begin();
		while((curnode!=nodes.end())&&
			((*curnode)!=onode))
			++curnode;

		if (curnode==nodes.end()) {
			//cout<<"MISSING oNODE FROM GENE NOT IN NODES OF GENOME!!"<<endl;
			//cin>>pause;
			return false;
		}

	}

	//Check for NNodes being out of order
	last_id=0;
	for(curnode=nodes.begin();curnode!=nodes.end();++curnode) {
		if ((*curnode)->node_id<last_id) {
			//cout<<"ALERT: NODES OUT OF ORDER in "<<this<<endl;
			//cin>>pause;
			return false;
		}

		last_id=(*curnode)->node_id;
	}


	//Make sure there are no duplicate genes
	for(curgene=genes.begin();curgene!=genes.end();++curgene) {

		for(curgene2=genes.begin();curgene2!=genes.end();++curgene2) {
			if (((*curgene)!=(*curgene2))&&
				((((((*curgene2)->lnk)->in_node)->node_id)==((((*curgene)->lnk)->in_node)->node_id))&&
				(((((*curgene2)->lnk)->out_node)->node_id)==((((*curgene)->lnk)->out_node)->node_id)))) {
					//cout<<"ALERT: DUPLICATE GENES: "<<(*curgene)<<(*curgene2)<<endl;
					//cout<<"INSIDE GENOME: "<<this<<endl;

					//cin>>pause;
				}


		}
	}

	//See if a gene is not disabled properly
	//Note this check does not necessary mean anything is wrong
	//
	//if (nodes.size()>=15) {
	//disab=false;
	////Go through genes and see if one is disabled
	//for(curgene=genes.begin();curgene!=genes.end();++curgene) {
	//if (((*curgene)->enable)==false) disab=true;
	//}

	//if (disab==false) {
	//cout<<"ALERT: NO DISABLED GENE IN GENOME: "<<this<<endl;
	////cin>>pause;
	//}

	//}
	//

	//Check for 2 disables in a row
	//Note:  Again, this is not necessarily a bad sign
	if (nodes.size()>=500) {
		disab=false;
		for(curgene=genes.begin();curgene!=genes.end();++curgene) {
			if ((((*curgene)->enable)==false)&&(disab==true)) {
				//cout<<"ALERT: 2 DISABLES IN A ROW: "<<this<<endl;
			}
			if (((*curgene)->enable)==false) disab=true;
			else disab=false;
		}
	}

	//cout<<"GENOME OK!"<<endl;

	return true;
}


//Print the genome to a file
void Genome::print_to_file(std::ofstream &outFile) {
  std::vector<NNode*>::iterator curnode;
  std::vector<Gene*>::iterator curgene;

  outFile<<"genomestart "<<genome_id<<std::endl;

  //Output the nodes
  for(curnode=nodes.begin();curnode!=nodes.end();++curnode) {
    (*curnode)->print_to_file(outFile);
  }

  //Output the genes
  for(curgene=genes.begin();curgene!=genes.end();++curgene) {
    (*curgene)->print_to_file(outFile);
  }

  outFile<<"genomeend "<<genome_id<<std::endl;

}


void Genome::print_to_file(std::ostream &outFile) {
	std::vector<NNode*>::iterator curnode;
	std::vector<Gene*>::iterator curgene;

	//char tempbuf[128];
	//sprintf(tempbuf, "genomestart %d\n", genome_id);
	//outFile.write(strlen(tempbuf), tempbuf);
    outFile<<"genomestart "<<genome_id<<std::endl;

	//Output the nodes
	for(curnode=nodes.begin();curnode!=nodes.end();++curnode) {
		(*curnode)->print_to_file(outFile);
	}

	//Output the genes
	for(curgene=genes.begin();curgene!=genes.end();++curgene) {
		(*curgene)->print_to_file(outFile);
	}

	//char tempbuf2[128];
	//sprintf(tempbuf2, sizeof(tempbuf2), "genomeend %d\n", genome_id);
	//outFile.write(strlen(tempbuf2), tempbuf2);
    outFile << "genomeend " << genome_id << std::endl << std::endl << std::endl;
	//char tempbuf4[1024];
	//sprintf(tempbuf4, sizeof(tempbuf4), "\n\n");
	//outFile.write(strlen(tempbuf4), tempbuf4);
}

void Genome::print_to_filename(char *filename) {
	std::ofstream oFile(filename);
	//oFile.open(filename, std::ostream::Write);
	print_to_file(oFile);
	oFile.close();
}

int Genome::get_last_node_id() {
	return ((*(nodes.end() - 1))->node_id)+1;
}

double Genome::get_last_gene_innovnum() {
	return ((*(genes.end() - 1))->innovation_num)+1;
}

Genome *Genome::duplicate(int new_id) {
	//Collections for the new Genome
	
	std::vector<NNode*> nodes_dup;
	std::vector<Gene*> genes_dup;

	//Iterators for the old Genome
	std::vector<NNode*>::iterator curnode;
	std::vector<Gene*>::iterator curgene;

	//New item pointers
	NNode *newnode;
	Gene *newgene;

	NNode *inode; //For forming a gene 
	NNode *onode; //For forming a gene

	Genome *newgenome;

	//verify();


	//Duplicate NNodes
	for(curnode=nodes.begin();curnode!=nodes.end();++curnode) {

		newnode=new NNode(*curnode);

		(*curnode)->dup=newnode;  //Remember this node's old copy
		//    (*curnode)->activation_count=55;
		nodes_dup.push_back(newnode);    
	}

	//Duplicate Genes
	for(curgene=genes.begin();curgene!=genes.end();++curgene) {
		//First find the nodes connected by the gene's link

		inode=(((*curgene)->lnk)->in_node)->dup;
		onode=(((*curgene)->lnk)->out_node)->dup;

		newgene=new Gene(*curgene,inode,onode);
		genes_dup.push_back(newgene);

	}

	//Finally, return the genome
	newgenome=new Genome(new_id,nodes_dup,genes_dup);

	return newgenome;

}

void Genome::mutate_random_trait() {

	//TRACK INNOVATION? (future possibility)

}

void Genome::mutate_link_trait(int times) {
	int genenum;
	std::vector<Gene*>::iterator thegene;     //Link to be mutated

	int count;
	int loop;

	for(loop=1;loop<=times;loop++) {

		//Choose a random linknum
		genenum=randint(0,genes.size()-1);

		//set the link to point to the new trait
		thegene=genes.begin();
		for(count=0;count<genenum;count++)
			++thegene;

		//Do not alter frozen genes
		if (!((*thegene)->frozen)) {

			if (randfloat() > 0.5)
				((*thegene)->lnk)->weight++;
			else
				((*thegene)->lnk)->weight--;
		}

	}
}

void Genome::mutate_node_trait(int times) {
	int nodenum;
	std::vector<NNode*>::iterator thenode;     //Link to be mutated
	std::vector<Gene*>::iterator thegene;  //Gene to record innovation
	int count;
	int loop;
	float r;
	for(loop=1;loop<=times;loop++) {

		//Choose a random nodenum
		nodenum=randint(0,nodes.size()-1);

		//set the link to point to the new trait
		thenode=nodes.begin();
		for(count=0;count<nodenum;count++)
			++thenode;

		//Do not mutate frozen nodes
		if (!((*thenode)->frozen)) {

			// If place node mutate the token count
			if ((*thenode)->type == PLACE){

				if (randfloat() > 0.5)
					(*thenode)->tok_count++;
				else
					(*thenode)->tok_count--;

				if((*thenode)->tok_count < 0)
					(*thenode)->tok_count = 0;
				else if((*thenode)->tok_count > 25)
					(*thenode)->tok_count = 25;


			} else{ // Mutate actions for transitions

				r = randfloat();
				
				if (r > 0.75)
					(*thenode)->action_ID = (*thenode)->action_ID ^ 3; // flip all bits
				else if (r > 0.5)
					(*thenode)->action_ID = (*thenode)->action_ID ^ 2; // flip the first bit
				else if (r > 0.25)
					(*thenode)->action_ID = (*thenode)->action_ID ^ 1; // flip the second bit

			}

		}
	}
}


void Genome::mutate_link_weights(double power,double rate,mutator mut_type) {
	std::vector<Gene*>::iterator curgene;
	double num;  //counts gene placement
	double randnum;


	//Go through all the Genes and perturb their link's weights
	num=0.0;
	
	//Loop on all genes  (ORIGINAL METHOD)
	for(curgene=genes.begin();curgene!=genes.end();curgene++) {


		//Don't mutate weights of frozen links
		if (!((*curgene)->frozen)) {

			randnum=randposneg()*randfloat()*2.0;
			
			((*curgene)->lnk)->weight+= (int) randnum;

			//Cap the weights at 8.0 (experimental)
			if (((*curgene)->lnk)->weight > 5.0) ((*curgene)->lnk)->weight = 5.0;
			else if (((*curgene)->lnk)->weight < 1.0) ((*curgene)->lnk)->weight = 1.0;

			//Record the innovation
			//(*curgene)->mutation_num+=randnum;
			(*curgene)->mutation_num=((*curgene)->lnk)->weight;

			num+=1.0;

		}

	} //end for loop


}

void Genome::mutate_toggle_enable(int times) {
	int genenum;
	int count;
	std::vector<Gene*>::iterator thegene;  //Gene to toggle
	std::vector<Gene*>::iterator checkgene;  //Gene to check
	int genecount;

	for (count=1;count<=times;count++) {

		//Choose a random genenum
		genenum=randint(0,genes.size()-1);

		//find the gene
		thegene=genes.begin();
		for(genecount=0;genecount<genenum;genecount++)
			++thegene;

		//Toggle the enable on this gene
		if (((*thegene)->enable)==true) {
			//We need to make sure that another gene connects out of the in-node
			//Because if not a section of network will break off and become isolated
			checkgene=genes.begin();
			while((checkgene!=genes.end())&&
				(((((*checkgene)->lnk)->in_node)!=(((*thegene)->lnk)->in_node))||
				(((*checkgene)->enable)==false)||
				((*checkgene)->innovation_num==(*thegene)->innovation_num)))
				++checkgene;

			//Disable the gene if it's safe to do so
			if (checkgene!=genes.end())
				(*thegene)->enable=false;
		}
		else (*thegene)->enable=true;
	}
}

void Genome::mutate_gene_reenable() {
	std::vector<Gene*>::iterator thegene;  //Gene to enable

	thegene=genes.begin();

	//Search for a disabled gene
	while((thegene!=genes.end())&&((*thegene)->enable==true))
		++thegene;

	//Reenable it
	if (thegene!=genes.end())
		if (((*thegene)->enable)==false) (*thegene)->enable=true;

}

bool Genome::mutate_add_link(std::vector<Innovation*> &innovs,double &curinnov,int tries) {

	int nodenum1,nodenum2;  //Random node numbers
	std::vector<NNode*>::iterator thenode1,thenode2;  //Random node iterators
	int nodecount;  //Counter for finding nodes
	int trycount; //Iterates over attempts to find an unconnected pair of nodes
	NNode *nodep1; //Pointers to the nodes
	NNode *nodep2; //Pointers to the nodes
	std::vector<Gene*>::iterator thegene; //Searches for existing link
	bool found=false;  //Tells whether an open pair was found
	std::vector<Innovation*>::iterator theinnov; //For finding a historical match
	Gene *newgene;  //The new Gene

	double newweight;  //The new weight for the new link

	bool done;
	

	//These are used to avoid getting stuck in an infinite loop checking
	//for recursion
	//Note that we check for recursion to control the frequency of
	//adding recurrent links rather than to prevent any paricular
	//kind of error
	//int thresh=(nodes.size())*(nodes.size());
	//int count=0;

	//Make attempts to find an unconnected pair
	trycount=0;

	//Loop to find a valid link
	while(trycount<tries) {

		//Choose random nodenums
		nodenum1=randint(0,nodes.size()-1);
		nodenum2=randint(0,nodes.size()-1);

		//Find the first node
		thenode1=nodes.begin();
		for(nodecount=0;nodecount<nodenum1;nodecount++)
			++thenode1;

		//Find the second node
		thenode2=nodes.begin();
		for(nodecount=0;nodecount<nodenum2;nodecount++)
			++thenode2;

		nodep1=(*thenode1);
		nodep2=(*thenode2);

		//See if a link already exists  ALSO STOP AT END OF GENES!!!!
		thegene=genes.begin();
		while ((thegene!=genes.end()) && 
			(!((((*thegene)->lnk)->in_node==nodep1)&&
			(((*thegene)->lnk)->out_node==nodep2)))) {
				++thegene;
			}

			if (thegene!=genes.end())
				trycount++;
			else if (nodep1->type == PLACE && nodep2->type == PLACE)
				trycount++;
			else if (nodep1->type == TRANSITION && nodep2->type == TRANSITION)
				trycount++;
			else {

				//count=0;
				trycount=tries;
				found=true;
		
			}

	} //End of normal link finding loop


	//Continue only if an open link was found
	if (found) {

		//Check to see if this innovation already occured in the population
		theinnov=innovs.begin();

		done=false;

		while(!done) {

			//The innovation is totally novel
			if (theinnov==innovs.end()) {

				//If the phenotype does not exist, exit on false,print error
				//Note: This should never happen- if it does there is a bug
				if (phenotype==0) {
					//cout<<"ERROR: Attempt to add link to genome with no phenotype"<<std::endl;
					return false;
				}


				//Choose the new weight
				//newweight=(gaussrand())/1.5;  //Could use a gaussian
				newweight = (int)(randfloat()*3.0); //used to be 10.0

				//Create the new gene
				newgene=new Gene(newweight,nodep1,nodep2,curinnov,newweight);

				//Add the innovation
				innovs.push_back(new Innovation(nodep1->node_id,nodep2->node_id,curinnov,newweight));

				curinnov=curinnov+1.0;

				done=true;
			}
			//OTHERWISE, match the innovation in the innovs list
			else if (((*theinnov)->innovation_type==NEWLINK)&&
				((*theinnov)->node_in_id==(nodep1->node_id))&&
				((*theinnov)->node_out_id==(nodep2->node_id))) {


					//Create new gene
					newgene=new Gene((*theinnov)->new_weight,nodep1,nodep2,(*theinnov)->innovation_num1,0);

					done=true;

				}
			else {
				//Keep looking for a matching innovation from this generation
				++theinnov;
			}
		}

		//Now add the new Genes to the Genome
		//genes.push_back(newgene);  //Old way - could result in out-of-order innovation numbers in rtNEAT
		add_gene(genes,newgene);  //Adds the gene in correct order


		return true;
	}
	else {
		return false;
	}

}

bool Genome::mutate_add_node(std::vector<Innovation*> &innovs,int &curnode_id, double &curinnov,int tries) {

        int nodenum1,nodenum2;  //Random node numbers
        std::vector<NNode*>::iterator thenode1,thenode2;  //Random node iterators
        int nodecount;  //Counter for finding nodes
        int trycount; //Iterates over attempts to find an unconnected pair of nodes
        NNode *nodep1; //Pointers to the nodes
        NNode *nodep2; //Pointers to the nodes
		NNode *newnode; // Pointer to the new node
        std::vector<Gene*>::iterator thegene; //Searches for existing link
        bool found=false;  //Tells whether an open pair was found
        std::vector<Innovation*>::iterator theinnov; //For finding a historical match
        Gene *newgene1;  //The new Gene
		Gene *newgene2;
        double newweight1, newweight2;  //The new weight for the new link

        bool done;
        
		//Make attempts to find an unconnected pair
        trycount=0;

        //Loop to find a valid link
        while(trycount<tries) {
    		float r = randfloat();

    		// A threshold that will tell wether the network is more likely to expand
    		// or to have a high number of connections
    		// A lower number gives a preference to more connections, a higher one
    		// will give a preference to add a place and transition off of a node
    		float sizeConnectionTradeoff = 0.1;

    		// Check the description of sizeConnectionTradeoff
    		if(r < sizeConnectionTradeoff) {
    			// Add a place and a transition from a random node

    			// TODO If this is chosen we need to add the corresponding nodes and genes
    			// Haven't been implemented yet cause I'm not sure what to do with the innovation


    			trycount = tries;
    			found = true;
    		}
    		else {
    			// Look for a pair of nodes of the same type without a node between them
    			// Add a new node between them and the respective links

                //Choose random nodenums
                nodenum1=randint(0,nodes.size()-1);
                nodenum2=randint(0,nodes.size()-1);

                //Find the first node
                thenode1=nodes.begin();
                for(nodecount=0;nodecount<nodenum1;nodecount++)
                        ++thenode1;

                //Find the second node
                thenode2=nodes.begin();
                for(nodecount=0;nodecount<nodenum2;nodecount++)
                       ++thenode2;

                nodep1=(*thenode1);
                nodep2=(*thenode2);

                // See if the nodes are of the same type, otherwise retry
				if (nodep1->type == PLACE && nodep2->type == TRANSITION)
                    trycount++;
                else if (nodep1->type == TRANSITION && nodep2->type == PLACE)
                    trycount++;
                else {
                	// If both nodes are of the same type, see if there's a node between them
                	std::vector<NNode*>::iterator thenode3;
                	NNode *cNode;
                	bool connectingNodeFound = false;
                	
                	//thenode3=nodes.begin();
                	for(int connectionNode = 0; connectionNode < nodes.size(); connectionNode++) {

	                //for(; !connectingNodeFound && thenode3 != nodes.end(); ++thenode3) {
	                	cNode = nodes[connectionNode];
	                	// For every incoming and outgoing link combination, see if the nodes match the ones we chose
	                	std::vector<NNode*>::iterator inlink, outlink;

	                	//inlink = cNode->incoming.begin();
	                	for(int inLinks = 0; inLinks < cNode->incoming.size(); inLinks++) {
	                	//for(; !connectingNodeFound && inlink != cNode->incoming.end(); ++inlink) {
	                		//outlink = cNode->outgoing.begin();
	                		for(int outLinks = 0; outLinks < cNode->outgoing.size(); outLinks++) {
	                		//for(; !connectingNodeFound && outlink != cNode->outgoing.end(); ++outlink) {
	                			// Check for either node being the in/out node
	                			// Incoming node to compare cNode->incoming[inLinks]->in_node
	                			// Outgoing node to compare cNode->outgoing[outLinks]->out_node

	                			if((cNode->incoming[inLinks]->in_node == nodep1 && cNode->outgoing[outLinks]->out_node == nodep2)
	                				|| (cNode->incoming[inLinks]->in_node == nodep2 && cNode->outgoing[outLinks]->out_node == nodep1)) {

	                			//if(((*inlink)->in_node == nodep1 && (*outlink)->out_node == nodep2)
	                			//	|| ((*inlink)->in_node == nodep2 && (*outlink)->out_node == nodep1)) {
	                				connectingNodeFound = true;
	                			}
	                		} // Finished comparing the incoming link with all outgoing links

	                	} // Finished checking all the incoming links for the node
	                	
	                } // Finished looking for connection nodes

	                // If a node connecting the nodes was found, then retry
	                if(connectingNodeFound) {
	                	trycount++;
	                }
	                // Otherwise, a pair of nodes was found
	                else {
	                	trycount = tries;
	                	found = true;
	                }
                }
    		}
        } //End of normal node finding loop


        //Continue only if an open link was found
        if (found) {

            //Check to see if this innovation already occured in the population
            theinnov=innovs.begin();

            done=false;

            while(!done) {

                //The innovation is totally novel
                if (theinnov==innovs.end()) {

                    //If the phenotype does not exist, exit on false,print error
                    //Note: This should never happen- if it does there is a bug
                    if (phenotype==0) {
                        //cout<<"ERROR: Attempt to add link to genome with no phenotype"<<std::endl;
                        return false;
					}

					//Create the new NNode
	                //By convention, it will point to the first trait
                    if(nodep1->type == PLACE)
						newnode=new NNode(TRANSITION,curnode_id++);
         			else
						newnode = new NNode(PLACE, curnode_id++);   	 

                    //Choose the new weight
                    newweight1 = (int)(randfloat()*3.0);
					newweight2 = (int)(randfloat()*3.0);
	
                    //Create the new gene
                    newgene1 =new Gene(newweight1,nodep1,newnode,curinnov,newweight1);		// Why is the other weight being asigned as the mutation number?
					newgene2 = new Gene(newweight2,newnode,nodep2,curinnov,newweight2);		// Why is the other weight being asigned as the mutation number?

                    //Add the innovation
                    innovs.push_back(new Innovation(nodep1->node_id,nodep2->node_id,curinnov,curinnov+1.0,newnode->node_id));

                    curinnov=curinnov+2.0;

                    done=true;
                }
                //OTHERWISE, match the innovation in the innovs list
                else if (((*theinnov)->innovation_type==NEWNODE)&&
                        ((*theinnov)->node_in_id==(nodep1->node_id))&&
                        ((*theinnov)->node_out_id==(nodep2->node_id))) {

                                
                            //Create new gene
                            newgene1=new Gene((*theinnov)->new_weight,nodep1,newnode,(*theinnov)->innovation_num1,0);
                            newgene2 = new Gene((*theinnov)->new_weight,newnode,nodep2,(*theinnov)->innovation_num2,0);
							done=true;

                }
                else {
                        //Keep looking for a matching innovation from this generation
                        ++theinnov;
                }
            }

            //Now add the new Genes to the Genome
            //genes.push_back(newgene);  //Old way - could result in out-of-order innovation numbers in rtNEAT
            add_gene(genes,newgene1);  //Adds the gene in correct order
			add_gene(genes,newgene2);
			node_insert(nodes, newnode);

            return true;
        }
        else {
            return false;
        }
}

//Adds a new gene that has been created through a mutation in the
//*correct order* into the list of genes in the genome
void Genome::add_gene(std::vector<Gene*> &glist,Gene *g) {
  std::vector<Gene*>::iterator curgene;
  double p1innov;

  double inum=g->innovation_num;

  //std::cout<<"**ADDING GENE: "<<g->innovation_num<<std::endl;

  curgene=glist.begin();
  while ((curgene!=glist.end())&&
	 (((*curgene)->innovation_num)<inum)) {
    //p1innov=(*curgene)->innovation_num;
    //printf("Innov num: %f",p1innov);  
    ++curgene;

    //Con::printf("looking gene %f", (*curgene)->innovation_num);
  }


  glist.insert(curgene,g);

}


void Genome::node_insert(std::vector<NNode*> &nlist,NNode *n) {
	std::vector<NNode*>::iterator curnode;

	int id=n->node_id;

	curnode=nlist.begin();
	while ((curnode!=nlist.end())&&
		(((*curnode)->node_id)<id)) 
		++curnode;

	nlist.insert(curnode,n);

}

Genome *Genome::mate_multipoint(Genome *g,int genomeid,double fitness1,double fitness2, bool interspec_flag) { 
	std::vector<NNode*> newnodes;   
	std::vector<Gene*> newgenes;    
	Genome *new_genome;

	std::vector<Gene*>::iterator curgene2;  //Checks for link duplication


	//iterators for moving through the two parents' genes
	std::vector<Gene*>::iterator p1gene;
	std::vector<Gene*>::iterator p2gene;
	double p1innov;  //Innovation numbers for genes inside parents' Genomes
	double p2innov;
	Gene *chosengene;  //Gene chosen for baby to inherit
//	int traitnum;  //Number of trait new gene points to
	NNode *inode;  //NNodes connected to the chosen Gene
	NNode *onode;
	NNode *new_inode;
	NNode *new_onode;
	std::vector<NNode*>::iterator curnode;  //For checking if NNodes exist already 


	bool disable;  //Set to true if we want to disabled a chosen gene

	disable=false;
	Gene *newgene;

	bool p1better; //Tells if the first genome (this one) has better fitness or not

	bool skip;


	//Figure out which genome is better
	//The worse genome should not be allowed to add extra structural baggage
	//If they are the same, use the smaller one's disjoint and excess genes only
	if (fitness1>fitness2) 
		p1better=true;
	else if (fitness1==fitness2) {
		if (genes.size()<(g->genes.size()))
			p1better=true;
		else p1better=false;
	}
	else 
		p1better=false;

	//NEW 3/17/03 Make sure all sensors and outputs are included
	for(curnode=(g->nodes).begin();curnode!=(g->nodes).end();++curnode) {
		if ((((*curnode)->gen_node_label)==INPUT)||
			(((*curnode)->gen_node_label)==BIAS)||
			(((*curnode)->gen_node_label)==OUTPUT)) {
				
				//Create a new node off the sensor or output
				new_onode=new NNode((*curnode));

				//Add the new node
				node_insert(newnodes,new_onode);

			}

	}


	//Now move through the Genes of each parent until both genomes end
	p1gene=genes.begin();
	p2gene=(g->genes).begin();
	while(!((p1gene==genes.end())&&
		(p2gene==(g->genes).end()))) {


			skip=false;  //Default to not skipping a chosen gene

			if (p1gene==genes.end()) {
				chosengene=*p2gene;
				++p2gene;
				if (p1better) skip=true;  //Skip excess from the worse genome
			}
			else if (p2gene==(g->genes).end()) {
				chosengene=*p1gene;
				++p1gene;
				if (!p1better) skip=true; //Skip excess from the worse genome
			}
			else {
				//Extract current innovation numbers
				p1innov=(*p1gene)->innovation_num;
				p2innov=(*p2gene)->innovation_num;

				if (p1innov==p2innov) {
					if (randfloat()<0.5) {
						chosengene=*p1gene;
					}
					else {
						chosengene=*p2gene;
					}

					//If one is disabled, the corresponding gene in the offspring
					//will likely be disabled
					if ((((*p1gene)->enable)==false)||
						(((*p2gene)->enable)==false)) 
						if (randfloat()<0.75) disable=true;

					++p1gene;
					++p2gene;

				}
				else if (p1innov<p2innov) {
					chosengene=*p1gene;
					++p1gene;

					if (!p1better) skip=true;

				}
				else if (p2innov<p1innov) {
					chosengene=*p2gene;
					++p2gene;
					if (p1better) skip=true;
				}
			}

			/*
			//Uncomment this line to let growth go faster (from both parents excesses)
			skip=false;

			//For interspecies mating, allow all genes through:
			if (interspec_flag)
				skip=false;
			*/

			//Check to see if the chosengene conflicts with an already chosen gene
			//i.e. do they represent the same link    
			curgene2=newgenes.begin();
			while ((curgene2!=newgenes.end())&&
				(!((((((*curgene2)->lnk)->in_node)->node_id)==((((chosengene)->lnk)->in_node)->node_id))&&
				(((((*curgene2)->lnk)->out_node)->node_id)==((((chosengene)->lnk)->out_node)->node_id))   ))&&
				(!((((((*curgene2)->lnk)->in_node)->node_id)==((((chosengene)->lnk)->out_node)->node_id))&&
				(((((*curgene2)->lnk)->out_node)->node_id)==((((chosengene)->lnk)->in_node)->node_id))  )))
			{	
				++curgene2;
			}

			if (curgene2!=newgenes.end()) skip=true;  //Links conflicts, abort adding

			if (!skip) {

				//Now add the chosengene to the baby

				//Next check for the nodes, add them if not in the baby Genome already
				inode=(chosengene->lnk)->in_node;
				onode=(chosengene->lnk)->out_node;

				//Check for inode in the newnodes list
				if (inode->node_id<onode->node_id) {
					//inode before onode

					//Checking for inode's existence
					curnode=newnodes.begin();
					while((curnode!=newnodes.end())&&
						((*curnode)->node_id!=inode->node_id)) 
						++curnode;

					if (curnode==newnodes.end()) {
						//Here we know the node doesn't exist so we have to add it
						//(normalized trait number for new NNode)

						//old buggy version:
						// if (!(onode->nodetrait)) nodetraitnum=((*(traits.begin()))->trait_id);

						new_inode=new NNode(inode);
						node_insert(newnodes,new_inode);

					}
					else {
						new_inode=(*curnode);

					}

					//Checking for onode's existence
					curnode=newnodes.begin();
					while((curnode!=newnodes.end())&&
						((*curnode)->node_id!=onode->node_id)) 
						++curnode;
					if (curnode==newnodes.end()) {
						//Here we know the node doesn't exist so we have to add it
						//normalized trait number for new NNode

						new_onode=new NNode(onode);

						node_insert(newnodes,new_onode);

					}
					else {
						new_onode=(*curnode);
					}

				}
				//If the onode has a higher id than the inode we want to add it first
				else {
					//Checking for onode's existence
					curnode=newnodes.begin();
					while((curnode!=newnodes.end())&&
						((*curnode)->node_id!=onode->node_id)) 
						++curnode;
					if (curnode==newnodes.end()) {
						//Here we know the node doesn't exist so we have to add it
						//normalized trait number for new NNode

						new_onode=new NNode(onode);
						//newnodes.push_back(new_onode);
						node_insert(newnodes,new_onode);

					}
					else {
						new_onode=(*curnode);

					}

					//Checking for inode's existence
					curnode=newnodes.begin();
					while((curnode!=newnodes.end())&&
						((*curnode)->node_id!=inode->node_id)) 
						++curnode;
					if (curnode==newnodes.end()) {
						//Here we know the node doesn't exist so we have to add it
						//normalized trait number for new NNode

						new_inode=new NNode(inode);

						node_insert(newnodes,new_inode);

					}
					else {
						new_inode=(*curnode);

					}

				} //End NNode checking section- NNodes are now in new Genome

				//Add the Gene
				newgene=new Gene(chosengene,new_inode,new_onode);
				if (disable) {
					newgene->enable=false;
					disable=false;
				}
				newgenes.push_back(newgene);
			}

		}

		new_genome=new Genome(genomeid,newnodes,newgenes);

		//Return the baby Genome
		return (new_genome);

}

Genome *Genome::mate_multipoint_avg(Genome *g,int genomeid,double fitness1,double fitness2,bool interspec_flag) {
	//The baby Genome will contain these new Traits, NNodes, and Genes
	std::vector<NNode*> newnodes;
	std::vector<Gene*> newgenes;

	std::vector<Gene*>::iterator curgene2; //Checking for link duplication

	//iterators for moving through the two parents' genes
	std::vector<Gene*>::iterator p1gene;
	std::vector<Gene*>::iterator p2gene;
	double p1innov;  //Innovation numbers for genes inside parents' Genomes
	double p2innov;
	Gene *chosengene;  //Gene chosen for baby to inherit
	NNode *inode;  //NNodes connected to the chosen Gene
	NNode *onode;
	NNode *new_inode;
	NNode *new_onode;

	std::vector<NNode*>::iterator curnode;  //For checking if NNodes exist already 

	//This Gene is used to hold the average of the two genes to be averaged
	Gene *avgene;

	Gene *newgene;

	bool skip;

	bool p1better;  //Designate the better genome


	//Set up the avgene
	avgene=new Gene(0,0,0,0,0);

	//NEW 3/17/03 Make sure all sensors and outputs are included
	for(curnode=(g->nodes).begin();curnode!=(g->nodes).end();++curnode) {
		if ((((*curnode)->gen_node_label)==INPUT)||
			(((*curnode)->gen_node_label)==OUTPUT)||
			(((*curnode)->gen_node_label)==BIAS)) {

				//Create a new node off the sensor or output
				new_onode=new NNode((*curnode));

				//Add the new node
				node_insert(newnodes,new_onode);

			}

	}

	//Figure out which genome is better
	//The worse genome should not be allowed to add extra structural baggage
	//If they are the same, use the smaller one's disjoint and excess genes only
	if (fitness1>fitness2) 
		p1better=true;
	else if (fitness1==fitness2) {
		if (genes.size()<(g->genes.size()))
			p1better=true;
		else p1better=false;
	}
	else 
		p1better=false;


	//Now move through the Genes of each parent until both genomes end
	p1gene=genes.begin();
	p2gene=(g->genes).begin();
	while(!((p1gene==genes.end())&&
		(p2gene==(g->genes).end()))) {

			avgene->enable=true;  //Default to enabled

			skip=false;

			if (p1gene==genes.end()) {
				chosengene=*p2gene;
				++p2gene;

				if (p1better) skip=true;

			}
			else if (p2gene==(g->genes).end()) {
				chosengene=*p1gene;
				++p1gene;

				if (!p1better) skip=true;
			}
			else {
				//Extract current innovation numbers
				p1innov=(*p1gene)->innovation_num;
				p2innov=(*p2gene)->innovation_num;

				if (p1innov==p2innov) {
					//Average them into the avgene
					//WEIGHTS AVERAGED HERE
					(avgene->lnk)->weight=(((*p1gene)->lnk)->weight+((*p2gene)->lnk)->weight)/2.0;

				

					////BLX-alpha method (Eschelman et al 1993)
					////Not used in this implementation, but the commented code works
					////with alpha=0.5, this will produce babies evenly in exploitation and exploration space
					////and uniformly distributed throughout
					//blx_alpha=-0.4;
					//w1=(((*p1gene)->lnk)->weight);
					//w2=(((*p2gene)->lnk)->weight);
					//if (w1>w2) {
					//blx_max=w1; blx_min=w2;
					//}
					//else {
					//blx_max=w2; blx_min=w1;
					//}
					//blx_range=blx_max-blx_min;
					//blx_explore=blx_alpha*blx_range;
					////Now extend the range into the exploraton space
					//blx_min-=blx_explore;
					//blx_max+=blx_explore;
					//blx_range=blx_max-blx_min;
					////Set the weight in the new range
					//(avgene->lnk)->weight=blx_min+blx_pos*blx_range;
					//

					if (randfloat()>0.5) (avgene->lnk)->in_node=((*p1gene)->lnk)->in_node;
					else (avgene->lnk)->in_node=((*p2gene)->lnk)->in_node;

					if (randfloat()>0.5) (avgene->lnk)->out_node=((*p1gene)->lnk)->out_node;
					else (avgene->lnk)->out_node=((*p2gene)->lnk)->out_node;

					avgene->innovation_num=(*p1gene)->innovation_num;
					avgene->mutation_num=((*p1gene)->mutation_num+(*p2gene)->mutation_num)/2.0;

					if ((((*p1gene)->enable)==false)||
						(((*p2gene)->enable)==false)) 
						if (randfloat()<0.75) avgene->enable=false;

					chosengene=avgene;
					++p1gene;
					++p2gene;
				}
				else if (p1innov<p2innov) {
					chosengene=*p1gene;
					++p1gene;

					if (!p1better) skip=true;
				}
				else if (p2innov<p1innov) {
					chosengene=*p2gene;
					++p2gene;

					if (p1better) skip=true;
				}
			}

			/*
			//THIS LINE MUST BE DELETED TO SLOW GROWTH
			skip=false;

			//For interspecies mating, allow all genes through:
			if (interspec_flag)
				skip=false;
			*/

			//Check to see if the chosengene conflicts with an already chosen gene
			//i.e. do they represent the same link    
			curgene2=newgenes.begin();
			while ((curgene2!=newgenes.end()))

			{

				if (((((((*curgene2)->lnk)->in_node)->node_id)==((((chosengene)->lnk)->in_node)->node_id))&&
					(((((*curgene2)->lnk)->out_node)->node_id)==((((chosengene)->lnk)->out_node)->node_id)))||
					((((((*curgene2)->lnk)->out_node)->node_id)==((((chosengene)->lnk)->in_node)->node_id))&&
					(((((*curgene2)->lnk)->in_node)->node_id)==((((chosengene)->lnk)->out_node)->node_id))   ))
				{ 
					skip=true;

				}
				++curgene2;
			}

			if (!skip) {

				//Now add the chosengene to the baby

				//Next check for the nodes, add them if not in the baby Genome already
				inode=(chosengene->lnk)->in_node;
				onode=(chosengene->lnk)->out_node;

				//Check for inode in the newnodes list
				if (inode->node_id<onode->node_id) {

					//Checking for inode's existence
					curnode=newnodes.begin();
					while((curnode!=newnodes.end())&&
						((*curnode)->node_id!=inode->node_id)) 
						++curnode;

					if (curnode==newnodes.end()) {
						//Here we know the node doesn't exist so we have to add it
						//normalized trait number for new NNode


						new_inode=new NNode(inode);

						node_insert(newnodes,new_inode);
					}
					else {
						new_inode=(*curnode);

					}

					//Checking for onode's existence
					curnode=newnodes.begin();
					while((curnode!=newnodes.end())&&
						((*curnode)->node_id!=onode->node_id)) 
						++curnode;
					if (curnode==newnodes.end()) {
						//Here we know the node doesn't exist so we have to add it
						//normalized trait number for new NNode

						new_onode=new NNode(onode);

						node_insert(newnodes,new_onode);
					}
					else {
						new_onode=(*curnode);
					}
				}
				//If the onode has a higher id than the inode we want to add it first
				else {
					//Checking for onode's existence
					curnode=newnodes.begin();
					while((curnode!=newnodes.end())&&
						((*curnode)->node_id!=onode->node_id)) 
						++curnode;
					if (curnode==newnodes.end()) {
						//Here we know the node doesn't exist so we have to add it

						new_onode=new NNode(onode);

						node_insert(newnodes,new_onode);
					}
					else {
						new_onode=(*curnode);
					}

					//Checking for inode's existence
					curnode=newnodes.begin();
					while((curnode!=newnodes.end())&&
						((*curnode)->node_id!=inode->node_id)) 
						++curnode;
					if (curnode==newnodes.end()) {
						//Here we know the node doesn't exist so we have to add it
						//normalized trait number for new NNode

						new_inode=new NNode(inode);

						node_insert(newnodes,new_inode);
					}
					else {
						new_inode=(*curnode);

					}

				} //End NNode checking section- NNodes are now in new Genome

				//Add the Gene
				newgene=new Gene(chosengene,new_inode,new_onode);

				newgenes.push_back(newgene);

			}  //End if which checked for link duplicationb

		}

		delete avgene;  //Clean up used object

		//Return the baby Genome
		return (new Genome(genomeid,newnodes,newgenes));

}

Genome *Genome::mate_singlepoint(Genome *g,int genomeid) {
	//The baby Genome will contain these new Traits, NNodes, and Genes 
	std::vector<NNode*> newnodes;   
	std::vector<Gene*> newgenes;    


	std::vector<Gene*>::iterator curgene2;  //Check for link duplication

	//iterators for moving through the two parents' genes
	std::vector<Gene*>::iterator p1gene;
	std::vector<Gene*>::iterator p2gene;
	std::vector<Gene*>::iterator stopper;  //To tell when finished
	std::vector<Gene*>::iterator p2stop;
	std::vector<Gene*>::iterator p1stop;
	double p1innov;  //Innovation numbers for genes inside parents' Genomes
	double p2innov;
	Gene *chosengene;  //Gene chosen for baby to inherit
//	int traitnum;  //Number of trait new gene points to
	NNode *inode;  //NNodes connected to the chosen Gene
	NNode *onode;
	NNode *new_inode;
	NNode *new_onode;
	std::vector<NNode*>::iterator curnode;  //For checking if NNodes exist already 
	

	//This Gene is used to hold the average of the two genes to be averaged
	Gene *avgene;

	int crosspoint; //The point in the Genome to cross at
	int genecounter; //Counts up to the crosspoint
	bool skip; //Used for skipping unwanted genes

	//Set up the avgene
	avgene=new Gene(0,0,0,0,0);

	//Decide where to cross  (p1gene will always be in smaller Genome)
	if (genes.size()<(g->genes).size()) {
		crosspoint=randint(0,(genes.size())-1);
		p1gene=genes.begin();
		p2gene=(g->genes).begin();
		stopper=(g->genes).end();
		p1stop=genes.end();
		p2stop=(g->genes).end();
	}
	else {
		crosspoint=randint(0,((g->genes).size())-1);
		p2gene=genes.begin();
		p1gene=(g->genes).begin();
		stopper=genes.end();
		p1stop=(g->genes.end());
		p2stop=genes.end();
	}

	genecounter=0;  //Ready to count to crosspoint

	skip=false;  //Default to not skip a Gene
	//Note that we skip when we are on the wrong Genome before
	//crossing

	//Now move through the Genes of each parent until both genomes end
	while(p2gene!=stopper) {

		avgene->enable=true;  //Default to true

		if (p1gene==p1stop) {
			chosengene=*p2gene;
			++p2gene;
		}
		else if (p2gene==p2stop) {
			chosengene=*p1gene;
			++p1gene;
		}
		else {
			//Extract current innovation numbers

			//if (p1gene==g->genes.end()) cout<<"WARNING p1"<<std::endl;
			//if (p2gene==g->genes.end()) cout<<"WARNING p2"<<std::endl;

			p1innov=(*p1gene)->innovation_num;
			p2innov=(*p2gene)->innovation_num;

			if (p1innov==p2innov) {

				//Pick the chosengene depending on whether we've crossed yet
				if (genecounter<crosspoint) {
					chosengene=*p1gene;
				}
				else if (genecounter>crosspoint) {
					chosengene=*p2gene;
				}
				//We are at the crosspoint here
				else {


					//WEIGHTS AVERAGED HERE
					(avgene->lnk)->weight=(((*p1gene)->lnk)->weight+((*p2gene)->lnk)->weight)/2.0;


					if (randfloat()>0.5) (avgene->lnk)->in_node=((*p1gene)->lnk)->in_node;
					else (avgene->lnk)->in_node=((*p2gene)->lnk)->in_node;

					if (randfloat()>0.5) (avgene->lnk)->out_node=((*p1gene)->lnk)->out_node;
					else (avgene->lnk)->out_node=((*p2gene)->lnk)->out_node;

					avgene->innovation_num=(*p1gene)->innovation_num;
					avgene->mutation_num=((*p1gene)->mutation_num+(*p2gene)->mutation_num)/2.0;

					if ((((*p1gene)->enable)==false)||
						(((*p2gene)->enable)==false)) 
						avgene->enable=false;

					chosengene=avgene;
				}

				++p1gene;
				++p2gene;
				++genecounter;
			}
			else if (p1innov<p2innov) {
				if (genecounter<crosspoint) {
					chosengene=*p1gene;
					++p1gene;
					++genecounter;
				}
				else {
					chosengene=*p2gene;
					++p2gene;
				}
			}
			else if (p2innov<p1innov) {
				++p2gene;
				skip=true;  //Special case: we need to skip to the next iteration
				//becase this Gene is before the crosspoint on the wrong Genome
			}
		}

		//Check to see if the chosengene conflicts with an already chosen gene
		//i.e. do they represent the same link    
		curgene2=newgenes.begin();

		while ((curgene2!=newgenes.end())&&
			(!((((((*curgene2)->lnk)->in_node)->node_id)==((((chosengene)->lnk)->in_node)->node_id))&&
			(((((*curgene2)->lnk)->out_node)->node_id)==((((chosengene)->lnk)->out_node)->node_id))  ))&&
			(!((((((*curgene2)->lnk)->in_node)->node_id)==((((chosengene)->lnk)->out_node)->node_id))&&
			(((((*curgene2)->lnk)->out_node)->node_id)==((((chosengene)->lnk)->in_node)->node_id))  )))
		{

			++curgene2;
		}


		if (curgene2!=newgenes.end()) skip=true;  //Link is a duplicate

		if (!skip) {
			//Now add the chosengene to the baby


			//Next check for the nodes, add them if not in the baby Genome already
			inode=(chosengene->lnk)->in_node;
			onode=(chosengene->lnk)->out_node;

			//Check for inode in the newnodes list
			if (inode->node_id<onode->node_id) {
				//cout<<"inode before onode"<<std::endl;
				//Checking for inode's existence
				curnode=newnodes.begin();
				while((curnode!=newnodes.end())&&
					((*curnode)->node_id!=inode->node_id)) 
					++curnode;

				if (curnode==newnodes.end()) {
					//Here we know the node doesn't exist so we have to add it
					//normalized trait number for new NNode


					new_inode=new NNode(inode);

					node_insert(newnodes,new_inode);
				}
				else {
					new_inode=(*curnode);
				}

				//Checking for onode's existence
				curnode=newnodes.begin();
				while((curnode!=newnodes.end())&&
					((*curnode)->node_id!=onode->node_id)) 
					++curnode;
				if (curnode==newnodes.end()) {
					//Here we know the node doesn't exist so we have to add it
					//normalized trait number for new NNode


					new_onode=new NNode(onode);
					node_insert(newnodes,new_onode);

				}
				else {
					new_onode=(*curnode);
				}
			}
			//If the onode has a higher id than the inode we want to add it first
			else {
				//Checking for onode's existence
				curnode=newnodes.begin();
				while((curnode!=newnodes.end())&&
					((*curnode)->node_id!=onode->node_id)) 
					++curnode;
				if (curnode==newnodes.end()) {
					//Here we know the node doesn't exist so we have to add it
					//normalized trait number for new NNode

					new_onode=new NNode(onode);
					node_insert(newnodes,new_onode);
				}
				else {
					new_onode=(*curnode);
				}

				//Checking for inode's existence
				curnode=newnodes.begin();

				while((curnode!=newnodes.end())&&
					((*curnode)->node_id!=inode->node_id)) 
					++curnode;
				if (curnode==newnodes.end()) {
					//Here we know the node doesn't exist so we have to add it
					//normalized trait number for new NNode

					new_inode=new NNode(inode);
					//newnodes.push_back(new_inode);
					node_insert(newnodes,new_inode);
				}
				else {
					new_inode=(*curnode);
				}

			} //End NNode checking section- NNodes are now in new Genome

			//Add the Gene
			newgenes.push_back(new Gene(chosengene,new_inode,new_onode));

		}  //End of if (!skip)

		skip=false;

	}


	delete avgene;  //Clean up used object

	//Return the baby Genome
	return (new Genome(genomeid,newnodes,newgenes));

}

double Genome::compatibility(Genome *g) {

	//iterators for moving through the two potential parents' Genes
	std::vector<Gene*>::iterator p1gene;
	std::vector<Gene*>::iterator p2gene;  

	//Innovation numbers
	double p1innov;
	double p2innov;

	//Intermediate value
	double mut_diff;

	//Set up the counters
	double num_disjoint=0.0;
	double num_excess=0.0;
	double mut_diff_total=0.0;
	double num_matching=0.0;  //Used to normalize mutation_num differences

	double max_genome_size; //Size of larger Genome

	//Get the length of the longest Genome for percentage computations
	if (genes.size()<(g->genes).size()) 
		max_genome_size=(g->genes).size();
	else max_genome_size=genes.size();

	//Now move through the Genes of each potential parent 
	//until both Genomes end
	p1gene=genes.begin();
	p2gene=(g->genes).begin();
	while(!((p1gene==genes.end())&&
		(p2gene==(g->genes).end()))) {

			if (p1gene==genes.end()) {
				++p2gene;
				num_excess+=1.0;
			}
			else if (p2gene==(g->genes).end()) {
				++p1gene;
				num_excess+=1.0;
			}
			else {
				//Extract current innovation numbers
				p1innov=(*p1gene)->innovation_num;
				p2innov=(*p2gene)->innovation_num;

				if (p1innov==p2innov) {
					num_matching+=1.0;
					mut_diff=((*p1gene)->mutation_num)-((*p2gene)->mutation_num);
					if (mut_diff<0.0) mut_diff=0.0-mut_diff;
					//mut_diff+=trait_compare((*p1gene)->lnk->linktrait,(*p2gene)->lnk->linktrait); //CONSIDER TRAIT DIFFERENCES
					mut_diff_total+=mut_diff;

					++p1gene;
					++p2gene;
				}
				else if (p1innov<p2innov) {
					++p1gene;
					num_disjoint+=1.0;
				}
				else if (p2innov<p1innov) {
					++p2gene;
					num_disjoint+=1.0;
				}
			}
		} //End while

		//Return the compatibility number using compatibility formula
		//Note that mut_diff_total/num_matching gives the AVERAGE
		//difference between mutation_nums for any two matching Genes
		//in the Genome

		//Normalizing for genome size
		//return (disjoint_coeff*(num_disjoint/max_genome_size)+
		//  excess_coeff*(num_excess/max_genome_size)+
		//  mutdiff_coeff*(mut_diff_total/num_matching));


		//Look at disjointedness and excess in the absolute (ignoring size)

		//cout<<"COMPAT: size = "<<max_genome_size<<" disjoint = "<<num_disjoint<<" excess = "<<num_excess<<" diff = "<<mut_diff_total<<"  TOTAL = "<<(disjoint_coeff*(num_disjoint/1.0)+excess_coeff*(num_excess/1.0)+mutdiff_coeff*(mut_diff_total/num_matching))<<std::endl;

		return (NEAT::disjoint_coeff*(num_disjoint/1.0)+
			NEAT::excess_coeff*(num_excess/1.0)+
			NEAT::mutdiff_coeff*(mut_diff_total/num_matching));
}

int Genome::extrons() {
	std::vector<Gene*>::iterator curgene;
	int total=0;

	for(curgene=genes.begin();curgene!=genes.end();curgene++) {
		if ((*curgene)->enable) ++total;
	}

	return total;
}

//Calls special constructor that creates a Genome of 3 possible types:
//0 - Fully linked, no hidden nodes
//1 - Fully linked, one hidden node splitting each link
//2 - Fully connected with a hidden layer 
//num_hidden is only used in type 2
//Saves to filename argument
Genome* NEAT::new_Genome_auto(int num_in,int num_out,int num_hidden,int type, const char *filename) {
	Genome *g=new Genome(num_in,num_out,num_hidden,type);

	//print_Genome_tofile(g,"auto_genome");
	print_Genome_tofile(g, filename);

	return g;
}


void NEAT::print_Genome_tofile(Genome *g,const char *filename) {

	//ofstream oFile(filename,ios::out);

    std::string file = "nero/data/neat/";
    file += filename;
    //strcpyl(file, 100, "nero/data/neat/", filename, 0);
	std::ofstream oFile(file.c_str());
//	oFile.open(file, std::ostream::Write);

	//Make sure	it worked
	//if (!oFile)	{
	//	cerr<<"Can't open "<<filename<<" for output"<<std::endl;
	//	return 0;
	//}
	g->print_to_file(oFile);

	oFile.close();
}

