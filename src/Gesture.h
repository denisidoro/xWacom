#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <utility>
#include <math.h>
#include <cstdlib>

#include "ofxWacomFeelMultiTouch.h"
#include "ofxXmlSettings.h"

using namespace std;

class Gesture {

public:

	// Enums
	enum Move {UNKNOWN = -1, R = 0, UR, U, UL, L, DL, D, DR, P, E, T, H, CW, CCW};
	enum Area {ANY, RIGHT, TOP_RIGHT, TOP, TOP_LEFT, LEFT, BOTTOM_LEFT, BOTTOM, BOTTOM_RIGHT};
	enum Action {NOTHING = -1, SEND_INPUT = 0, AUTOHOTKEY, OPEN_FILE};
	
	void executeAction(Action action, string params);
	void executeInput(vector<int> keys);
	void executeAutohotkey(string code);

	// Constructor and destructor
	Gesture();
	~Gesture();

	// Main functions
	bool analyse(Finger *fingers, int count);
	void reset();
	string getMainMovementString();

private:

	string getForegroundFilename();

	// Settings
	float alpha, diagonalArc, zoneBoundaries[8];
	int multitouchTolerance, minimalIterationCount, minimalIterationFactor, minimalDistance;

	// Buffers
	Finger startingFinger;
	vector<Finger> history;
	vector<vector<Move>> movements;
	vector<Move> mainMovement;
	int iteration;
	float previousMeanDistance;
	void setHistory(Finger *fingers, int count);

	// String and interpration
	Move moveID(string command);
	vector<string> explode(string const & s, char delim);
	Move interpretLastMove();
	bool monitor();
	bool searchForMove(string xml, string mainMov);
	string moveToString(Move moveVector);

	// Setup
	void setZones(float alpha);

	// Math
	Move getZone(float theta);
	float distance(float x1, float y1, float x2, float y2);
	float distance(Finger f1, Finger f2);
	float historyMeanDistance();
	bool insideArea(Area a, Finger f);

};
