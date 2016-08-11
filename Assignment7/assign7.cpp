#include<iostream>
#include<vector>
#include<iomanip>
#include<string>
#include<fstream>
#include "Entry.h"
/*
NAME:CODY HAMMOND
Z-ID: 1615158
ASSIGNMENT 7
*/

//various function prototypes
void modify();
void allocate();
void allocateFAT(Entry*&);
void rename();
void deallocate(); 
void deallocateFAT(Entry*&);
void copy();
void printFAT();
void removeEntry(Entry&);
void printEntries();
Entry* search(std::string);

//counter for printing
short counter = 0;
//const variables for printing and entry size
const unsigned HOW_OFTEN = 4;
const unsigned COL = 25;
const unsigned MAX = 12;
//global FAT table
std::vector<std::vector<short> >FAT(COL);
//global entry vector
std::vector<Entry>entries;
//file object
std::fstream file;

int main()
{
	//make directory entry
	Entry* directory = new Entry(".", 512);
	//assign entry object FAT table coordinates
	directory->index.push_back(std::pair<short, short>(0, 0));
	//assign cluster number
	directory->clusters.push_back(0); 
	//push into entry vector
	entries.push_back(*directory);
	
	//make parent directory
	Entry* parent = new Entry("..", 0);
	//push into entry vector
	entries.push_back(*parent);
	
	//complete the structure of the FAT table
	for (unsigned i = 0; i < FAT.size(); i++)
	{
		FAT[i].resize(12);
	}

	//first location is reserved for directory.
	FAT[0][0] = -1;

	file.open("data7.txt");//open file

	if (!file.is_open())//if file not open
	{
		std::cout << "File did not open, terminating program.";//display message
		return -1;
	}

	char symbol = ' ';//initialized symbol variable
	printEntries();//print entries
	printFAT();//print FAT table
	while (symbol != '?')//while symbol is not the delimiter
	{
		file >> symbol;//read in symbol
		if (counter == HOW_OFTEN)
		{
			printEntries();
			printFAT();
			counter = 0;
		}
		

		switch (symbol)//switch statement
		{
			case 'N':
				allocate();//if N call allocate function
				break;
			case 'C':
				copy();//if C call copy function
				break;
			case 'M'://if M call modify function
				modify();
				break;
			case 'D'://if D call deallocate function
				deallocate();
				break;
			case 'R':
				rename();//if R call rename function
				break;
		default://if none of the above
			break;
		}
		counter++;
	}

	printEntries();//print entries
    printFAT();//print FAT table

}


//This function allocates a file
void allocate()
{
	std::string name;//variable for name
	unsigned size;//variable for byte size
	//read in name and size
	file >> name;
	file >> size;
	std::cout << "\nTransaction,add a new file.\n";//display operation
	Entry* entry = search(name);//search name
	if (entry != nullptr)//if entry is not null
	{
		std::cout << "Error, a file with that name already exists.\n";//display error message
		return;
	}
		
    //create new entry object
	entry = new Entry(name, size);
	
	//insert entry into vector
	entries.push_back(*entry);
	//check for directory limit
	if ((entries.size() - 1) % MAX == 0)
	{
		Entry* directory = &entries[0];//get directory
		directory->resize(directory->bytes + CLUSTER);//resize directory
		allocateFAT(directory);//allocate the new size
		
	}
	
	//if size is not 0
	if (size != 0)
	{
		Entry* entry = &entries.back();//get last entry in vector
		allocateFAT(entry);//allocate entry in table
	}
	
	std::cout << "New file " << name << " of size " << size << " created.\n";//display sucess message
}

//This function updates the contents of the FAT table when allocating files. Accepts a reference to a pointer.
void allocateFAT(Entry*&entry)
{
	
	
	//variable size and count to find suitable amount of empty clusters
	unsigned size = entry->clustersize;
	unsigned count = entry->index.size();
	//if size is equal to 0 exit function
	if (size == 0)
		return; 

	//variable to hold the block number
	unsigned blocknum = 0;

	//nested for loop to search through FAT table
	for (unsigned i = 0; i < COL; i++)
	{
		for (unsigned j = 0; j < MAX; j++)
		{
			//if count is less than size
			if (count < size)
			{
				if (FAT[i][j] == 0)//if location has an available space
				{
					//insert table coordinates of space into entry object
					entry->index.push_back(std::pair<short, short>(i, j));
					//insert clusters occupied into entry object
					entry->clusters.push_back(blocknum);	
					count++;//increment count
				}
				blocknum++;//increment block number
			}
			else
			{
				j = MAX;//assign max value to break condition
				i = COL;//assign max value to break condition
			}
		}
	}

	//variable to hold a table coordinates
	std::pair<short, short>location;
	int k = 0;//counter variable
	while( k < entry->clustersize-1)//loop through entry's index table and place clusters numbers in table coordinates
	{
		location = entry->index[k];//get location
		FAT[location.first][location.second]=entry->clusters[k+1];//insert cluster num into FAT
		k++;//increment k
	}

	location = entry->index[k];//get last location
	FAT[location.first][location.second] = -1;//insert ending value
	
}

