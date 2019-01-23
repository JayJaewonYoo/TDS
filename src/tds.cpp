#include "tds.h"
#include "io.h"
#include "spec.h"
#include <map> // ADS
#include <sstream> // ADS

TDS::TDS(string fileName, string filePath){
    root.reset(IO::inputReader(fileName, filePath,nullptr, this));
    initialSet = false;
    supC2P = bddtrue;
}
DES* TDS::findComponent(string G){
    return root->findComponent(G);
}
event* TDS::findEvent(string s){
    auto it = Sigma.find(s);
    if (it == Sigma.end())
        return nullptr;
    else return it->second.get();
}
event* TDS::addEvent(istream& is){
    string e;
    bool ctrb;
    is >> e >> ctrb;
    if ( Sigma[e] == nullptr)
        Sigma[e].reset(new event(e,ctrb));
    return Sigma[e].get();
}
// open-loop run of the system
void TDS::setInitials(){
    P0 = root->initialPredicate();
    Pm = root->markerPredicate();
    for(auto& sigma:Sigma)
        sigma.second->setVarSets();
    initialSet = true;
}

void TDS::run(string G){
    if(!initialSet)
        setInitials();
    vector<bdd> visitedStates;
    visitedStates.push_back(P0);
    int numOfTransitions = 0;
    bdd setOfVars = bddtrue;
    bool localSimulation = false;
    if (!G.empty()){
        DES* localRoot = root->findComponent(G);
        if (localRoot == nullptr){
            cerr << "Error: Local root " << G << " not found!" << endl;
            abort();
        }
        int localFddIndex = localRoot->getfddDomain();
        for(int i = 1; i < fdd_domainnum(); i+=2){
            if (i != localFddIndex )
                setOfVars &= fdd_ithset(i);
        }
        localSimulation = true;
        cout << "Open-loop behavior under the component " << localRoot->getName() << endl;
    }
    else {
        cout << "Open loop simulation: " << endl;
    }
    runRec(P0,visitedStates,0, numOfTransitions,localSimulation, setOfVars);
    cout << "States: " << visitedStates.size() <<
            ", Transitions: " << numOfTransitions << endl;
}
void TDS::runRec(bdd current, vector<bdd>& visitedStates, int currIndex,
                 int& numOfTransitions, bool local, bdd &setOfVars){
    if (local){
        cout << currIndex << ":"; fdd_printset(bdd_exist(current,setOfVars));
    }
    else {
        cout << currIndex << ":"; fdd_printset(current);
    }

    if ( (current & Pm) != bddfalse)
        cout << "*";
    cout << endl;
    for(auto& sigma:Sigma){
        bdd next = sigma.second->delta(current);
        if ( next != bddfalse ){
            numOfTransitions++;
            cout << currIndex << "-Via: " << sigma.first <<endl;
            auto it = find_if(visitedStates.begin(), visitedStates.end(),[&](auto& P){
                return P == next;
            });
            int nextIndex = distance(visitedStates.begin(),it);
            if (it == visitedStates.end()){
                visitedStates.push_back(next);
                runRec(next,visitedStates,nextIndex, numOfTransitions, local, setOfVars);
            }
            else if (local){
                    cout << nextIndex << ":"; fdd_printset(bdd_exist(next,setOfVars));cout << endl;
                }
            else {
                cout << nextIndex << ":"; fdd_printset(next);cout << endl;
            }
         }
    }
}

//specification given in dijuntive form
//predicate corresponding to illegal set of states

