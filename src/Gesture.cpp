#include "Gesture.h"
#include <iostream>
#include <Windows.h>
#include <Psapi.h>

#pragma comment(lib, "psapi.lib")


/* CONSTRUCTOR */

Gesture::Gesture() {

	ofxXmlSettings settings;
	settings.loadFile("settings.xml");

	multitouchTolerance = settings.getValue("settings:multitouchTolerance", 15);
	minimalIterationCount = settings.getValue("settings:minimalIterationCount", 12);
	minimalIterationFactor = settings.getValue("settings:minimalIterationFactor", 110);
	minimalDistance = settings.getValue("settings:minimalDistance", 16);
	diagonalArc = settings.getValue("settings:diagonalArc", 22);
	setZones(diagonalArc);
	reset();

}

Gesture::~Gesture() {}


string Gesture::getForegroundFilename() {

	string filename = "";
	
	HWND hwnd = GetForegroundWindow();
	DWORD result = 0;
	DWORD processID = 0;
	GetWindowThreadProcessId(hwnd, &processID);

    HMODULE hMods[1024];
    HANDLE hProcess;
    DWORD cbNeeded;
    unsigned int i;

    // Get a handle to the process.
    hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);

    if (NULL == hProcess)
        return false;

    TCHAR szModName[MAX_PATH];

    // Get the full path to the module's file.
    if (GetModuleFileNameEx(hProcess, NULL, szModName, sizeof(szModName) / sizeof(TCHAR))) {
		// Convert full filename to string
		std::wstring arr_w(szModName);
		std::string fullFilename(arr_w.begin(), arr_w.end());
		// Get executable name only
		vector<string> parts = explode(fullFilename, '\\');
		if (parts.size() > 0)
			filename = parts.back();
    }

    CloseHandle(hProcess);
	return filename;

}




/* MAIN */

bool Gesture::analyse(Finger *fingers, int count) {

	int historySize = history.size();

	if (iteration < minimalIterationCount * pow((float)minimalIterationFactor/100, historySize + 1)) {
		iteration++;
		return false;
	}

	bool minDistance = false;

	if (historySize > 0) {

		for (int i = 0; i < historySize && !minDistance; i++) {
			float dist = distance(fingers[i], history[i]);
			//WacomTrace("%d | %f, %f, %f, %f | %f \n", i, fingers[i].X, history[i].X, fingers[i].Y, history[i].Y, dist);
			if (dist > (float)minimalDistance/1000)
				minDistance = true;
		}

		if (!minDistance)
			return false;

	}

	//cout << "hS: " << historySize << endl;

	for (int i = 0; i < count && i < historySize; i++) {
		float theta = atan2(-fingers[i].pos.y + history[i].pos.y, fingers[i].pos.x - history[i].pos.x);
		Move zone = getZone(theta);
		if (movements[i].size() == 0 || zone != movements[i].back()) {
			movements[i].push_back(zone);
		}
	}

	setHistory(fingers, count);
	iteration = 0;

	if (monitor()) {
		reset();
	}

	return true;

}

void Gesture::reset() {
	history.clear();
	movements.clear();
	mainMovement.clear();
	previousMeanDistance = 0;
	iteration = 0;
	ofLogNotice() << "reset!";
}


/* MISC */

void Gesture::setHistory(Finger *fingers, int count) {

	//WacomTrace("(setHistory: %d, %d)\n", history.size(), count);
	//cout << "(" << fingers[0].X << ")" << endl;

	if (history.size() == 0 && count > 0)
		startingFinger = fingers[0];

	for (size_t i = 0; i < history.size(); i++) {
		if (i < (size_t)count)
			history[i] = fingers[i]; // update
		else {
			try { // remove
				//WacomTrace("removing %d, count %d", i, count);
				history.pop_back(); 
				movements.pop_back();
			}
			catch (out_of_range o) { cout << o.what() << endl; }
		}
	}

	for (int i = history.size(); i < count; i++) {
		try { // add
			//WacomTrace("add new finger!\n");
			history.push_back(fingers[i]);
			movements.push_back(vector<Move>());
			/*if (count > 1) {
				mainMovement.clear(); // DELETE MOVEMENT HISTORY WHEN ADDING A NEW FINGER
				previousMeanDistance = historyMeanDistance();
				iteration = minimalIterationCount - 2; // ANALYSE QUICKLY
				return;
			}*/
		}
		catch (out_of_range o) { cout << o.what() << endl; }
	}

	Move move = interpretLastMove();
	if (move != UNKNOWN && (mainMovement.size() == 0 || mainMovement.back() != move))
		mainMovement.push_back(move);

}

