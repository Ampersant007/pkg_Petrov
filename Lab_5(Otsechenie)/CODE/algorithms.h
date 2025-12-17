#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include <vector>
#include <algorithm>
#include <map>
#include <set>
#include <cmath>

struct Point {
    double x, y;
    Point(double x = 0, double y = 0) : x(x), y(y) {}

    bool operator==(const Point& other) const {
        return std::abs(x - other.x) < 1e-10 && std::abs(y - other.y) < 1e-10;
    }

    bool operator!=(const Point& other) const {
        return !(*this == other);
    }

    bool operator<(const Point& other) const {
        if (std::abs(x - other.x) < 1e-10)
            return y < other.y - 1e-10;
        return x < other.x - 1e-10;
    }
};

struct Segment {
    Point p1, p2;
    Segment(Point p1 = Point(), Point p2 = Point()) : p1(p1), p2(p2) {}
};

namespace Algorithms {
bool liangBarskyClip(double x1, double y1, double x2, double y2,
                     double xmin, double ymin, double xmax, double ymax,
                     double &x1_clip, double &y1_clip, double &x2_clip, double &y2_clip);

bool cyrusBeckClip(const Segment& segment, const std::vector<Point>& polygon,
                   Segment& clippedSegment);
double dotProduct(const Point& a, const Point& b);
double crossProduct(const Point& a, const Point& b);
Point perpendicular(const Point& p);
bool isConvexPolygon(const std::vector<Point>& polygon);

std::vector<Point> makeCounterClockwise(const std::vector<Point>& polygon);
bool isPolygonCounterClockwise(const std::vector<Point>& polygon);

// Вспомогательные функции
bool isPointInsidePolygon(const Point& point, const std::vector<Point>& polygon);
bool doSegmentsIntersect(const Point& p1, const Point& p2, const Point& p3, const Point& p4);
Point findIntersectionPoint(const Point& p1, const Point& p2, const Point& p3, const Point& p4);
}

#endif // ALGORITHMS_H
