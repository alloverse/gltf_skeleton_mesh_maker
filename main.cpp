#include <stdio.h>
#include <iterator>

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "tinygltf/tiny_gltf.h"
#include <unistd.h>
#include <iostream>
#include <fstream>

using std::cout;
using std::endl;


// the 8 corners of

template <typename Num>
struct Point {
    Num x, y, z;
    Point() : x(0.0), y(0.0), z(0.0) {}
    Point(Num x, Num y, Num z) : x(x), y(y), z(z) {}
    
    bool operator<(Point &right) {
        return x < right.x && y < right.y && z < right.z;
    }
    bool operator>(Point &right) {
        return x > right.x && y > right.y && z > right.z;
    }
    
    Point operator+(Point right) {
        return Point(x+right.x, y+right.y, z+right.z);
    }
    Point operator-(Point right) {
        return Point(x-right.x, y-right.y, z-right.z);
    }
    Point operator/(Num right) {
        return Point(x/right, y/right, z/right);
    }
    
    bool closeto(Point<Num> other) {
        const auto eps = __FLT_EPSILON__;
        return abs(x-other.x) < eps && abs(y-other.y) < eps && abs(z-other.z) < eps;
    }
    
    Num sum() {
        return x + y + z;
    }
    
    Num product() {
        return x * y * z;
    }
};

template <typename Num>
std::ostream& operator<<(std::ostream &os, Point<Num>& point) {
    os << point.x << ", " << point.y << ", " << point.z;
    return os;
}



typedef Point<float> Pointf;
// the 8 corners of a cube
void vertices(std::vector<Pointf> out) {
    const float s = 0.5;
    // front side (-y) starting top left (-x, +z)
    out.push_back(Pointf(-s, -s,  s)); // 0
    out.push_back(Pointf( s, -s,  s)); // 1
    out.push_back(Pointf(-s, -s, -s)); // 2
    out.push_back(Pointf( s, -s,  s)); // 3
    
    // backside (+y) starting top left (-x, +z)
    out.push_back(Pointf(-s,  s,  s)); // 4
    out.push_back(Pointf( s,  s,  s)); // 5
    out.push_back(Pointf(-s,  s, -s)); // 6
    out.push_back(Pointf( s,  s, -s)); // 7
}

const Pointf points[] {
    // front side (-y) starting top left (-x, +z)
    Pointf(-0.5f, -0.5f,  0.5f), // 0
    Pointf( 0.5f, -0.5f,  0.5f), // 1
    Pointf(-0.5f, -0.5f, -0.5f), // 2
    Pointf( 0.5f, -0.5f,  0.5f), // 3
    // backside (+y) starting top left (-x, +z)
    Pointf(-0.5f,  0.5f,  0.5f), // 4
    Pointf( 0.5f,  0.5f,  0.5f), // 5
    Pointf(-0.5f,  0.5f, -0.5f), // 6
    Pointf( 0.5f,  0.5f, -0.5f)  // 7
};
const int planes[] {
    //-x: 0
    4,6,2,2,0,4,
    //+x: 1
    1,3,5,5,3,7,
    // -y: 2
    0,2,1,1,2,3,
    // +y: 3
    4,5,6,6,5,7,
    //-z: 4
    2,6,3,3,6,7,
    //+z: 5
    0,1,4,4,1,5
};


void add(std::vector<Pointf> &vert, std::vector<int> &ind, Pointf point) {
    for (int i = 0; i < vert.size(); i++) {
        if (point.closeto(vert[i])) {
            ind.push_back(i);
            return;
        }
    }
    ind.push_back(vert.size());
    vert.push_back(point);
}

void cube(std::vector<Pointf> &vert, std::vector<int> &ind, Pointf origin, int direction) {
    add(vert, ind, origin + points[planes[direction*6 + 0]]);
    add(vert, ind, origin + points[planes[direction*6 + 1]]);
    add(vert, ind, origin + points[planes[direction*6 + 2]]);
    add(vert, ind, origin + points[planes[direction*6 + 3]]);
    add(vert, ind, origin + points[planes[direction*6 + 4]]);
    add(vert, ind, origin + points[planes[direction*6 + 5]]);
}


 