Gesture::Move Gesture::moveID(string command) {

	if (command == "R")			return R;
	else if (command == "UR")	return UR;
	else if (command == "U")	return U;
	else if (command == "UL")	return UL;
	else if (command == "L")	return L;
	else if (command == "DL")	return DL;
	else if (command == "D")	return D;
	else if (command == "DR")	return DR;
	else if (command == "P")	return P;
	else if (command == "E")	return E;
	else if (command == "T")	return T;
	else if (command == "H")	return H;
	else if (command == "CW")	return CW;
	else if (command == "CCW")	return CCW;
	else						return UNKNOWN;

}

bool Gesture::searchForMove(string xml, string mainMov) {

	ofxXmlSettings actions;

	//ofLogNotice() << "trying to open " << xml;
	
	if (actions.loadFile(xml)) {
		actions.pushTag("actions");
		int actionCount = actions.getNumTags("action");
		//ofLogNotice() << "actionCount: " << actionCount;
		for (int i = 0; i < actionCount; i++) {
			actions.pushTag("action", i);
			string move = actions.getValue("m", "");
			//ofLogNotice() << "found move: " << move;
			//ofLogNotice() << "move: (" << move << ", " << mainMov << ")\ninsideArea: " << insideArea(static_cast<Area>(actions.getValue("s", 0)), startingFinger) << "\ncount: (" << actions.getValue("c", 1) << ", " << history.size() << ")";
			if (move == mainMov && 
			insideArea(static_cast<Area>(actions.getValue("s", 0)), startingFinger) &&
			actions.getValue("c", 1) == history.size()) {
				ofLogNotice() << "ACTION FOUND";
				executeAction(static_cast<Action>(actions.getValue("a", -1)), actions.getValue("p", ""));
				reset(); // why is it necessary?
				return true;
			}
			actions.popTag();
		}
		actions.popTag();
	}

	return false;

}

string Gesture::moveToString(Move move) {

	switch (move) {
		case R:		return "R";
		case UR:	return "UR";
		case U:		return "U";
		case UL:	return "UL";
		case L:		return "L";
		case DL:	return "DL";
		case D:		return "D";
		case DR:	return "DR";
		case P:		return "P";
		case E:		return "E";
		case T:		return "T";
		case H:		return "H";
		case CW:	return "CW";
		case CCW:	return "CCW";
	}
	
	return "";

}

string Gesture::getMainMovementString() {

	string s = "";

	for (size_t i = 0; i < mainMovement.size(); i++) {
		string move = moveToString(mainMovement[i]);
		if (move != "")
			s += move + (i == mainMovement.size() - 1 ? "" : "_");
	}
	
	return s;

}

bool Gesture::monitor() {

	string mainMov = getMainMovementString();
	string foregroundFilename = getForegroundFilename();
	bool foundMove = searchForMove(foregroundFilename + ".xml", mainMov);

	if (!foundMove)
		searchForMove("global.xml", mainMov);

	return foundMove;

}

vector<string> Gesture::explode(string const & s, char delim) {
	
	vector<string> result;
	istringstream iss(s);

	for (string token; getline(iss, token, delim);)
	{
		result.push_back(move(token));
	}

	return result;

}

Gesture::Move Gesture::interpretLastMove() {

	vector<Move> singleMoves;
	size_t count = movements.size();

	if (count == 0)
		return UNKNOWN;

	for (size_t i = 0; i < count; i++) {
		if (movements[i].size() > 0)
			singleMoves.push_back(movements[i].back());
	}

	if (count == 1 && singleMoves.size() > 0) {
		return singleMoves.back();
	}

	else {

		float dist = historyMeanDistance();
		Move move = UNKNOWN;

		//ofLogNotice() << "multidistance: " << dist << ", " << previousMeanDistance;

		if (previousMeanDistance > 0) {
			if (dist > previousMeanDistance * (1 + (float)multitouchTolerance / 100))
				move = E;
			else if (dist < previousMeanDistance * (1 - (float)multitouchTolerance / 100))
				move = P;
			else if (singleMoves.size() > 0)
				move = singleMoves.back();
		}

		previousMeanDistance = dist;
		return move;

	}

}


void Gesture::executeAction(Action action, string params) {

	switch (action) {
		
		case AUTOHOTKEY:
			executeAutohotkey(params);
		break;

		case SEND_INPUT:
			vector<int> iKeys;
			size_t *e;
			vector<string> sKeys = explode(params, ',');
			for (size_t i = 0; i < sKeys.size(); i++)
				iKeys.push_back(std::stol(sKeys[i], e, 16));
			executeInput(iKeys);
		break;

	}

}

