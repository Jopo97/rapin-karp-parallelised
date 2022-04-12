//Parallelised Rabin Karp algorithm implementation created by Jonah McElfatrick for CMP202 Data structures and Algorithms 2

#include <iostream>
#include <string>
#include <fstream>
#include <list>
#include <chrono>
#include <mutex>
#include <thread>
#include <vector>
using namespace std;

#define alph 256

//Clock benchmarking
using chrono::duration_cast;
using chrono::milliseconds;
using this_thread::sleep_for;
typedef chrono::steady_clock the_clock;

const int prime = 101;

mutex cout_mutex;
bool cout_bool = false;
condition_variable cout_cv;

int findHash(string txt, string pattern, int position, int txtHash, int patternLength, int h) {

	txtHash = ((alph * (txtHash - (h * txt[position]))) + txt[position + patternLength]) % prime;
	if (txtHash < 0) {
		txtHash = txtHash + prime;
	}

	return txtHash;
}

vector<int> BoyerMoore(string txt, string pattern, vector<int> FoundPositions) {
	//Holds the number of position jumps that can be made for each character
	int jumps[256];

	//Stores the number of times that the search loop is run
	int operations = 0;

	//Length of the text contents and the pattern
	int pat_len = pattern.size();
	int txt_len = txt.size();

	//Fills the jumps array with the number of jumps needed for each character that might appear
	for (int i = 0; i < 256; i++) {
		//Fils the entire jumps array with the length of the pattern as a base point to then calculate the number of jumps needed for each character
		jumps[i] = pat_len;
	}

	for (int i = 0; i < pat_len; i++) {
		//Calculates the number of jumps required for each character that is in the pattern
		//and overwrites the corresponding 8-bit character with the number of jumps
		jumps[int(pattern[i])] = (pat_len - 1) - i;
	}

	//Prints out the comparred values and compares them together
	for (int i = 0; i < txt_len - pat_len; ++i) {
		operations++;
		//Initialise variable j for 'jumps' 
		int j = jumps[int(txt[i + pat_len - 1])];
		if (j != 0) {
			//Jumps forwards the number of positions required to 
			i += j - 1;
			continue;
		}
		int h;
		//Loop to check to see if the pattern is the same as the comparrison value
		for (h = 0; h < pat_len; h++) {
			if (txt[i + h] != pattern[h]) {
				//Pattern is not found and breaks from the loop
				break;
			}
		}
		if (h == pat_len) {
			//Pattern is found and then added onto the found_positions 
			cout_mutex.lock();
			cout << pattern << " found at position: " << i << endl;
			cout_mutex.unlock();
			cout_cv.notify_one();
			FoundPositions.push_back(i);
		}
	}
	return FoundPositions;
}

vector<int> RabinKarp(string txt, string pattern, vector<int> FoundPositions) {
	//
	int txtHash = 0, patternHash = 0;

	//Stores the number of times that the search loop is run
	int operations = 0;

	//prime number for the reduction of false hits when comparring hashes
	const int prime = 101;

	// h = multiplier for MSB
	int h = 1;

	//stringSize variable for comparring the size of the pattern to the string
	int stringSize = 0;

	//Length of the text being processed and the length of the pattern being searched for
	int txtLength = txt.length();
	int patternLength = pattern.length();

	// Calculate h, the value of h would be eqv to pow(alph, patternLength-1)
	for (int i = 0; i < patternLength - 1; i++) {
		h = (h * alph) % prime;
	}

	// Calculate the starting hash for the text at the start of the whole text string and then the pattern hash
	// The text hash will change frequently later on when comparring the two
	for (int i = 0; i < patternLength; i++) {
		txtHash = ((alph * txtHash) + txt[i]) % prime;
		patternHash = ((alph * patternHash) + pattern[i]) % prime;
	}

	for (int i = 0; i <= txtLength - patternLength; i++) {
		operations++;
		//show_context(txt, i);
		// Compare the hash of the text and the pattern
		if (txtHash == patternHash) {
			//Hashes match, therefore now check if characters match up
			for (int j = 0; j < patternLength; j++) {
				//Variable for the size of the string being comparred
				stringSize = j + 1;
				if (txt[i + j] != pattern[j]) {
					break;
				}
			}
			//Checks to see if the length of the pattern is equal to the length of the string being compared
			if (stringSize == patternLength) {
				cout_mutex.lock();
				cout << pattern << " found at position: " << i << endl;
				cout_mutex.unlock();
				cout_cv.notify_one();
				FoundPositions.push_back(i);
			}
		}
		// calculating the next hash for the next part of txt
		//txtHash = findHash(txt, pattern, i, txtHash, patternLength, h);
		txtHash = ((alph * (txtHash - (h * txt[i]))) + txt[i + patternLength]) % prime;
		if (txtHash < 0) {
			txtHash = txtHash + prime;
		}
	}
	return FoundPositions;
}