//This function performs the copy transaction
void copy()
{
	//variables to store data read from file
	std::string name;
	std::string copy;
	file >> name;//read data
	file >> copy;
	Entry* entry = search(name);//search name

	std::cout << "\nTransaction, copy file.\n";//display transaction
	
	if (entry == nullptr)//if entry is null
	{
		std::cout << "Error, source file does not exists.\n";//display error
		return;
	}
		

	Entry* copyEntry = search(copy);//search copy name

	if (copyEntry != nullptr)//if copy is not null 
	{
		std::cout << "Error,destination file already exists.\n";//display error
		return;
	}
	
	copyEntry = new Entry(copy, entry->bytes);//make new entry object

	allocateFAT(copyEntry);//allocate clusters
	entries.push_back(*copyEntry);//push entry into vector
	std::cout << "Copy Successful.\n";
}


//This function performs the modify transaction.
void modify()
{
	//variables to read from file
	std::string name;
	unsigned size;
	file >> name;//read in data
	file >> size;
	Entry* entry=search(name);//search name

	std::cout << "\nTranscation, modify file.\n";//display transaction
	if (entry == nullptr)//if entry is null
	{
		std::cout << "Error, file not found.\n";//display error
		return;
	}

	//if entry size is greater than new size
	if (entry->bytes > size)
	{
		entry->resize(size);//resize entry struct
		
		deallocateFAT(entry);//deallocate clusters
		
	}
	else if (entry->bytes < size)//if entry size is less than new size
	{
		entry->resize(size);//resize entry struct
		allocateFAT(entry);//allocate clusters
	}

	std::cout << "Successfully modified file," << name << "\n";//display success message
}


//This function deallocates clusters in the FAT table. Takes a reference to an Entry pointer as an argument
void deallocateFAT(Entry*& entry)
{
	int size = entry->clustersize;//get cluster size
	int count = entry->index.size()-1;//get index size
	std::pair<short, short>location;//variable to store FAT coordinates
	while(count >= size) //while count is greater than size
	{
		if (count == -1)//if count == -1 exit function
			return;

		location = entry->index[count];//get location
		FAT[location.first][location.second] = 0;//set cluster location to zero
		entry->index.pop_back();//remove coordinate from index
		count--;//decrement count
	}
	if (entry->index.size() != 0)//if index size
	{
		location = entry->index[count];
		FAT[location.first][location.second] = -1;
	}
	entry->clusters.resize(size);

}

//This function performs the deallocate transaction
void deallocate()
{
	//variables for file data
	std::string name;
	file >> name;//read in data
	Entry* entry=search(name);//search name

	std::cout << "\nTransaction,delete file.\n";//display transaction
	if (entry == nullptr)//if entry is null
	{
		std::cout << "Error,file not found.\n";//display error
		return;
	}
		
	entry->clustersize = -1;//set cluster size to negative one, marking deletion.
	deallocateFAT(entry);//deallocate clusters
	removeEntry(*entry);//remove from vector
	std::cout << "Successfully deleted file.\n";
	
}

//This function peformes the rename transaction
void rename()
{
	//variables for file data
	std::string name;
	std::string newName;
	file >> name;//read in data
	file >> newName;
	Entry* entry=search(name);//search for name
	if (entry==nullptr)//if null exit function
		return;

	entry->filename = newName;//rename entry.

	std::cout << "Successfully renamed file " << name << " to " << newName << ".\n";
}

//This function searchs the name of a file and returns a pointer to an entry struct or null. Accepts a string argument.
Entry* search(std::string name)
{
	//loop through entries vector
	for (unsigned i = 0; i < entries.size(); i++)
	{
		if (name == entries[i].filename)//if match
			return &entries[i];//return address of entry
	}

	return nullptr;//return null 
}

//This function removes an entry from the entries vector. Takes a reference to an entry as an argument
void removeEntry(Entry& entry)
{
	std::vector<Entry>::iterator iter;//iterator
	for (iter = entries.begin(); iter != entries.end(); iter++)
	{
		if (iter->filename == entry.filename)//if match
		{
			entries.erase(iter);//delete from vector
			return;
		}
	}
}

//This function prints the entries
void printEntries()
{
	//For loop to print information.
	for (unsigned i = 0; i < entries.size(); i++)
	{
		std::cout << "\nFile Name: " << entries[i].filename << "      Size: "<<entries[i].bytes;
		std::cout << "\nClusters in use: ";
		entries[i].printClusters();//print clusters occupied
	}
}


//This function prints out the FAT table and total file and byte size
void printFAT()
{
	int total = 0;//variable for total
	std::cout << "\n";
	for (unsigned i = 0; i < COL; i++)
	{
		for (unsigned j = 0; j < MAX; j++)
		{
			//print clusters
			std::cout << std::setw(8) << FAT[i][j];
			if (j == 11)
			{
				std::cout << "\n";
			}
		}
	}
	//add up bytes
	for (unsigned i = 0; i < entries.size(); i++)
	{
		total += entries[i].bytes;
	}
	//print total files and file size
	std::cout << "\nTotal File Size: " << entries.size()<<"    Total Byte Size: " << total ;
	std::cout << "\n";
}