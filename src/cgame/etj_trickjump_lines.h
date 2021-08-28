/*
 * MIT License
 * 
 * Copyright (c) 2020 ETJump team <zero@etjump.com>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef TRICKJUMP_LINES_HPP
#define TRICKJUMP_LINES_HPP

#include <string>
#include <vector>
#include <array>
#include "etj_rotation_matrix.h"
#include <map>

enum routeStatus{
	map,
	load,
	record
};


class TrickjumpLines
{

public:
	static const unsigned LINE_WIDTH = 8;

	typedef float vec_t;
	typedef unsigned char vec_c;
	typedef vec_t vec3_t[3];
	typedef vec_t vec4_t[4];
	typedef vec_c vec4_c[4];

	std::map<std::string, std::vector<unsigned char>> colorMap;

	struct Node
	{
		vec3_t coor;
		float speed;
		//bool isCrouch; // TODO: Add for ghost
		//bool isProne; // TODO: Add for ghost
	};

	struct Route
	{
		std::string name;		
		std::vector< std::vector< Node > > trails;
		vec4_c color;
		float width;
		routeStatus status;
		std::string filename;
	};

	TrickjumpLines();
	~TrickjumpLines();

	/**
	 * Starts tjl recording
	 * @param name Name of the tjl file. Can be nullptr -> automatically generated filename
	 */
	void record(const char *name);
	/**
	 * Stops tjl recording and saves the current tjl
	 */
	void stopRecord();
	/**
	 * Adds current position to the currently recorded tjl (if we're recording)
	 * @param pos Current position
	 */
	void addPosition(vec3_t pos);
	/**
	 * Displays the latest recorded route
	 */
	void displayCurrentRoute(int x);
	
	bool isRecording() { return _recording; }

	bool isDebug() { return _debugVerbose; }
	int countRoute() { return _routes.size(); }	

	void toggleRoutes(bool state);
	void toggleMarker(bool state);

	bool isEnableLine();
	bool isEnableMarker();
	
	void listRoutes();
	void displayByName(const char *name);
	void displayNearestRoutes();
	void renameRoute(const char *oldName, const char *newName);
	void deleteRoute (const char *name);

	int getRoutePositionByName(const char *name);

	void overwriteRecording(const char *name);

	void saveRoutes(const char *savename);
	void loadRoutes(const char *loadname);
	bool loadedRoutes(const char *loadname);

	int getCurrentRouteToRender(){ return _currentRouteToRender; }
	void setCurrentRouteToRender(int nb){ _currentRouteToRender = nb; }

	bool getEnableLine(){ return _enableLine; }
	void setEnableLine(bool state){ _enableLine = state; }

	bool getEnableMarker(){ return _enableMarker; }
	void setEnableMarker(bool state){ _enableMarker = state; }
		
private:

	// Private function.
	unsigned long gcd_ui(unsigned long x, unsigned long y);
	unsigned long binomial(unsigned long n, unsigned long k);
	void addTrickjumpRecursiveBezier(std::vector< Node > points, vec4_c color, float width, int nbDivision);

	void draw4VertexLine(vec3_t start, vec3_t end, float width, vec4_c color);
	void draw4VertexLine2Color(vec3_t start, vec3_t end, float width, vec4_c colorStart, vec4_c colorEnd);

	void addTrickjumpLines(std::vector< Node > points, vec4_c color, float width);
	void addTrickjumpLinesColor(std::vector< Node > points, float minSpeed, float maxSpeed, float width);

	void addJumpIndicator(vec3_t point, vec4_c color, float quadSize);	
	
	float normalizeSpeed(float max, float min, float speed);
	void computeHSV(float speed, vec3_t& hsv);
	void hsv2rgb(vec3_t& hsv, vec3_t& rgb);
	void computeColorForNode(float max, float min, float speed, vec3_t& color);

	// Private variable
	bool _recording;	
	bool _enableLine;
	bool _enableMarker;
	bool _jumpRelease;
	bool _debugVerbose;

	Route _currentRoute;
	std::vector<Route> _routes;
	std::vector<Node> _currentTrail;
	unsigned _nextRecording;
	int _nextAddTime;
	int _currentRouteToRender;
	RotationMatrix _currentRotation;

	// Private inline function.
	float euclideanDist(const vec3_t a, const vec3_t b)
	{
		float sum = 0;

		for (int i = 0; i < 3; i++)
			sum += (a[i] - b[i])*(a[i] - b[i]);

		return std::sqrt(sum);
	}	
};
#endif
