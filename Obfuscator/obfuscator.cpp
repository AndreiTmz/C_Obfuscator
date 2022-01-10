#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <windows.h>
#include <stdio.h>
#include <regex>
#include <map>

#define MAX_LINE 512

using namespace std;

bool REMOVE_DEFAULT_LINE_BREAKS = true; // keep initial "\n"s or remove them and add them in other places
vector<string> DATA_TYPES = { "int", "float", "char", "short", "void" };

map<string, string> CUSTOM_DEFINES = { make_pair("printf", "x7n1rp"),
									   make_pair("scanf", "fn4c5"),      
									   make_pair("main", "ex17"),       //ex17 = exit -> replace value for main   
									   make_pair("sizeof","f03z15"),
									   make_pair("return", "jUmP"),
									   make_pair("int", "_1n7"),
									   make_pair("char", "r4hc"),
									   make_pair("[", "rsb"),  //rsb - right square bracket -> replace value for the left square bracket
									   make_pair("]", "lsb"),
									   make_pair("if", "_1_x_f_"),
									   make_pair("for", "_r_x_0_x_f"), 
									   make_pair("while", "_3l1hvv"), 
									   make_pair("void", "d10v") 
									 };

//split the whole code into a vector of strings
vector<string> splitString(string text, char separator) {
	stringstream fileNameStream(text);
	string segment;
	vector<string> fileNameSplitted;

	while (getline(fileNameStream, segment, separator)) {
		fileNameSplitted.push_back(segment);
	}

	return fileNameSplitted;
}

//check if parameter file is a ".c" file
bool isFileNameValid(string fileName) {

	vector<string> fileNameSplitted = splitString(fileName, '.');

	string extension = fileNameSplitted.at(fileNameSplitted.size() - 1);
	if (fileNameSplitted.size() < 2 || extension.compare("c")) {
		return false;
	}

	return true;
}

// add the random number to the initial file name
string alterFileName(string fileName, int random_number) {
	vector<string> fileNameSplitted = splitString(fileName, '.');
	int size = fileNameSplitted.size();

	string alteredName = fileNameSplitted.at(0);

	for (int i = 1; i < size - 1; i++) {
		alteredName += "." + fileNameSplitted.at(i);
	}

	alteredName += "_" + to_string(random_number);
	alteredName += "." + fileNameSplitted.at(size - 1);

	return alteredName;
}

/*****
* Source of inspiration for this method:
* https://www.go4expert.com/articles/download-file-using-urldownloadtofile-c-t28721/
*****/
// make a call to random.org to get a truly random number (not pseudo random number)
int generateRandomNumber() {
	HRESULT dl;

	char url[MAX_LINE] = "https://www.random.org/integers/?num=1&min=10000&max=999999&col=1&base=10&format=plain&rnd=new";
	char destination[MAX_LINE] = "random_number.txt";
	char buffer[MAX_LINE];

	typedef HRESULT(WINAPI* URLDownloadToFileA_t)(LPUNKNOWN pCaller, LPCSTR szURL, LPCSTR szFileName, DWORD dwReserved, void* lpfnCB);
	URLDownloadToFileA_t xURLDownloadToFileA;
	xURLDownloadToFileA = (URLDownloadToFileA_t)GetProcAddress(LoadLibraryA("urlmon"), "URLDownloadToFileA");

	dl = xURLDownloadToFileA(NULL, url,
		destination, 0, NULL);

	int random_number = -1;

	if (dl == S_OK)
	{
		ifstream rand_file(destination);

		rand_file >> random_number;
		rand_file.close();
		remove(destination);
	}
	else if (dl == E_OUTOFMEMORY)
	{
		cout << "Failed To Download File Reason: Insufficient Memory";
	}
	else
	{
		cout << "Failed To Download File Reason: Unknown";
	}
	return random_number;
}

//check if vector contains given text
bool isStringInVector(vector<string> &v, string text) {
	return count(v.begin(), v.end(), text);
}

// check if map contains any <key,value> pair with a given key (the parameter text in this case)
bool isStringInMapKeys(map<string, string> map, string text) {
	return map.count(text);
}

// method to find the last preprocessor directive
// nedeed in order to add custome #define directives
int findLastPreProcessor(vector<string>& code, string pre) {
	int pos = 0;
	for (int i = 0; i < code.size(); i++) {
		if (code[i] == pre) {
			pos = i;
		}
	}
	return pos;
}

//method to preserve line breaks in the first part of the code (directives must be one per line)
int getPosUntilPreserveLineBreaks(vector<string> &code) {
	int lastDefinePosition = findLastPreProcessor(code, "#define");
	int lastIncludePosition = findLastPreProcessor(code, "#include");

	//add 2 or 3 to jump the name of the library included or the define name and value
	int posUntilPreserveLineBreaks = lastDefinePosition > lastIncludePosition ? lastDefinePosition + 3 : lastIncludePosition + 2;
	
	return posUntilPreserveLineBreaks;
}

