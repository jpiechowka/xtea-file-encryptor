#include <iostream>
#include <fstream>
#include <stdint.h>

using namespace std;

#define BLOCK_SIZE 8 //XTEA uses 64-bit blocks, 64 bits is 8 bytes

unsigned int key[4] = { 0xFACE, 0xDEAD, 0xBABE, 0xD00D };

//XTEA block cipher - code from Wikipedia
//https://en.wikipedia.org/wiki/XTEA

void encipher(unsigned int num_rounds, uint32_t v[2], uint32_t const key[4]) 
{
	unsigned int i;
	uint32_t v0 = v[0], v1 = v[1], sum = 0, delta = 0x9E3779B9;
	for (i = 0; i < num_rounds; i++) 
	{
		v0 += (((v1 << 4) ^ (v1 >> 5)) + v1) ^ (sum + key[sum & 3]);
		sum += delta;
		v1 += (((v0 << 4) ^ (v0 >> 5)) + v0) ^ (sum + key[(sum >> 11) & 3]);
	}
	v[0] = v0; v[1] = v1;
}

void decipher(unsigned int num_rounds, uint32_t v[2], uint32_t const key[4])
{
	unsigned int i;
	uint32_t v0 = v[0], v1 = v[1], delta = 0x9E3779B9, sum = delta * num_rounds;
	for (i = 0; i < num_rounds; i++)
	{
		v1 -= (((v0 << 4) ^ (v0 >> 5)) + v0) ^ (sum + key[(sum >> 11) & 3]);
		sum -= delta;
		v0 -= (((v1 << 4) ^ (v1 >> 5)) + v1) ^ (sum + key[sum & 3]);
	}
	v[0] = v0; v[1] = v1;
}

void xtea(char filePath[], bool boolEncrypt)
{
	//Open file
	fstream file(filePath, ios::in | ios::out | ios::binary);
	cout << endl << "Opening file: " << filePath << endl;
	if (!file) //Check if file is correctly opened
	{
		cout << "File " << filePath << " cannot be opened" << endl; 
		return;
	}

	cout << "File open" << endl;
	file.seekg(0, ios::end);
	unsigned fileSize = file.tellg(); //Get file size
	cout << "File size: " << fileSize << " bytes" << endl;
	file.seekg(ios::beg);
	file.clear();

	//Calculate number of blocks to be encrypted/decrypted
	int blockNumber = fileSize / BLOCK_SIZE;
	if (fileSize % BLOCK_SIZE != 0) { ++blockNumber; }
	cout << "Number of blocks: " << blockNumber << endl;

	//Decalre data array for file operations
	unsigned char dataArray[BLOCK_SIZE];
	unsigned filePosition = file.tellg();

	if (boolEncrypt) { cout << "Starting encryption" << endl; }
	else { cout << "Starting decryption" << endl; }
	for (int i = 0; i < blockNumber; i++)
	{
		//Get data from file
		file.seekg(filePosition);
		file.read((char*)dataArray, BLOCK_SIZE);

		//Encrypt/decrypt
		if (boolEncrypt) { encipher(32, (uint32_t*)dataArray, key); }
		else { decipher(32, (uint32_t*)dataArray, key); }

		//Write to file
		file.seekp(filePosition);
		file.write((char*)dataArray, BLOCK_SIZE);

		//Zero out the data array and increase the pos counter
		memset(dataArray, 0, BLOCK_SIZE);
		filePosition += BLOCK_SIZE;
	}
	//Close file
	file.close();
	cout << "Closing file" << endl;
	if (boolEncrypt) { cout << "File " << filePath << " has been encrypted" << endl; }
	else { cout << "File " << filePath << " has been decrypted" << endl; }
}

int main(int argc, char *argv[])
{
	if (argc != 3) 
	{
		if ((argc == 2) && (string(argv[1]) == "-h"))
		{
			cout << endl << "Usage: " << "[" << argv[0] << "]" << " [file] [-e/-d]" << endl;
			cout << "[file] is path to the file you want to encrypt/decrypt" << endl;
			cout << "[-e/-d] choose one to encrypt/decrypt" << endl;
			cout << endl << "Example usage: XTEA.exe photo.jpg -e" << endl;
		}
		else { cout << endl << argv[0] << " You have entered invalid number of parameters" << endl; }
		return 1;
	}

	bool encrypt = false;
	if ((string(argv[2]) != "-e") && ((string(argv[2]) != "-d"))) { cout << "Invalid parameter " << argv[2] << endl; return 1; }
	if (string(argv[2]) == "-e") { encrypt = true; }
	
	//Encrypt/decrypt
	xtea(argv[1], encrypt);
	return 0;
}

