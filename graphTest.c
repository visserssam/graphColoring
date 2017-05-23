/* simpel sorting algorithem for C++ that utilizes mpi to sort over multiple processors*/
/* writen by sam vissers*/

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>
#include <iostream>
#include <string>
#include <vector>
using namespace std;

struct node_t{
	int numbOfedges;
	int name;
	int color;
	vector <node_t*> edges;
};

struct map_t{
	vector <node_t> nodes;
};

MPI_Datatype map_type;

void initColor(map_t &graph){
	int i;
	for ( i = 0; i < graph.nodes.size(); i++)
	{
		graph.nodes[i].color = (rand()%4) ;
	}
	
}

int fittness(map_t input){
	/* takes in a graph and determis the fitness by counting the nymber of connected nodes 
	* with the same number, the higher the number the less fit the graph is*/
	int fittnes;
	int outercheck;
	int innercheck;
	fittnes = 0;
	for (outercheck = 0; outercheck < input.nodes.size(); outercheck++)
	{
		for (innercheck = 0; innercheck < input.nodes[outercheck].edges.size(); innercheck++)
		{
			if( input.nodes[outercheck].edges[innercheck]->color == input.nodes[outercheck].color ){
				fittnes++;
			}
		}
	}
	return fittnes;
}

map_t change( map_t input){
	//changes the label of a node whos neighbor is the same color as it to a different color
		int outercheck, innercheck;
	
	for (outercheck = 0; outercheck < input.nodes.size(); outercheck++)
	{
		for (innercheck = 0; innercheck < input.nodes[outercheck].edges.size(); innercheck++)
		{
			if(input.nodes[outercheck].edges[innercheck]->color == input.nodes[outercheck].color ){
				input.nodes[outercheck].color = (rand()%4);
			}
		}
	}
	return input;
}

void aneiling(map_t &input, int temp, float coolingfactor, int freezPoint){
	map_t current = input;
	map_t p;
	int oldFitness = fittness(input);
	map_t best = input;
	int eBest = oldFitness;
	int T = temp;
	while(T > freezPoint){
		p = change(current);
		int Ep = fittness(p);
		int delta = Ep - oldFitness;
		if(delta < 0){
			oldFitness = Ep;
			current = p;
			if(oldFitness < eBest){
				best = current;
				eBest = oldFitness;
			}
		}
		else{
			if((rand()%1) >= (delta / T) ){
				oldFitness = Ep;
				current = p;
			}
		}
		T = T * coolingfactor;
	}

	input = best;
}

void pack(int arr[], map_t map){
	arr[0] = map.nodes.size();
	int i;
	for(i = 1; i<= map.nodes.size(); i++){
		arr[(i*3)-2] = map.nodes[i-1].name;
		arr[(i*3)-1] = map.nodes[i-1].color;
		arr[(i*3)] = map.nodes[i-1].numbOfedges;
	}

	i = ((i-1) * 3 ) + 1;
	for( int a = 0; a < map.nodes.size(); a++){
		for(int c= 0; c < map.nodes[a].numbOfedges; c++){
			arr[i] = (map.nodes[a].edges[c]->name);
			i++;
		}
	}
	

}
void unpack(int arr[], map_t &map){
	int i;
	for(i = 1; i < (arr[0])*3; i=i+3){
		node_t temp;
		temp.name = arr[i];
		temp.color = arr[i+1];
		temp.numbOfedges =arr[i+2];
		map.nodes.push_back(temp);
	}
	for(int c = 0; c < map.nodes.size(); c++){
		for(int f= 0; f <map.nodes[c].numbOfedges;f++){
			map.nodes[c].edges.push_back(&map.nodes[(arr[i])-1]);
			i++;
		}
	}
}

void readIn(map_t &graph){
	//takes input to make the graph
	//first you input the name of the nodes, these must be whole numbers starting at 1
	//next you input -1 to say you are don making nodes then you type the name of the node you want to
	//make neighbors for once you have enderd all the nieghbors of that node you enter -2 to ender then 
	//name of the node you want to enter nighbors for next and fianll end with a -1
	// 
	int inputString;
	int nodeName;
	cout<< "please enter the name of the nodes then type -1 when done\n";
	cin>>inputString;

	while( inputString != -1){  
		node_t temp;
		temp.name = inputString;
		temp.numbOfedges = 0;
		graph.nodes.push_back(temp);
		cin>>inputString;
	}
	
	inputString = 0;
	while( inputString != -1){
		cout<<"now enter the name of the node and its neighbors, type -2 befor entering the next nodes neighbors, and -1 when you are finished"<< endl;
		cin>> inputString;
		nodeName = inputString;
		int c = 0;
		while(inputString != -2 && inputString > 0){
			cout<<"next neighbor pleas\n";
			cin>>inputString;
			if(inputString != -2){
				
				graph.nodes[nodeName-1].edges.push_back(& graph.nodes[inputString - 1]);
				graph.nodes[nodeName-1].numbOfedges++;
				c++;
	
			}
		}
		
	}

}

int main(int argc, char ** argv)
{
	MPI_Init(NULL, NULL);
	srand(time(NULL));
	int world_size;
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	
	int world_rank;
  	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
	MPI_Status status;
	int messageSize;
	map_t temp;
	int n;
	int signal;
	node_t nodeTest;
	nodeTest.name = 1;
	nodeTest.color = 1;
  	if(world_rank == 0){
  		readIn(temp);
		n = (temp.nodes.size()*temp.nodes.size())+(2*(temp.nodes.size()));
		int packMe[n];
		cout<< "coloring graph"<<endl;
		initColor(temp);
		pack(packMe, temp);
		messageSize = n;
		for(int i = 1; i < world_size ; i++){
			MPI_Send(&n, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
			MPI_Send(&messageSize, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
			MPI_Send(&packMe, messageSize, MPI_INT, i, 0, MPI_COMM_WORLD);
			
  		}	
  		map_t best = temp;
		
  		cout<<"best = temp\n";

		for(int i = 1; i < world_size ; i++){
			map_t test;
			cout<<"getting size of buffer\n";
			MPI_Recv(&messageSize, 1, MPI_INT, i,0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  			cout<<"getting gaph from world "<<i<<endl;
			MPI_Recv(&packMe, messageSize, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			unpack(packMe, test);
			cout<<"unpacking graphs\n";
			fittness(best);
			fittness(test);
  			if(fittness(temp)<fittness(best)){
				cout<<"in the if\n";
  				best = temp;
  			}
		}
		int f = fittness(best);
		cout<<"best graph had a fitness of "<< f<<"and the node labels are\n";
		for(int y= 0; y < best.nodes.size(); y++){
			cout<<"node "<< y+1<<" has label of "<<best.nodes[y].color<<endl;
		}

  	}
  	else if(world_rank > 0){
  		MPI_Recv(&n, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		int unpackMe[n];
		MPI_Recv(&messageSize, 1, MPI_INT, 0,0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		MPI_Recv(&unpackMe, messageSize, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		unpack(unpackMe, temp);
		cout<<"anealing at world "<<world_rank<<endl;
		aneiling(temp, (rand()%100 +50), .95, 20);
		pack(unpackMe, temp);
		
		
		MPI_Send(&messageSize, 1, MPI_INT, 0,0, MPI_COMM_WORLD);
		MPI_Send(&unpackMe, messageSize, MPI_INT, 0, 0, MPI_COMM_WORLD);
		cout<<"graph sent from world "<<world_rank<<endl;
		
	
		
		}
	
  	MPI_Finalize();
	return 0;
}
