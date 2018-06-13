#ifndef ANGLE_H
#define ANGLE_H

#include "Utils.h"

double getAngle(Vec2f a, Vec2f b);
double slope(Vec2f a, Vec2f b);
double slopeFromAngle(double degree);

//Given two points on a line, finds the slope of a perpendicular line crossing it.
double inverseSlope(Vec2f a, Vec2f b);

//Given a slope and an offset, returns two points on that line.
Vec2f* pointsOnLine(double slope, Vec2f use, double distance);

Vec2f followAngle(double angle, Vec2f use, double distance);

//Using a line formed from point a to b, tells if point c is on S side of that line.
char isSideLine(Vec2f a, Vec2f b, Vec2f c); 

char angleIsWithin(double angle, Vec2f range);

int rangeToAngle(Vec2f range);

void computeAngleRanges(Node* cell1, Node* cell2);

double* getEdgeLinesFromPoint(Node* cell1, Node* cell2, double radius);

double* getAngleRange(Node* cell1, Node* cell2, double index, double radius);

#endif