string inputFile() {
	//Variables for the contents of the file
	string contents;

	//Opens the text file for reading
	ifstream file("3000names.txt", ios_base::binary);

	//Check to see if the TextFile.txt file was sucessfully opened
	if (!file.good()) {
		cout << "Unable to open Text File" << endl;
		system("pause");
		exit(1);
	}

	//Finds end of text file
	file.seekg(0, ios::end);

	contents.reserve(file.tellg());

	//Finds beginning of text file
	file.seekg(0, ios::beg);

	//Assigns the contents of the text file to the string variable contents
	contents.assign(istreambuf_iterator<char>(file), istreambuf_iterator<char>());

	//Closes the input textfile
	file.close();

	return contents;
}

int validateInt(int min, int max, int value) {
	while (value < min || value > max) {
		cout << "Value out of range, please enter a value between " << min << " and " << max << ": ";
		cin >> value;
		cout << endl;
	}
	return value;
}

void recieve_input(vector<string> nameList) {
	bool emailbool = true;
	int noNames = 0;
	cout << "How many names would you like to search for (1 - 5)?: ";
	cin >> noNames;
	cout << endl;
	noNames = validateInt(1, 5, noNames);
	for (int i = 0; i < noNames; i++) {
		cout << "Please enter the next name: ";
		cin >> nameList[i];
		cout << endl;
	}
}

int main() {
	//Variables for holding the text contents of a file, and the pattern that is being searched for
	string txt;
	string patterns[5] = {};
	patterns[0] = "liam1@outlook.com";
	patterns[1] = "noah4@yahoo.co.uk";
	patterns[2] = "william0@aol.com";
	patterns[3] = "james9@aol.com";
	patterns[4] = "logan5@gmail.co.uk";

	thread *tarray[10];
	vector<vector<int>> Found_Positions;
	//Found_Positions.resize(10);
	//vector<string> patternList;
	//recieve_input(patternList);

	//Fills the variable 'txt' with the values from the text file
	txt = inputFile();

	for (int i = 0; i < 10; i++) {
		//string pattern = patterns[i];
		if (i < 5) {
			tarray[i] = new thread(RabinKarp, txt, patterns[i], Found_Positions[i]);
		}
		else {
			tarray[i] = new thread(BoyerMoore, txt, patterns[i - 5], Found_Positions[i]);
		}
	}

	for (int x = 0; x < 10; x++) {
		// Wait for myThread to finish.
		tarray[x]->join();
	}

	for (int i = 0; i < 10; i++) {
		cout << "Email: " << patterns[i] << endl;
		cout << Found_Positions[i].size() << endl;
		for (int x = 0; x < Found_Positions[i].size(); x++) {
			cout << "hi";
			cout << "Found at position :" << Found_Positions[i][x] << endl;
		}
	}
	//Found_Positions = BoyerMoore(txt, pattern);
	//Found_Positions = RabinKarp(txt, pattern);

	//Pauses the interface when the results have been outputted to the screen
	system("pause");
	return 0;
}