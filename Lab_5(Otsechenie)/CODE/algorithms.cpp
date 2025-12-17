// algorithms.cpp
#include "algorithms.h"
#include <cmath>
#include <iostream>
#include <set>
#include <map>
#include <algorithm>
#include <vector>
#include <limits>
#include <cassert>

static const double EPS = 1e-9;

double Algorithms::crossProduct(const Point& a, const Point& b)
{
    return a.x * b.y - a.y * b.x;
}

double Algorithms::dotProduct(const Point& a, const Point& b)
{
    return a.x * b.x + a.y * b.y;
}

Point Algorithms::perpendicular(const Point& p)
{
    return Point(-p.y, p.x);
}

bool Algorithms::isConvexPolygon(const std::vector<Point>& polygon)
{
    if (polygon.size() < 3) return false;

    int n = polygon.size();
    int sign = 0;

    for (int i = 0; i < n; i++) {
        Point p1 = polygon[i];
        Point p2 = polygon[(i + 1) % n];
        Point p3 = polygon[(i + 2) % n];

        double cross = (p2.x - p1.x) * (p3.y - p2.y) - (p2.y - p1.y) * (p3.x - p2.x);

        if (std::abs(cross) > 1e-12) {
            if (sign == 0) {
                sign = (cross > 0) ? 1 : -1;
            } else if (sign * cross < 0) {
                return false;
            }
        }
    }

    return true;
}

std::vector<Point> Algorithms::makeCounterClockwise(const std::vector<Point>& polygon)
{
    if (polygon.size() < 3) return polygon;

    double area = 0.0;
    int n = polygon.size();

    for (int i = 0; i < n; i++) {
        Point p1 = polygon[i];
        Point p2 = polygon[(i + 1) % n];
        area += (p1.x * p2.y - p2.x * p1.y);
    }

    area /= 2.0;

    if (area < 0) {
        std::vector<Point> reversed = polygon;
        std::reverse(reversed.begin(), reversed.end());
        return reversed;
    }

    return polygon;
}

bool Algorithms::isPolygonCounterClockwise(const std::vector<Point>& polygon)
{
    if (polygon.size() < 3) return true;

    double area = 0.0;
    int n = polygon.size();

    for (int i = 0; i < n; i++) {
        Point p1 = polygon[i];
        Point p2 = polygon[(i + 1) % n];
        area += (p1.x * p2.y - p2.x * p1.y);
    }

    area /= 2.0;
    return area > 0;
}

// ============================================================================
// Liang-Barsky (axis-aligned rectangle clipping)
// ============================================================================
bool Algorithms::liangBarskyClip(double x1, double y1, double x2, double y2,
                                 double xmin, double ymin, double xmax, double ymax,
                                 double &x1_clip, double &y1_clip, double &x2_clip, double &y2_clip)
{
    double p[4], q[4];
    double t1 = 0.0, t2 = 1.0;

    double dx = x2 - x1;
    double dy = y2 - y1;

    p[0] = -dx; q[0] = x1 - xmin;
    p[1] = dx;  q[1] = xmax - x1;
    p[2] = -dy; q[2] = y1 - ymin;
    p[3] = dy;  q[3] = ymax - y1;

    for (int i = 0; i < 4; i++) {
        if (std::abs(p[i]) < 1e-12) {
            if (q[i] < 0) return false;
        } else if (p[i] > 0) {
            t2 = std::min(t2, q[i] / p[i]);
        } else {
            t1 = std::max(t1, q[i] / p[i]);
        }
    }

    if (t1 > t2) return false;

    x1_clip = x1 + t1 * dx;
    y1_clip = y1 + t1 * dy;
    x2_clip = x1 + t2 * dx;
    y2_clip = y1 + t2 * dy;

    return true;
}

// ============================================================================
// Cyrus-Beck (convex polygon clipping)
// ============================================================================
bool Algorithms::cyrusBeckClip(const Segment& segment, const std::vector<Point>& polygon,
                               Segment& clippedSegment)
{
    if (polygon.size() < 3) return false;

    double tE = 0.0;
    double tL = 1.0;

    Point AB(segment.p2.x - segment.p1.x, segment.p2.y - segment.p1.y);

    for (size_t i = 0; i < polygon.size(); i++) {
        Point Ci = polygon[i];
        Point Ci1 = polygon[(i + 1) % polygon.size()];
        Point CiCi1(Ci1.x - Ci.x, Ci1.y - Ci.y);

        Point ACi(Ci.x - segment.p1.x, Ci.y - segment.p1.y);

        // Векторное произведение AB × CiCi1
        double crossAB = AB.x * CiCi1.y - AB.y * CiCi1.x;
        // Векторное произведение ACi × CiCi1
        double crossAC = ACi.x * CiCi1.y - ACi.y * CiCi1.x;

        if (std::abs(crossAB) < 1e-10) {
            // Параллельный случай: если точка Ci лежит "снаружи" — отрезок полностью вне
            if (crossAC < 0) return false;
            continue;
        }

        double t = crossAC / crossAB;

        if (crossAB < 0) {
            // Входная точка
            if (t > tE) tE = t;
        } else {
            // Выходная точка
            if (t < tL) tL = t;
        }

        if (tE > tL) return false;
    }

    if (tE <= tL && tE <= 1.0 && tL >= 0.0) {
        double t1 = std::max(0.0, tE);
        double t2 = std::min(1.0, tL);
        clippedSegment.p1.x = segment.p1.x + t1 * AB.x;
        clippedSegment.p1.y = segment.p1.y + t1 * AB.y;
        clippedSegment.p2.x = segment.p1.x + t2 * AB.x;
        clippedSegment.p2.y = segment.p1.y + t2 * AB.y;
        return true;
    }

    return false;
}