void Gesture::executeInput(vector<int> keys) {
	
    // This structure will be used to create the keyboard
    // input event.
    INPUT ip;
 
    // Set up a generic keyboard event.
    ip.type = INPUT_KEYBOARD;
    ip.ki.wScan = 0; // hardware scan code for key
    ip.ki.time = 0;
    ip.ki.dwExtraInfo = 0;
	
	for (size_t i = 0; i < keys.size(); i++) {
		ip.ki.wVk = keys[i]; // virtual-key code
		ip.ki.dwFlags = 0; // 0 for key press
		SendInput(1, &ip, sizeof(INPUT));
		Sleep(10);
	}

	for (size_t i = keys.size() - 1; i >= 0; i--) {
		ip.ki.wVk = keys[i]; // virtual-key code
		ip.ki.dwFlags = KEYEVENTF_KEYUP; // 0 for key press
		SendInput(1, &ip, sizeof(INPUT));
		Sleep(10);
	}
 
    // Press the "A" key
    ip.ki.wVk = 0x5B; // virtual-key code for the "a" key //0x5B
    ip.ki.dwFlags = 0; // 0 for key press
    SendInput(1, &ip, sizeof(INPUT));
 
    // Release the "A" key
    ip.ki.dwFlags = KEYEVENTF_KEYUP; // KEYEVENTF_KEYUP for key release
    SendInput(1, &ip, sizeof(INPUT));

}



/* SETUP */

void Gesture::setZones(float alpha) {

	for (int i = 0; i < 8; i++)
		zoneBoundaries[i] = (float) (-45 + 90 * ((i + 1) / 2) + (alpha / 2) * pow(-1, i));

}



/* MATH */

Gesture::Move Gesture::getZone(float theta) { // radians

	theta *= (float) (180 / 3.1415);
	if (theta < 0)
		theta += 360;

	while (theta >= 360 + zoneBoundaries[0])
		theta -= 360;

	for (int i = 7; i >= 0; i--) {
		if (theta >= zoneBoundaries[i])
			return static_cast<Move>(i);
	}

	return UNKNOWN;

}

float Gesture::distance(float x1, float y1, float x2, float y2) {
	return (float)sqrt(pow(x1 - x2, 2) + pow(y1 - y2, 2));
}

float Gesture::distance(Finger f1, Finger f2) {
	return distance(f1.pos.x, f1.pos.y, f2.pos.x, f2.pos.y);
}

float Gesture::historyMeanDistance() {

	float dist = 0;
	int count = 0;
	
	for (size_t i = 0; i < history.size() - 1; i++) {
		dist += distance(history[i], history[i + 1]);
		count++;
	}
	
	return dist / count;

}

bool Gesture::insideArea(Area a, Finger f) {

	ofVec2f p1, p2;

	switch (a) {
		case ANY: return true; 
		case RIGHT:			p1.set(.5, 0);	p2.set(1, 1);	break;
		case TOP_RIGHT:		p1.set(.5, 0);	p2.set(1., .5); break;
		case TOP:			p1.set(0, 0);	p2.set(1, .5);	break;
		case TOP_LEFT:		p1.set(0, 0);	p2.set(.5, .5); break;
		case LEFT:			p1.set(0, 0);	p2.set(.5, 1);	break;
		case BOTTOM_LEFT:	p1.set(0, 0);	p2.set(.5, .5); break;
		case BOTTOM:		p1.set(0, .5);	p2.set(1, 1);	break;
		case BOTTOM_RIGHT:	p1.set(.5, .5); p2.set(1, 1);	break;
		default: return false;
	}

	return (f.pos.x >= p1.x && f.pos.x <= p2.x && f.pos.y >= p1.y && f.pos.y <= p2.y); 

}

void Gesture::executeAutohotkey(string code) {

	ofxXmlSettings settings;
	settings.loadFile("settings.xml");

	string ahkLoc = settings.getValue("settings:autohotkeyLocation", "");
	string enzymeLoc = settings.getValue("settings:enzymeLocation", "");
	string enzymeLocWithParams = enzymeLoc + " \"" + code + "\"";

	//ofLogNotice() << "ahkloc: " << ahkLoc << ", " << ahkLoc.c_str() << ", enzymeLoc: " << enzymeLoc << ", " << enzymeLoc.c_str();

	ShellExecuteA(GetDesktopWindow(), "open", ahkLoc.c_str(), enzymeLocWithParams.c_str(), NULL, SW_SHOW);

}