void TDS::supcon(string filePath){
    if(!initialSet)
        setInitials();
    bdd P = spec::readSpec1File(filePath,this);
    P &= spec::readSpec2File(filePath, this);
    //fdd_printset(P); cout << endl;
    createSupC2P(P);
    //fdd_printset(supC2P); cout << endl;
    for(auto& sigma:Sigma)
        sigma.second->setFSigma(supC2P);
}
void TDS::createSupC2P(bdd& P){
    bdd newBDD1 = P;
    bdd newBDD2 = P;
    do{
       newBDD1 = newBDD2;
       bdd temp = supCP(newBDD1);
       newBDD2 = P & CR(temp);
    }while( newBDD1 != newBDD2);
    supC2P = newBDD2;
}
bdd TDS::CR(bdd& P){
    bdd newBDD1 = P & Pm;
    bdd newBDD2 = P & Pm;
    do{
        newBDD1 = newBDD2;
        for(auto& sigma:Sigma){
            newBDD2 |= (P & sigma.second->gamma(newBDD1));
        }
    }while( newBDD1 != newBDD2);
    return newBDD2;
}
bdd TDS::supCP(bdd& P){
    bdd temp = bdd_not(P);
    return bdd_not(Bracket(temp));
}

bdd TDS::Bracket(bdd& P){
    bdd newBDD1 = P;
    bdd newBDD2 = P;
    do{
        newBDD1 = newBDD2;
        for(auto& sigma:Sigma){
            if(!sigma.second->isControllable()){
                newBDD2 |= sigma.second->gamma(newBDD1);
            }
        }
    }while( newBDD2 != newBDD1);
    return newBDD2;
}
//
void TDS::runUnderControl(){
    cout << "Closed loop simulation: " << endl;
    if(!initialSet)
        setInitials();
    vector<bdd> visitedStates;
    if ((P0 & supC2P) == bddfalse){
        cout << "Closed behavior is empty!" << endl;
        return;
    }
    visitedStates.push_back(P0);
    int numOfTransitions = 0;
    runUnderControlRec(P0,visitedStates,0, numOfTransitions);
    cout << "States: " << visitedStates.size() <<
            ", Transitions: " << numOfTransitions << endl;
}

void TDS::runUnderControlRec(bdd current, vector<bdd>& visitedStates, int currIndex, int& numOfTransitions){
    cout << currIndex << ":"; fdd_printset(current);
    if ( (current & Pm) != bddfalse)
        cout << "*";
    cout << endl;

    for(auto& sigma:Sigma){
        if((sigma.second->getFSigma() & current) != bddfalse){
            bdd next = sigma.second->delta(current);
            if ( next != bddfalse ){
                numOfTransitions++;
                cout << currIndex << "-Via: " << sigma.first <<endl;
                auto it = find_if(visitedStates.begin(), visitedStates.end(),[&](auto& P){
                    return P == next;
                });
                int nextIndex = distance(visitedStates.begin(),it);

                if (it == visitedStates.end()){
                    visitedStates.push_back(next);
                    runUnderControlRec(next,visitedStates,nextIndex, numOfTransitions);
                }
                else {
                    cout << nextIndex << ": "; fdd_printset(next);cout << endl;
                }
            }
        }
    }
}
void TDS::print(){
    cout << *root;
}
void TDS::printControlData(string filePath){
    FILE* controlledBehavior;
    string directory = filePath + "/results/controlledBehavior.txt";
    controlledBehavior = fopen(directory.c_str(),"w");
    if (controlledBehavior == nullptr){
        cerr << "Error: controlledBehavior cannot be created!" << endl;
        abort();
    }
    fdd_fprintset(controlledBehavior,supC2P);
    fclose(controlledBehavior);
    for(auto& sigma: Sigma){
        if (sigma.second->isControllable()){
            FILE* sigmaEN;
            string tempDir = filePath + "/results/"+sigma.second->getName()+".txt";
            sigmaEN = fopen(tempDir.c_str(),"w");
            fdd_fprintset(sigmaEN, sigma.second->getFSigma());
            fclose(sigmaEN);
        }
    }

}