int main(int argc, char **argv) {
    using namespace tinygltf;

    Model model;
    TinyGLTF loader;
    std::string err;
    std::string warn;
    
    char *filename = argv[1];
    if (!filename) {
//        filename = "../../PUSHILIN_forest.glb";
        filename = "../../DamagedHelmet.glb";
//        printf("No model file\n");
//        return 1;
    }
    
    printf("%s exists: %s\n", filename, std::filesystem::exists(filename) ? "yes" : "no");
//    bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, filename);
    bool ret = loader.LoadBinaryFromFile(&model, &err, &warn, filename); // for binary glTF(.glb)

    if (!warn.empty()) {
      printf("Warn: %s\n", warn.c_str());
    }

    if (!err.empty()) {
      printf("Err: %s\n", err.c_str());
    }
    
    if (!ret) {
      printf("Failed to parse glTF\n");
      return -1;
    }
    
    // Collect points and find the size
    Point min(MAXFLOAT, MAXFLOAT, MAXFLOAT);
    Point max(-MAXFLOAT, -MAXFLOAT, -MAXFLOAT);
    std::vector<Point<float>> points;
    for (auto &mesh : model.meshes) {
        for (auto primitive : mesh.primitives) {
            const tinygltf::Accessor& accessor = model.accessors[primitive.attributes["POSITION"]];
            const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
            
            // cast to float type read only. Use accessor and bufview byte offsets to determine where position data
            // is located in the buffer.
            const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];
            // bufferView byteoffset + accessor byteoffset tells you where the actual position data is within the buffer. From there
            // you should already know how the data needs to be interpreted.
            const float* positions = reinterpret_cast<const float*>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);
            // From here, you choose what you wish to do with this position data. In this case, we  will display it out.
            for (size_t i = 0; i < accessor.count; ++i) {
                // Positions are Vec3 components, so for each vec3 stride, offset for x, y, and z.
//                std::cout << "(" << positions[i * 3 + 0] << ", "// x
//                << positions[i * 3 + 1] << ", " // y
//                << positions[i * 3 + 2] << ")" // z
//                << "\n";
                
                Point point(positions[i * 3 + 0], positions[i * 3 + 1], positions[i * 3 + 2]);
                min.x = point.x < min.x ? point.x : min.x;
                min.y = point.y < min.y ? point.y : min.y;
                min.z = point.z < min.z ? point.z : min.z;
                
                max.x = point.x > max.x ? point.x : max.x;
                max.y = point.y > max.y ? point.y : max.y;
                max.z = point.z > max.z ? point.z : max.z;
                
                points.push_back(point);
            }
        }
    }
    
    Point offset = Point<float>() - min;
    min = min + offset;
    max = max + offset;
    Point size = max;
    cout << "min: " << min << endl;
    cout << "max: " << max << endl;
    cout << "size: " << size << endl;
    
    struct Grid {
        bool *data;
        Point<int> dim;
        Point<float> size;
        
        Grid(Point<int> dimension, Point<float> size) : dim(dimension), size(size) {
            Point ds = Point(1,1,1) + dim;
            this->data = new bool[ds.x * ds.y * ds.z];
            for(int i = 0; i < ds.x * ds.y * ds.z; i++) {
                this->data[i] = false;
            }
        }
        ~Grid() {
            delete[] data;
        } 
        
        int index(int x, int y, int z) {
            assert(x >= 0 && y >= 0 && z >= 0);
            assert(x <= dim.x && y <= dim.y && z <= dim.z);
            int i = z * (dim.x * dim.y) + y * (dim.x) + x;
//            assert(i < dim.x * dim.y * dim.z);
            return i;
        }
        
        void add(Point<float> point) {
            int x = dim.x * (point.x / size.x);
            int y = dim.y * (point.y / size.y);
            int z = dim.z * (point.z / size.z);
            data[index(x, y, z)] = true;
        }
        
        bool get(int x, int y, int z) {
            return data[index(x, y, z)];
        }
        
        void set(int x, int y, int z, bool value) {
            data[index(x, y, z)] = value;
        }
    };

    Grid grid(Point(20, 20, 20), size);
    for (auto point : points) {
        grid.add(offset + point);
    }
    
    
    // Connect all corners to make a homogenous thing
    int added_count;
    int maxiter = 10;
    do {
        added_count = 0;
        for (int z = 0; z < grid.dim.z; z++) {
            for (int y = 0; y < grid.dim.y; y++) {
                for (int x = 0; x < grid.dim.x; x++) {
                    if (grid.get(x, y, z)) {
                        int min = 0xffffff, max = -0xffffff;
                        for (int i = 0; i < grid.dim.x; i++) {
                            if (grid.get(i, y, z) && i != x) {
                                min = i < min ? i : min;
                                max = i > max ? i : max;
                            }
                        }
                        for (int i = min; i < max; i++) {
                            grid.set(i, y, z, true);
                            added_count++;
                        }
                        min = min = 0xffffff, max = -0xffffff;
                        for (int i = 0; i < grid.dim.x; i++) {
                            if (grid.get(x, i, z) && i != y) {
                                min = i < min ? i : min;
                                max = i > max ? i : max;
                            }
                        }
                        for (int i = min; i < max; i++) {
                            grid.set(x, i, z, true);
                            added_count++;
                        }
                        min = min = 0xffffff, max = -0xffffff;
                        for (int i = 0; i < grid.dim.z; i++) {
                            if (grid.get(x, y, i) && i != z) {
                                min = i < min ? i : min;
                                max = i > max ? i : max;
                            }
                        }
                        for (int i = min; i < max; i++) {
                            grid.set(x, y, i, true);
                            added_count++;
                        }
                    }
                }
            }
        }
        if (--maxiter <= 0) {
            cout << "That's a livelock" << endl;
            break;
        };
    } while (added_count > 0);
    
    
    // print a slice of the grid?
    for (int z = 0; z < grid.dim.z; z += 2) {
        for (int y = 0; y < grid.dim.x; y++) {
            for (int x = 0; x < grid.dim.x; x++) {
                if (x == 0) cout << endl;
                //        auto filled = plane[i];
                auto filled = grid.get(x, y, z);
                cout << (filled ? "0 " : "  ");
            }
        }
        cout << endl;
    }
    
    
    // project to one plane
    bool *plane = new bool[grid.dim.x * grid.dim.y];
    // print a slice of the grid?
    for (int z = 0; z < grid.dim.z; z++) {
        for (int y = 0; y < grid.dim.x; y++) {
            for (int x = 0; x < grid.dim.x; x++) {
                //        auto filled = plane[i];
                auto filled = grid.get(x, y, z);
                plane[grid.index(x, y, 0)] |= filled;
            }
        }
    }
    
    for (int y = 0; y < grid.dim.x; y++) {
        for (int x = 0; x < grid.dim.x; x++) {
            if (x == 0) cout << endl;
            //        auto filled = plane[i];
            auto filled = plane[grid.index(x, y, 0)];
            cout << (filled ? "0 " : "  ");
        }
    }
    cout << endl;
    
    // marchinf of the cubes
    std::vector<Pointf> vertices;
    std::vector<int> indices;
    for (int z = 0; z < grid.dim.z; z++) {
        for (int y = 0; y < grid.dim.y; y++) {
            for (int x = 0; x < grid.dim.x; x++) {
                if (!grid.get(x, y, z)) continue;
                //-x: 0
                //+x: 1
                // -y: 2
                // +y: 3
                //-z: 4
                //+z: 5
                bool hits[] = {
                    (x > 0 && grid.index(x-1, y, z)),
                    (x < grid.dim.x && grid.get(x+1, y, z)),
                    (y > 0 && grid.index(x, y-1, z)),
                    (y < grid.dim.y && grid.get(x, y+1, z)),
                    (z > 0 && grid.index(x, y, z-1)),
                    (z < grid.dim.z && grid.get(x, y, z+1))
                };
                
                // add geometry where neighbors are missing
                for (int side = 0; side < 6; side++) {
                    if (hits[side] == false) {
                        Pointf origin = Pointf(x, y, z);
                        cube(vertices, indices, origin, side);
                    }
                }
            }
        }
    }
    
    // Build obj file
    std::string contents = "";
    std::ofstream file;
    file.open("out.obj");
    for (auto point : vertices) {
        file << "v " << point.x << " " << point.y << " " << point.z << endl;
    }
    file << endl;
    for (int i = 0; i < indices.size(); i+=3) {
        file << "f " << indices[i+0]+1 << " " << indices[i+1]+1 << " " << indices[i+2]+1 << endl;
    }
    file.close();
    
}
