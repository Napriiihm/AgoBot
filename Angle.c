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

int rangeToAngle(Vec2f range)
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

double* getEdgeLinesFromPoint(Node* cell1, Node* cell2, double radius)
{
	double* ret = malloc(6 * sizeof(double));

	Vec2 cell1Pos = NodetoVec2(cell1);
	Vec2 cell2Pos = NodetoVec2(cell2);

	char invert = 0;

	double tmpRadius = getDist(cell1Pos, cell2Pos);
	if(tmpRadius <= radius)
	{
		radius = tmpRadius - 5;
		invert = 1;
	}

	Vec2 d; d.x = cell2Pos.x - cell1Pos.x; d.y = cell2Pos.y - cell1Pos.y;
	double dd = sqrt(d.x * d.x + d.y * d.y);
	double a = asin(radius / dd);
	double b = atan2(d.y, d.x);

	double t = b - a;
	Vec2f ta; ta.x = radius * -sin(t); ta.y = radius * cos(t);

	t = b + a;
	Vec2f tb; tb.x = radius * -sin(t); tb.y = radius * cos(t);

	Vec2f arg1; arg1.x = cell2Pos.x + ta.x; arg1.y = cell2Pos.y + ta.y;
	double angleLeft = getAngle(arg1, Vec2toVec2f(cell1Pos));

	Vec2f arg2; arg2.x = cell2Pos.x + tb.x; arg2.y = cell2Pos.y + tb.y;
	double angleRight = getAngle(arg2, Vec2toVec2f(cell1Pos));

	double angleDistace = (int)(angleRight - angleLeft) % 360;

	memcpy(ret, &angleLeft, sizeof(double));
	memcpy(ret + sizeof(double), &angleDistace, sizeof(double));
	memcpy(ret + sizeof(double) * 2, &(arg2.x), sizeof(double));
	memcpy(ret + sizeof(double) * 3, &(arg2.y), sizeof(double));
	memcpy(ret + sizeof(double) * 4, &(arg1.x), sizeof(double));
	memcpy(ret + sizeof(double) * 5, &(arg1.y), sizeof(double));

	return ret;
}

double* getAngleRange(Node* cell1, Node* cell2, double index, double radius)
{
	double* angleStuff = getEdgeLinesFromPoint(cell1, cell2, radius);

	Vec2f arg; memcpy(&arg, angleStuff, 2 * sizeof(double));
	double leftAngle; memcpy(&leftAngle, angleStuff, sizeof(double));
	double rightAngle = (double)rangeToAngle(arg);
	double difference; memcpy(&difference, angleStuff + sizeof(double), sizeof(double));

	//drawPoint(angleStuff[2][0], angleStuff[2][1], 3, "");
    //drawPoint(angleStuff[3][0], angleStuff[3][1], 3, "");

    Vec2f lineLeft = followAngle(leftAngle, Vec2toVec2f(NodetoVec2(cell1)), 150 + cell1->size - index * 10);
    Vec2f lineRight = followAngle(rightAngle, Vec2toVec2f(NodetoVec2(cell1)), 150 + cell1->size - index * 10);

	/*
	if (blob2.isVirus()) {
	    drawLine(blob1.x, blob1.y, lineLeft[0], lineLeft[1], 6);
	    drawLine(blob1.x, blob1.y, lineRight[0], lineRight[1], 6);
	    drawArc(lineLeft[0], lineLeft[1], lineRight[0], lineRight[1], blob1.x, blob1.y, 6);
	} else if(getCells().hasOwnProperty(blob2.id)) {
	    drawLine(blob1.x, blob1.y, lineLeft[0], lineLeft[1], 0);
	    drawLine(blob1.x, blob1.y, lineRight[0], lineRight[1], 0);
	    drawArc(lineLeft[0], lineLeft[1], lineRight[0], lineRight[1], blob1.x, blob1.y, 0);
	} else {
	    drawLine(blob1.x, blob1.y, lineLeft[0], lineLeft[1], 3);
	    drawLine(blob1.x, blob1.y, lineRight[0], lineRight[1], 3);
	    drawArc(lineLeft[0], lineLeft[1], lineRight[0], lineRight[1], blob1.x, blob1.y, 3);
	}
	*/

	double* ret = malloc(2 * sizeof(double));
	memcpy(ret, &leftAngle, sizeof(double));
	memcpy(ret + sizeof(double), &leftAngle, sizeof(double));

	return ret;
}