// ADS Supervisory Control File Output:
void TDS::printADSsupervisor(string filePath, string rootFile) {
	if(!initialSet)
	setInitials();
	vector<bdd> visitedStates;
	if ((P0 & supC2P) == bddfalse){
	cout << "Closed behavior is empty!" << endl;
	return;
	}
	visitedStates.push_back(P0);
	int numOfTransitions = 0;

	int maxIndex = 0;
	int highest_odd_event = -1;
	int highest_even_event = -1;
	map<string, int> eventsMap;
	vector<string> statesList;

	struct node {
		string fileName;
		struct node *next;
	};

	// Getting list of DES that will become ADS files:
	struct node* fileLinkedListHead = NULL;
	ifstream root;
	root.open(filePath + "/" + rootFile + ".txt");
	if(!root) {
		cerr << "Error: ADS file cannot be created!" << endl;
		abort();
	}
	string line;
	string tempLine = "";
	while(root >> line) {
		if(line == "xor") {
			struct node* tempNode = new node;
			tempNode->fileName = tempLine;
			tempNode->next = fileLinkedListHead;
			fileLinkedListHead = tempNode;
		}
		tempLine = line;
	}
	root.close();

	// Creating ADS file:
	FILE* adsFile;
	string directory = filePath + "/results/supervisor.ads";
	adsFile = fopen(directory.c_str(), "w");
	if(adsFile == nullptr) {
		cerr << "Error: ADS file cannot be created!" << endl;
		abort();
	}
	fputs((rootFile + "_Supervisor").c_str(), adsFile);

	// Finding events:
	while(fileLinkedListHead != NULL) {
		// Checking the file:
		string currFileName = fileLinkedListHead->fileName + ".txt";

		if(fileLinkedListHead->next != NULL) {
			struct node* toBeDeleted = fileLinkedListHead;
			fileLinkedListHead = fileLinkedListHead->next;
			free(toBeDeleted);
		}
		else fileLinkedListHead = NULL;

		FILE* currFile;
		string currFileDirectory = filePath + "/" + currFileName;
		currFile = fopen(currFileDirectory.c_str(), "r");
		if(currFile == nullptr) {
			cerr << "Error: ADS file cannot be created!" << endl;
			abort();
		}

		int c, i, size = 1024;
		char* buffer = (char *)malloc(size);

		bool first = true;
		unsigned int eventInt = 0;

		do {
			i = 0;
			do {
				c = fgetc(currFile);
				if(c != EOF) buffer[i++] = (char)c;
				if(i >= size - 1) {
					size += size;
					buffer = (char*)realloc(buffer, size);
				}
			} while(c != EOF && c != '\n');
			buffer[i] = 0;

			if(first) first = false;
			else {
				// Parsing the text files:
				string line(buffer);
				if(line.find("simple") != string::npos) {
					string stateName = line.substr(0, line.find(" "));
					if(find(statesList.begin(), statesList.end(), stateName) == statesList.end()) statesList.push_back(stateName);
				}
				else if(line.find("simple") == string::npos && line.find("xor") == string::npos && isdigit(line[0])) {
					bool valid = false;
					for(vector<string>::iterator it = statesList.begin(); it != statesList.end(); ++it) {
						if(line.find(*it) != string::npos) {
							valid = true;
							line = line.substr(line.find(*it) + (*it).length());
							while(line[0] == ' ') {
								line = line.substr(1);
							}
							line = line.substr(0, line.find(' '));

							
							// USE https://stackoverflow.com/questions/97050/stdmap-insert-or-stdmap-find INSTEAD
							if(line != "" && line != " " && line != "\n") {
								pair<map<string, int>::iterator, bool> existCheck = eventsMap.insert(pair<string, int>(line, eventInt));
								//if(eventsMap.find(line) == eventsMap.end()) {
								if(existCheck.second == false) {
									//eventsMap[line] = eventInt;
									eventInt++;
								}
							}
						}
					}
				}
			}	
		} while(c != EOF);
		fclose(currFile);
		free(buffer);

	}
	free(fileLinkedListHead);

	// Testing, remove later:
	fputs("\n", adsFile);
	for(map<string, int>::iterator it = eventsMap.begin(); it != eventsMap.end(); ++it) {
		fputs("*", adsFile);
		fputs((it->first).c_str(), adsFile);
		fputs("*", adsFile);
		fputs("\n", adsFile);
		//fprintf(adsFile, "%d", it->second);
		//fputs("\n", adsFile);
	}
	/******************/

	// max index + 1 as max number of states, pass in the vectors of strings to pass through for each section then return
	// printADSsupervisor_rec(P0, visitedStates, 0, numOfTransitions, maxIndex); // Make this return what you need
	// File manipulation here
	fclose(adsFile);
}

