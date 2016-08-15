

// structs

struct triangle;
struct edge;

// functions

float area(in triangle t);
vec3 projectOntoPlane(in vec3 point, in vec3 plane_normal);
float distToAxis(in vec3 axis, in vec3 v);
vec3 closestPointOnSegment(in vec3 p, in vec3 a, in vec3 b);
float cylinderOverlap(in vec3 position0, in vec3 normal0, in float radius0, in vec3 position1, in vec3 normal1, in float radius1);
float sphereIntersectionDistance(in vec3 position0, in float radius0, in vec3 position1, in float radius1);
void closestPointsOnTwoSegments(inout vec3 c0, inout vec3 c1, in vec3 position0, in vec3 normal0, in vec3 position1, in vec3 normal1);
bool graftIntersectTriangle(in vec3 position, in vec3 normal, in triangle t, in float radius);
bool rayIntersectTriangle(inout vec3 point, inout float t, in vec3 rayStart, in vec3 rayDir, in triangle tri);

void getRotationFromTo(inout mat4 rot, in vec3 from, in vec3 to);
void getRotation(inout mat4 mat, in float angle, in vec3 axis);