// removes "\n"
void removeLineBreaks(vector<string>& code) {
	for (int i = 0; i < code.size(); i++) {
		if (code[i] == "\n" && i > getPosUntilPreserveLineBreaks(code)) {
			code[i] = " ";
		}
	}
}

//removes comments and empty positions
void cleanContent(vector<string>& v) {
	vector<string>::iterator i = v.begin();
	while (i != v.end())
	{
		if (*i == "/*") {
			i = v.erase(i);
			while (*i != "*/") {
				i = v.erase(i);
			}
			i = v.erase(i);
		}
		else if (*i == "//") {
			i = v.erase(i);
			while (*i != "\n") {
				i = v.erase(i);
			}
			i = v.erase(i);
		}
		else if (*i == "")
		{
			i = v.erase(i);
		}
		else {
			i++;
		}
			
	}
}

//method to identify variables & methods names
void identifyVariablesAndMethods(vector<string> &splitted_source_code, map<string, string> &methods, map<string, string> &variables) {

	bool variables_in_row = false;  //variables declared like "int a, b, c;"
	bool end_of_variables_in_row = false;

	string underscores = "_";

	for (int i = 0; i < splitted_source_code.size(); i++) {
		string word = splitted_source_code.at(i);

		if (isStringInVector(DATA_TYPES, word) || variables_in_row) {
			string variable = splitted_source_code.at(i + 1);

			variables_in_row = variable.at(variable.length() - 1) == ',';

			if (variable.length() >= 2 && variable.compare("main(") != 0 && variable.at(variable.length() - 1) == '(' && 
				!isStringInMapKeys(methods, variable)) {
				//addItemToMap(methods, variable, underscores);
				methods.insert(make_pair(variable, underscores));
				underscores += "_";
			}
			else if ((variable.compare("main(") != 0) && !isStringInMapKeys(variables, variable) &&
				      word.compare("void") != 0) {
				//addItemToMap(variables, variable, underscores);
				if (variable[variable.size() - 1] == ',') {
					variable.pop_back();
				}
				variables.insert(make_pair(variable, underscores));
				underscores += "_";
			}
		}
	}
}

// used after identifying vairbales and methods. This method replaces them with underscores
void replaceInCode(map<string, string> map, vector<string> &code) {
	bool insideString = false;

	for (int i = 0; i < code.size(); i++) {
			
		string word = code[i];	
		
		//0x22 - hex value for quote marks (")
		//ox5c - hex value for backslash (\)
		if (word.at(0) == 0x22) {
			if (insideString && code[i-1].at(0) != 0x5c) { 
				insideString = false;
				/*cout << "\n String end:" << word;*/
			}
			else {
				insideString = true;
				/*cout << "\n String Start:" << word;*/
			}
		}
		else if (word.at(word.length() - 1) == 0x22) {
			insideString = false;
			/*cout << "\n String end:" << word;*/
		}

		if (!insideString) {
			bool inRow = word[word.size() - 1] == ',';
			if (inRow) {
				word.pop_back();
			}

			if (map.count(word)) {
				code[i] = inRow ? map.at(word) + "," : map.at(word);

				if (word.length() >= 2 && word.at(word.length() - 1) == '(') {
					code[i] += "(";
				}
			}
		}
	}
}

//after doing some changes to the string vector, use this method to build the string back
string buildObfuscatedCode(vector<string> &code) {
	if (REMOVE_DEFAULT_LINE_BREAKS) {
		removeLineBreaks(code);
	}
	
	string obfuscated_code = "";
	bool insideString = false;
	int breakCount = 0;

	for (int i = 0; i < code.size(); i++) {

		string word = code[i];

		if (word == "\"" && code[i + 1] == " " && code[i + 2] == "\"") {
			i += 2;
			continue;
		}

		//0x22 - hex value for quote marks (")
		//ox5c - hex value for backslash (\)
		if (word.at(0) == 0x22) {
			if (insideString && code[i - 1].at(0) != 0x5c && code[i-1].at(code[i-1].length() - 1) != 0x5c) {
				insideString = false;
			}
			else {
				insideString = true;
			}
		}
		else if (word.at(word.length() - 1) == 0x22) {
			insideString = false;
		}
		

		obfuscated_code += code[i] + " ";

		if (REMOVE_DEFAULT_LINE_BREAKS && i > getPosUntilPreserveLineBreaks(code)) {
			if (!insideString) {
				breakCount++;
				if (breakCount == 20) {
					obfuscated_code += "\n";
					breakCount = 0;
				}
			}
		}
	}

	obfuscated_code = regex_replace(obfuscated_code, regex("\\ \""), "\"");
	obfuscated_code = regex_replace(obfuscated_code, regex(" <"), "<");
	obfuscated_code = regex_replace(obfuscated_code, regex("\" "), "\"");
	obfuscated_code = regex_replace(obfuscated_code, regex("\"%d\""), "\"%d \"");
	obfuscated_code = regex_replace(obfuscated_code, regex(":"), ": ");
	return obfuscated_code;
}

