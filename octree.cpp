#include "octree.h"
#include "point.h"

void Octree::divide() {
    divide(1);
}

void Octree::divide(int depth) {
    float xmid = (xmin + xmax) / 2;
    float ymid = (ymin + ymax) / 2;
    float zmid = (zmin + zmax) / 2;
    children[lefttopfront] = new Octree(vertices, xmin, ymid, zmin, xmid, ymax, zmid, depth - 1);
    children[righttopfront] = new Octree(vertices, xmid, ymid, zmin, xmax, ymax, zmid, depth - 1);
    children[leftbottomfront] = new Octree(vertices, xmin, ymin, zmin, xmid, ymid, zmid, depth - 1);
    children[rightbottomfront] = new Octree(vertices, xmid, ymin, zmin, xmax, ymid, zmid, depth - 1);
    children[lefttopback] = new Octree(vertices, xmin, ymid, zmid, xmid, ymax, zmax, depth - 1);
    children[righttopback] = new Octree(vertices, xmid, ymid, zmid, xmax, ymax, zmax, depth - 1);
    children[leftbottomback] = new Octree(vertices, xmin, ymin, zmid, xmid, ymid, zmax, depth - 1);
    children[rightbottomback] = new Octree(vertices, xmid, ymin, zmid, xmax, ymid, zmax, depth - 1);
    divided = true;
}

bool Octree::containsPoint(Pointf point) {
    return point.x >= xmin && point.y >= ymin && point.z >= zmin &&
           point.x <= xmax && point.y <= ymax && point.z <= zmax;
}

bool Octree::intersectsLineSegment(Pointf p1, Pointf p2) {
    float minx = std::min(p1.x, p2.x);
    float maxx = std::max(p1.x, p2.x);
    if (minx > xmax || maxx < xmin) return false;
    float miny = std::min(p1.y, p2.y);
    float maxy = std::max(p1.y, p2.y);
    if (miny > ymax || maxy < ymin) return false;
    float minz = std::min(p1.z, p2.z);
    float maxz = std::max(p1.z, p2.z);
    if (minz > zmax || maxz < zmin) return false;
    if (minx == maxx && miny == maxy && minz == maxz) return false;
    if (minx == maxx && miny == maxy) return minz < zmax && maxz > zmin;
    if (minx == maxx && minz == maxz) return miny < ymax && maxy > ymin;
    if (miny == maxy && minz == maxz) return minx < xmax && maxx > xmin;
    return true;
}

bool Octree::intersectsTriangle(Pointf p1, Pointf p2, Pointf p3) {
    // Check if all points of the triangle are inside this node
    if (p1.x >= xmin && p1.y >= ymin && p1.z >= zmin &&
        p1.x <= xmax && p1.y <= ymax && p1.z <= zmax &&
        p2.x >= xmin && p2.y >= ymin && p2.z >= zmin &&
        p2.x <= xmax && p2.y <= ymax && p2.z <= zmax &&
        p3.x >= xmin && p3.y >= ymin && p3.z >= zmin &&
        p3.x <= xmax && p3.y <= ymax && p3.z <= zmax) {
        return true;
    }
    // Check if any point of the triangle is inside this node
    if (containsPoint(p1) || containsPoint(p2) || containsPoint(p3)) {
        return true;
    }
    // Check if any edge of the triangle intersects this node
    if (intersectsLineSegment(p1, p2) ||
        intersectsLineSegment(p2, p3) ||
        intersectsLineSegment(p3, p1)) {
        return true;
    }
    return false;
}

bool Octree::intersectsTriangle(Triangle<Pointf> &triangle) {
    return intersectsTriangle(triangle.p1, triangle.p2, triangle.p3);
}

bool Octree::addTriangle(Triangle<int> &triangleIndices, Triangle<Pointf> &trianglePoints) {
    if (intersectsTriangle(trianglePoints)) {
        if (divided) {
            for (int i = 0; i < 8; i++) {
                children[i]->addTriangle(triangleIndices, trianglePoints);
            }
        } else {
            triangles.push_back(triangleIndices);
        }
        return true;
    }
    return false;
}

bool Octree::addTriangle(int i1, int i2, int i3) {
    Triangle<Pointf> trianglePoints(vertices[i1], vertices[i2], vertices[i3]);
    Triangle<int> triangleIndices(i1, i2, i3);
    return addTriangle(triangleIndices, trianglePoints);
}

bool Octree::addPoint(int index) {
    return addPoint(index, vertices[index]);
}

bool Octree::addPoint(int index, Pointf point) {
    if (containsPoint(point)) {
        if (divided) {
            for (int i = 0; i < 8; i++) {
                children[i]->addPoint(index, point);
            }
        } else {
            points.push_back(index);
        }
        return true;
    }
    return false;
}

void Octree::leafs(std::vector<Octree *> &result) {
    if (divided) {
        for (auto child : children) {
            child->leafs(result);
        }
    } else {
        result.push_back(this);
    }
}

std::vector<Octree *> Octree::leafs() {
    std::vector<Octree *> result;
    leafs(result);
    return result;
}

