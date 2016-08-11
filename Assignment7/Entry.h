#ifndef ENTRY_H
#define ENTRY_H
#include<string>
#include<vector>
#include<iostream>

const double CLUSTER = 512;//variable cluster size
struct Entry
{
	//variable for file name
	std::string filename;
	std::vector<short>clusters;//blocknum of clusters occupied
	std::vector<std::pair<short, short> >index;//location of clusters occupied
    unsigned bytes;//size in bytes
	int clustersize;//size of clusters occpied
	//Contructor accepting name of file and size of file as arguments.
	Entry(std::string name, unsigned size)
	{
		filename = name;
		this->bytes = size;
		findClusterSize();//once values have been assigned find cluster size
		
	}
	//finds amount of clusters a file takes up
	 void findClusterSize()
	{
		 if (bytes != 0)//if bytes does not equal 0
		 {
			 int result = .9999 + (((double)bytes) / CLUSTER);//bytes / CLUSTER and add .9999 to round up.
			 clustersize = result;//assigne result to clustersize
			 
		 }
		 else
		 {
			 clustersize = 0;//assign cluster size to 0 if bytes is 0
		 }
			
	}
	 //Function used to resize the amount of clusters accupied by file
	 void resize(unsigned Bytes)
	 {

		 if (Bytes != 0)//if Bytes does not equal 0
		 {
			 int result = .9999 + (((double)Bytes) / CLUSTER);//do arithmetic ,add .9999 to round up
			 clustersize = result;//assign result to clustersize
			 bytes = Bytes;//get new size
			 return;
		 }
		 else //if bytes equal to 0
		 { 
			 clustersize = -1;//assign clustersize to -1
			 bytes = 0;//bytes equal to 0
			 clusters.clear();//clear cluster vector
		 }
		 
	 }

	 //This function prints the vector of clusters
	 void printClusters()
	 {
		 //if vector size is equal to zero display none
		 if (clusters.size() == 0)
		 {
			 std::cout << "none.";
		 }

		 //loop through cluster vector
		 for (unsigned i = 0; i < clusters.size(); i++)
		 {
			 std::cout << clusters[i] << "  ";//display block value
		 }
		 std::cout << "\n";
	 }
};


#endif