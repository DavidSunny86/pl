#pragma once

#include "plCommon.h"

class plVector3;
class plVector4;
class plMatrix44;
class plTriangle;
class plPolygon;
class plIntersection;

namespace plMath
{

    plVector3 projectVectorOnPlane(
        const plVector3 &vector,
        const plVector3 &plane_normal);

    plVector3 closestPointOnSegment(
        const plVector3 &point,
        const plVector3 &a,
        const plVector3 &b);

    plVector3 closestPointOnLine(
        const plVector3 &point,
        const plVector3 &lineOrigin,
        const plVector3& lineDirection);

    bool closestPointsBetweenSegments(
        const plVector3 &e1Pt1,
        const plVector3 &e1Pt2,
        const plVector3 &e2Pt1,
        const plVector3 &e2Pt2,
        plVector3& closestPointEdge1,
        plVector3& closestPointEdge2,
        float32_t& distanceBetweenLines);

    plVector3 closestPointOnPlane(
        const plVector3 &linDir,
        const plVector3 &linPt,
        const plVector3 &plnN,
        const plVector3 &plnPt);

    bool intersectTwoLines(
        const plVector3 &edge1Pt1,
        const plVector3 &edge1Pt2,
        const plVector3 &edge2Pt1,
        const plVector3 &edge2Pt2,
        float32_t &edge1Param,
        float32_t &edge2Param);

    float32_t fsqrt(float32_t x);
    float32_t clamp(float32_t val, float32_t min = -1, float32_t max = 1);

    void convexPolysToTris(
        std::vector<plTriangle> &tris,
        const std::vector<plPolygon> &polys);

    void concavePolysToTris(
        std::vector<plTriangle> &tris,
        const std::vector<plPolygon> &polys);

    plIntersection rayIntersect(
        const std::vector<plTriangle>& triangles,
        const plVector3 &rayOrigin,
        const plVector3 &rayDirection,
        bool smoothNormal = false,
        bool ignoreBehindRay = false,
        bool backFaceCull = false);

    plIntersection rayIntersect(
        const plVector3 &rayOrigin,
        const plVector3 &rayDirection,
        const plVector3 &planePoint,
        const plVector3 &planeNormal);

    plVector3 getAverageNormal(
        const std::vector<plTriangle>& triangles,
        float32_t radius,
        const plVector3 &origin,
        const plVector3 &normal);

    plIntersection getClosestPointToRay(
        const std::vector<plTriangle>& triangles,
        const plVector3 &rayOrigin,
        const plVector3 &rayDirection);

}