//some code formatting for easier parsing
void formatCode(string &content) {
	content = regex_replace(content, regex("\n"), " \n ");
	content = regex_replace(content, regex("\t"), " ");
	content = regex_replace(content, regex(";"), " ; ");
	content = regex_replace(content, regex("\\("), "( ");
	content = regex_replace(content, regex("\\)"), " ) ");
	content = regex_replace(content, regex("\\["), " [ ");
	content = regex_replace(content, regex("\\]"), " ] ");
	content = regex_replace(content, regex("\\+\\+"), " ++");
	content = regex_replace(content, regex("\\-\\-"), " --");
	content = regex_replace(content, regex("\""), " \" ");
	content = regex_replace(content, regex("\\\""), " \" ");
	content = regex_replace(content, regex("<"), " <");
}

// method to add custom #define directives
void addCustomDefineInstructions(vector<string>& code) {
	int defineInsertPos = getPosUntilPreserveLineBreaks(code);
	vector<string>::iterator it;
	map<string, string>::iterator itr = CUSTOM_DEFINES.begin();
	string firstCustomDef = itr->first;

	for (pair<string, string> customDef : CUSTOM_DEFINES) {
		it = code.begin() + defineInsertPos;
		if (customDef.first == firstCustomDef) {
			code.insert(it, "\n#define " + customDef.second + " " + customDef.first + "\n");
			
		}
		else {
			code.insert(it, "\n#define " + customDef.second + " " + customDef.first);
		}	
	}
}

// use the added #define directives to replace keywords in code
void useCustomDefineInstructions(vector<string>& code) {
	int defineInsertPos = getPosUntilPreserveLineBreaks(code);
	defineInsertPos += CUSTOM_DEFINES.size();
	vector<string>::iterator it;

	bool insideString = false;

	for (int i = defineInsertPos; i < code.size(); i++) {
		string word = code[i];
		if (word.at(0) == 0x22) {
			if (insideString && code[i - 1].at(0) != 0x5c) {
				insideString = false;
				/*cout << "\n String end:" << word;*/
			}
			else {
				insideString = true;
				/*cout << "\n String Start:" << word;*/
			}
		}
		else if (word.at(word.length() - 1) == 0x22) {
			insideString = false;
			/*cout << "\n String end:" << word;*/
		}

		if (!insideString) {
			for (pair<string, string> customDef : CUSTOM_DEFINES) {
				if (strstr(code[i].c_str(), customDef.first.c_str())) {
					if (!(customDef.first == "int" && strstr(code[i].c_str(), "printf"))) {
						if (customDef.first == "[" || customDef.first == "]") {
							code[i] = regex_replace(code[i], regex("\\" + customDef.first), customDef.second);
						}
						else {
							code[i] = regex_replace(code[i], regex(customDef.first), customDef.second);
						}
						break;
					}
				}
			}
		}
	}
}

int main(int argc, char* argv[]) {
	if (argc != 2) {  
		cout << "Please send the name of your file as a command line argument.\n";
		cout << "Example: obfuscator.exe myfile.c";
	}
	else {
		string fileName = argv[1];

		if (!isFileNameValid(fileName)) {
			cout << "Please enter a valid file name. (Only .c files accepted)";
		}
		else {
			ifstream file;
			file.open(fileName);

			if (!file.is_open()) {
				cout << "File not found!";
			}
			else {
				map<string, string> variables;
				map<string, string> methods;
				vector<string> splitted_source_code;

				//read file
				string content((istreambuf_iterator<char>(file)), (istreambuf_iterator<char>()));

				//a little bit of formatting the string
				formatCode(content);
				
				splitted_source_code = splitString(content, ' ');
				cleanContent(splitted_source_code);

				//identify variables and replace them with underscores
				
				identifyVariablesAndMethods(splitted_source_code, methods, variables);
				
				// first we replace methods because they have a '(' at the end and this way 
				// we avoid replacing method names when calling the function for replacing only variables
				replaceInCode(methods, splitted_source_code);
				replaceInCode(variables, splitted_source_code);

				addCustomDefineInstructions(splitted_source_code);
				useCustomDefineInstructions(splitted_source_code);
				
				string obfuscated_content = buildObfuscatedCode(splitted_source_code);

				cout << "\nIdentified variables:";
				for (pair<string, string> variable : variables) {
					cout << endl << variable.first << " " << variable.second;
				}

				cout << "\n\nIdentified methods:";
				for (pair<string,string> m : methods) {
					cout <<endl << m.first << " " << m.second;
				}

				//generate random number and append it to file name
				int random = generateRandomNumber();

				if (random != -1) {
					fileName = alterFileName(fileName, random);
				}

				//write the modified content into a new file
				ofstream obfuscated(fileName);
				obfuscated << obfuscated_content;
			}
		}
	}

	cout << "\n\n";
}