void TDS::printADSsupervisor_rec(bdd current, vector<bdd>& visitedStates, int currIndex, int& numOfTransitions, int maxIndex /*, stringMap, markerState*/) {
    /* cout << currIndex << ":"; fdd_printset(current);
    if ( (current & Pm) != bddfalse)
        cout << "*";
    cout << endl;

    for(auto& sigma:Sigma){
        if((sigma.second->getFSigma() & current) != bddfalse){
            bdd next = sigma.second->delta(current);
            if ( next != bddfalse ){
                numOfTransitions++;
                cout << currIndex << "-Via: " << sigma.first <<endl;
                auto it = find_if(visitedStates.begin(), visitedStates.end(),[&](auto& P){
                    return P == next;
                });
                int nextIndex = distance(visitedStates.begin(),it);

                if (it == visitedStates.end()){
                    visitedStates.push_back(next);
                    runUnderControlRec(next,visitedStates,nextIndex, numOfTransitions);
                }
                else {
                    cout << nextIndex << ": "; fdd_printset(next);cout << endl;
                }
            }
        }
    }*/
}

// ADS Input File Output:
void TDS::printADSinputs(string filePath, string rootFile) {
	/*FILE* adsFile;
	string directory = filePath + "/results/" + rootFile + ".ads";
	adsFile = fopen(directory.c_str(), "w");
	if(adsFile == nullptr) {
		cerr << "Error: ADS file cannot be created!" << endl;
		abort();
	}

	fputs(rootFile.c_str(), adsFile);

	fclose(adsFile);*/

	struct node {
		string fileName;
		struct node *next;
	};

	// Getting list of DES that will become ADS files:
	struct node* fileLinkedListHead = NULL;
	ifstream root;
	root.open(filePath + "/" + rootFile + ".txt");
	if(!root) {
		cerr << "Error: ADS file cannot be created!" << endl;
		abort();
	}
	string line;
	string tempLine = "";
	while(root >> line) {
		if(line == "xor") {
			struct node* tempNode = new node;
			tempNode->fileName = tempLine;
			tempNode->next = fileLinkedListHead;
			fileLinkedListHead = tempNode;
		}
		tempLine = line;
	}
	root.close();

	// Creating each ADS file:
	while(fileLinkedListHead != NULL) {
		// Checking the file:
		string currFileName = fileLinkedListHead->fileName + ".txt";

		if(fileLinkedListHead->next != NULL) {
			struct node* toBeDeleted = fileLinkedListHead;
			fileLinkedListHead = fileLinkedListHead->next;
			free(toBeDeleted);
		}
		else fileLinkedListHead = NULL;

		// Creating the ADS file:
		//cout << currFileName << endl;
		FILE* adsFile;
		string directory = filePath + "/results/" + currFileName.substr(0, currFileName.find(".")) + ".ads";
		adsFile = fopen(directory.c_str(), "w");
		if(adsFile == nullptr) {
			cerr << "Error: ADS file cannot be created!" << endl;
			abort();
		}
		fputs(currFileName.substr(0, currFileName.find(".")).c_str(), adsFile);

		FILE* currFile;
		string currFileDirectory = filePath + "/" + currFileName;
		currFile = fopen(currFileDirectory.c_str(), "r");
		if(currFile == nullptr) {
			cerr << "Error: ADS file cannot be created!" << endl;
			abort();
		}

		int c, i, size = 1024;
		char* buffer = (char *)malloc(size);

		unsigned int stateCount = 0;
		unsigned int transitionCount = 0;
		map<string, int> statesMap;
		map<string, int> transitionsMap;
		vector<int> markerStates;
		vector<string> transitionFunction;
		bool first = true;

		do {
			i = 0;
			do {
				c = fgetc(currFile);
				if(c != EOF) buffer[i++] = (char)c;
				if(i >= size - 1) {
					size += size;
					buffer = (char*)realloc(buffer, size);
				}
			} while(c != EOF && c != '\n');
			buffer[i] = 0;

			if(first) first = false;
			else {
				// Parsing the text files:
				string line(buffer);
				bool simpleCheck = line.find("simple") != string::npos;	
				if(simpleCheck) {
					// Count states and create dictionary of states
					string stateName = line.substr(0, line.find(" "));
					if(statesMap.find(stateName) == statesMap.end()) {
						statesMap[stateName] = stateCount;	
					
						string last = line.substr(line.find("simple"), line.find("  "));
						last = last.substr(last.find(" ") + 1, last.length() - 1);
						last = last.substr(last.find(" ") + 1, last.find(" ") + 1);
						last = last.substr(0, 1);
						if(last == "1") markerStates.push_back(stateCount);
						stateCount++;
					}
				} else if(!simpleCheck && line.find("xor") == string::npos && line.find("sync") == string::npos) {
					bool valid = false;
					for(map<string, int>::iterator it = statesMap.begin(); it != statesMap.end(); ++it) {
						if(line.find(it->first) != string::npos) {
							valid = true;
							break;
						}
					}
					if(valid) {
						// Create transition function
						string sourceState = line.substr(4, line.find(" ", 4));
						sourceState = sourceState.substr(0, sourceState.find(" "));
						string transitionString; // Contains string to output to ADS file
						ostringstream temp;
						temp << statesMap[sourceState];
						transitionString = temp.str() + " ";
						string transition = line.substr(4 + sourceState.length(), line.length() - 1);
						transition = transition.substr(transition.find_first_not_of(" "), transition.length() - 1);
						line = transition;
						transition = transition.substr(0, transition.find(" "));
						if(transitionsMap.find(transition) == transitionsMap.end()) {
							transitionsMap[transition] = transitionCount;
							transitionCount++;
						}
						temp.str("");
						temp.clear();
						temp << transitionsMap[transition];
						transitionString = transitionString + temp.str() + " ";
						line = line.substr(line.find_first_not_of(" ", transition.length()), line.length() - 1);
						line = line.substr(line.find(" ") + 1, line.length() - 1);
						line = line.substr(0, line.find("\n"));
						temp.str("");
						temp.clear();
						temp << statesMap[line];
						transitionString = transitionString + temp.str();
						transitionFunction.push_back(transitionString);
					}
				}
			}	
		} while(c != EOF);
		fclose(currFile);
		free(buffer);

		fputs("\n\nState size (State set will be (0,1....,size-1)):\n", adsFile);
		// <-- Enter state size, in range 0 to 2000000, on line below. 
		fprintf(adsFile, "%d", stateCount);
		
		fputs("\n\nMarker states:", adsFile);
		// <-- Enter marker states, one per line.
		// To mark all states, enter *.
		// If no marker states, leave line blank.
		// End marker list with blank line.
		for(unsigned int j = 0; j < markerStates.size(); j++) {
			fputs("\n", adsFile);
			fprintf(adsFile, "%d", markerStates.at(j));
		}

		fputs("\n\nVocal states:\n", adsFile);
		// <-- Enter vocal output states, one per line.
		// Format: State  Vocal_Output.  Vocal_Output in range 10 to 99.
		// Example: 0 10
		// If no vocal states, leave line blank.
		// End vocal list with blank line.

		fputs("\n\nTransitions:", adsFile);
		// <-- Enter transition triple, one per line.
		// Format: Exit_(Source)_State  Transition_Label  Entrance_(Target)_State.
		// Transition_Label in range 0 to 999.
		// Example: 2 0 1 (for transition labeled 0 from state 2 to state 1).
		for(unsigned int j = 0; j < transitionFunction.size(); j++) {
			fputs("\n", adsFile);
			fputs(transitionFunction.at(j).c_str(), adsFile);
		}
		
		fclose(adsFile);

		/*****************************/
		/*FILE* adsFile;
		string directory = filePath + "/results/" + rootFile + ".ads";
		adsFile = fopen(directory.c_str(), "w");
		if(adsFile == nullptr) {
			cerr << "Error: ADS file cannot be created!" << endl;
			abort();
		}

		fputs(rootFile.c_str(), adsFile);

		fclose(adsFile);*/
	}
	free(fileLinkedListHead);
}
