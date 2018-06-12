#include "Angle.h"

double getAngle(Vec2f a, Vec2f b)
{
	if(a.x == b.x)
		return a.y < b.y ? 271.f : 89.f;
	
	return floor(atan2(-(a.y - b.y), -(a.x - b.x)) / M_PI * 180.f + 180.f);
}

double slope(Vec2f a, Vec2f b)
{
	return (a.y - b.y) / (a.x - b.x);
}

double slopeFromAngle(double degree)
{
	if(degree == 270)
		degree = 271;
	else if(degree == 90)
		degree = 91;

	return tan((degree - 180) / 180 * M_PI);
}

double inverseSlope(Vec2f a, Vec2f b)
{
	double slop = slope(a, b);

	return (-1)/slop;
}

Vec2f* pointsOnLine(double slope, Vec2f use, double distance)
{
	Vec2f* ret = malloc(2 * sizeof(Vec2f));

	double b = use.y - slope * use.x;
	double r = sqrt(1 + slope * slope);

	Vec2f new1, new2;
	new1.x = use.x + distance / r;
	new1.y = use.y + distance * slope / r;
	new2.x = use.x + (-distance) / r;
	new2.y = use.y + (-distance) * slope / r;

	memcpy(ret, &new1, sizeof(Vec2f));
	memcpy(ret + sizeof(Vec2f), &new2, sizeof(Vec2f));

	return ret;
}

Vec2f followAngle(double angle, Vec2f use, double distance)
{
	double slope = slopeFromAngle(angle);
	Vec2f* coords = pointsOnLine(slope, use, distance);

	if((int)(angle - 90) % 360 < 180)
		return *coords;
	return *(coords + sizeof(Vec2f));
}

char isSideLine(Vec2f a, Vec2f b, Vec2f c)
{
	return ((b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x) > 0);
}

char angleIsWithin(double angle, Vec2f range)
{
	double diff = (int)(rangeToAngle(range) - angle) % 360;

	return (diff >= 0 && diff <= range.y);
}

double rangeToAngle(Vec2f range)
{
	return (int)(range.x + range.y) % 360;
}

void computeAngleRanges(Node* cell1, Node* cell2)
{
	Vec2 cell1Pos, cell2Pos;
	memcpy(cell1 + 4, &cell1Pos, sizeof(Vec2));
	memcpy(cell2 + 4, &cell2Pos, sizeof(Vec2));

	double mainAngle = getAngle(Vec2toVec2f(cell1Pos), Vec2toVec2f(cell2Pos));
	double leftAngle = (int)(mainAngle - 90) % 360;
	double rightAngle = ((int)mainAngle + 90) % 360;

	Vec2f cell1Left = followAngle(leftAngle, Vec2toVec2f(cell1Pos), (double)cell1->size);
	Vec2f cell1Right = followAngle(rightAngle, Vec2toVec2f(cell1Pos), (double)cell1->size);

	Vec2f cell2Left = followAngle(rightAngle, Vec2toVec2f(cell2Pos), (double)cell2->size);
	Vec2f cell2Right = followAngle(leftAngle, Vec2toVec2f(cell2Pos), (double)cell2->size);

	double cell1AngleLeft = getAngle(Vec2toVec2f(cell2Pos), cell1Left);
	double cell1AngleRight = getAngle(Vec2toVec2f(cell2Pos), cell1Right);

	double cell2AngleLeft = getAngle(Vec2toVec2f(cell1Pos), cell2Left);
	double cell2AngleRight = getAngle(Vec2toVec2f(cell1Pos), cell2Right);

	double cell1range = (int)(cell1AngleRight - cell1AngleLeft) % 360;
	double cell2range = (int)(cell2AngleRight - cell2AngleLeft) % 360;

	/*
	if ((cell1range / cell2range) > 1) 
	{
        drawPoint(cell1Left.x, cell1Left.y, 3, "");
        drawPoint(cell1Right.x, cell1Right.y, 3, "");
        drawPoint(blob1.x, blob1.y, 3, "" + cell1range + ", " + cell2range + " R: " + (Math.round((cell1range / cell2range) * 1000) / 1000));
    }
	*/
}