// ============================================================================
// Вспомогательные функции (общие)
// ============================================================================
bool Algorithms::isPointInsidePolygon(const Point& point, const std::vector<Point>& polygon) {
    if (polygon.size() < 3) return false;

    int windingNumber = 0;
    int n = polygon.size();

    for (int i = 0; i < n; i++) {
        Point p1 = polygon[i];
        Point p2 = polygon[(i + 1) % n];

        if (p1.y <= point.y) {
            if (p2.y > point.y) {
                if (crossProduct(Point(p1.x - point.x, p1.y - point.y),
                                 Point(p2.x - point.x, p2.y - point.y)) > 0) {
                    windingNumber++;
                }
            }
        } else {
            if (p2.y <= point.y) {
                if (crossProduct(Point(p1.x - point.x, p1.y - point.y),
                                 Point(p2.x - point.x, p2.y - point.y)) < 0) {
                    windingNumber--;
                }
            }
        }
    }

    return windingNumber != 0;
}

bool Algorithms::doSegmentsIntersect(const Point& p1, const Point& p2, const Point& p3, const Point& p4) {
    double d1 = crossProduct(Point(p3.x - p1.x, p3.y - p1.y), Point(p4.x - p1.x, p4.y - p1.y));
    double d2 = crossProduct(Point(p3.x - p2.x, p3.y - p2.y), Point(p4.x - p2.x, p4.y - p2.y));
    double d3 = crossProduct(Point(p1.x - p3.x, p1.y - p3.y), Point(p2.x - p3.x, p2.y - p3.y));
    double d4 = crossProduct(Point(p1.x - p4.x, p1.y - p4.y), Point(p2.x - p4.x, p2.y - p4.y));

    if (d1 * d2 < 0 && d3 * d4 < 0) return true;

    // Проверка коллинеарных случаев
    if (std::abs(d1) < 1e-10 && (p3.x >= std::min(p1.x, p2.x) - 1e-10 && p3.x <= std::max(p1.x, p2.x) + 1e-10 &&
                                 p3.y >= std::min(p1.y, p2.y) - 1e-10 && p3.y <= std::max(p1.y, p2.y) + 1e-10)) return true;
    if (std::abs(d2) < 1e-10 && (p4.x >= std::min(p1.x, p2.x) - 1e-10 && p4.x <= std::max(p1.x, p2.x) + 1e-10 &&
                                 p4.y >= std::min(p1.y, p2.y) - 1e-10 && p4.y <= std::max(p1.y, p2.y) + 1e-10)) return true;
    if (std::abs(d3) < 1e-10 && (p1.x >= std::min(p3.x, p4.x) - 1e-10 && p1.x <= std::max(p3.x, p4.x) + 1e-10 &&
                                 p1.y >= std::min(p3.y, p4.y) - 1e-10 && p1.y <= std::max(p3.y, p4.y) + 1e-10)) return true;
    if (std::abs(d4) < 1e-10 && (p2.x >= std::min(p3.x, p4.x) - 1e-10 && p2.x <= std::max(p3.x, p4.x) + 1e-10 &&
                                 p2.y >= std::min(p3.y, p4.y) - 1e-10 && p2.y <= std::max(p3.y, p4.y) + 1e-10)) return true;

    return false;
}

Point Algorithms::findIntersectionPoint(const Point& p1, const Point& p2, const Point& p3, const Point& p4) {
    double A1 = p2.y - p1.y;
    double B1 = p1.x - p2.x;
    double C1 = A1 * p1.x + B1 * p1.y;

    double A2 = p4.y - p3.y;
    double B2 = p3.x - p4.x;
    double C2 = A2 * p3.x + B2 * p3.y;

    double det = A1 * B2 - A2 * B1;

    if (std::abs(det) < 1e-10) {
        // Параллельные отрезки - возвращаем середину перекрывающейся части
        double minx = std::max(std::min(p1.x, p2.x), std::min(p3.x, p4.x));
        double maxx = std::min(std::max(p1.x, p2.x), std::max(p3.x, p4.x));
        double miny = std::max(std::min(p1.y, p2.y), std::min(p3.y, p4.y));
        double maxy = std::min(std::max(p1.y, p2.y), std::max(p3.y, p4.y));
        return Point((minx + maxx) / 2.0, (miny + maxy) / 2.0);
    }

    double x = (B2 * C1 - B1 * C2) / det;
    double y = (A1 * C2 - A2 * C1) / det;

    return Point(x, y);
}
