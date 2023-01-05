#include <vector>
#import "point.h"

enum Child {
    lefttopfront = 0,
    righttopfront = 1,
    leftbottomfront = 2,
    rightbottomfront = 3,
    lefttopback = 4,
    righttopback = 5,
    leftbottomback = 6,
    rightbottomback = 7
};

template <typename PT>
struct Triangle {
    PT p1, p2, p3;
    Triangle(PT p1, PT p2, PT p3) : p1(p1), p2(p2), p3(p3) {}
};

struct Octree {
private:
    float xmin, ymin, zmin;
    float xmax, ymax, zmax;
    
public:
    std::vector<Triangle<int>> triangles;
    std::vector<int> points;
    std::vector<Pointf> &vertices;
    Octree *children[8]; // undefined if `divided` == false;
    bool divided = false;
        
    Octree(std::vector<Pointf> &vertices, Pointf min, Pointf max, int divisions = 0)
    : Octree(vertices, min.x, min.y, min.z, max.x, max.y, max.z, divisions) {
    }
    
    Octree(std::vector<Pointf> &vertices, float xmin, float ymin, float zmin, float xmax, float ymax, float zmax, int depth = 0)
    : vertices(vertices),
      xmin(xmin), ymin(ymin), zmin(zmin),
      xmax(xmax), ymax(ymax), zmax(zmax)
    {
        if (depth > 0) {
            divide(depth);
        }
    }
    
    Pointf min() { return Pointf(xmin, ymin, zmin); }
    Pointf max() { return Pointf(xmax, ymax, zmax); }
    Pointf center() { return Pointf((xmax+xmin)/2, (ymin+ymax)/2, (zmin+zmax)/2); }

    void divide();
    void divide(int depth);
    
    bool containsPoint(Pointf point);
    bool intersectsLineSegment(Pointf p1, Pointf p2);
    bool intersectsTriangle(Pointf p1, Pointf p2, Pointf p3);
    bool intersectsTriangle(Triangle<Pointf> &triangle);
    
    bool addTriangle(int i1, int i2, int i3);
    bool addTriangle(Triangle<int> &triangleIndices, Triangle<Pointf> &trianglePoints);
    
    bool addPoint(int index);
    bool addPoint(int index, Pointf point);
    
    void leafs(std::vector<Octree *> &result);
    std::vector<Octree *> leafs();
};

