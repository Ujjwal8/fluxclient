#include <iostream>
#include <math.h>
#include <map>
#include <algorithm>
#include <limits>

#include <pcl/filters/voxel_grid.h>
////////////////////   fake code ////////////////////////////////////
#include <pcl/io/pcd_io.h>
//////////////////////////////////////////////////////
#include "printer_module.h"
#define likely(x)  __builtin_expect((x),1)

struct cone
{
  float pos[3];
  float theta;
  cone(){
    pos[0] = 0;
    pos[1] = 0;
    pos[2] = 0;
    theta = 0;
  }
  cone(float x, float  y, float z, float t){
    pos[0] = x;
    pos[1] = y;
    pos[2] = z;
    theta = t;
  }
  cone(cone const &cone2){
    pos[0] = cone2.pos[0];
    pos[1] = cone2.pos[1];
    pos[2] = cone2.pos[2];
    theta = cone2.theta;
  }
};

struct tree_node
{
  int left, right, index, height;
  tree_node(int l, int r, int i, int h){
    left = l;
    right = r;
    index = i;
    height = h;
  }
};

inline std::ostream& operator<< (std::ostream& stream, const tree_node& n)
{
  stream << n.left << " " << n.right << " " << n.index << " " << n.height << " " << std::endl;
  return stream;
}

class SupportTree{
public:
  SupportTree();
  ~SupportTree();

  pcl::PointCloud<pcl::PointXYZ>::Ptr cloud;
  std::vector<tree_node> tree;
};

SupportTree::SupportTree(){
  pcl::PointCloud<pcl::PointXYZ>::Ptr cloud2 (new pcl::PointCloud<pcl::PointXYZ>);
  cloud = cloud2;
}

SupportTree::~SupportTree(){

}

MeshPtr createMeshPtr(){
  MeshPtr mesh(new pcl::PolygonMesh);
  return mesh;
}

int set_point(MeshPtr triangles, std::vector< std::vector<float> > points){
  pcl::PointCloud<pcl::PointXYZ>::Ptr cloud (new pcl::PointCloud<pcl::PointXYZ>);
  for (uint32_t i = 0; i < points.size(); i += 1){
    pcl::PointXYZ p;
    p.x = points[i][0];
    p.y = points[i][1];
    p.z = points[i][2];
    cloud -> push_back(p);
  }

  toPCLPointCloud2(*cloud, triangles->cloud);
  return 0;
}

int push_backFace(MeshPtr triangles, int v0, int v1, int v2){
  pcl::Vertices v;
  v.vertices.resize(3);
  v.vertices[0] = v0;
  v.vertices[1] = v1;
  v.vertices[2] = v2;
  triangles->polygons.push_back(v);
  return 0;
}

int add_on(MeshPtr base, MeshPtr add_on_mesh){
  pcl::PointCloud<pcl::PointXYZ>::Ptr cloud (new pcl::PointCloud<pcl::PointXYZ>);
  fromPCLPointCloud2(base->cloud, *cloud);
  int size_to_add_on = cloud->size();

  pcl::PointCloud<pcl::PointXYZ>::Ptr cloud2 (new pcl::PointCloud<pcl::PointXYZ>);
  fromPCLPointCloud2(add_on_mesh->cloud, *cloud2);

    // add cloud together
  *cloud += *cloud2;

  pcl::Vertices v;
  v.vertices.resize(3);
    // add faces, but shift the index for add on mesh
  for (uint32_t i = 0; i < add_on_mesh->polygons.size(); i += 1){
    v.vertices[0] = add_on_mesh->polygons[i].vertices[0] + size_to_add_on;
    v.vertices[1] = add_on_mesh->polygons[i].vertices[1] + size_to_add_on;
    v.vertices[2] = add_on_mesh->polygons[i].vertices[2] + size_to_add_on;
    base->polygons.push_back(v);
  }

  toPCLPointCloud2(*cloud, base->cloud);
  return 0;
}

int bounding_box(MeshPtr triangles, std::vector<float> &b_box){
  pcl::PointCloud<pcl::PointXYZ>::Ptr cloud (new pcl::PointCloud<pcl::PointXYZ>);
  fromPCLPointCloud2(triangles->cloud, *cloud);
  float minx = std::numeric_limits<double>::infinity(), miny = std::numeric_limits<double>::infinity(), minz = std::numeric_limits<double>::infinity();
  float maxx = -1 * std::numeric_limits<double>::infinity(), maxy = -1 * std::numeric_limits<double>::infinity(), maxz = -1 * std::numeric_limits<double>::infinity();

  for (uint32_t i = 0; i < cloud->size(); i += 1){
    if((*cloud)[i].x > maxx){
      maxx = (*cloud)[i].x;
    }
    if((*cloud)[i].y > maxy){
      maxy = (*cloud)[i].y;
    }
    if((*cloud)[i].z > maxz){
      maxz = (*cloud)[i].z;
    }

    if((*cloud)[i].x < minx){
      minx = (*cloud)[i].x;
    }
    if((*cloud)[i].y < miny){
      miny = (*cloud)[i].y;
    }
    if((*cloud)[i].z < minz){
      minz = (*cloud)[i].z;
    }
  }

  b_box.resize(6);
  b_box[0] = minx;
  b_box[1] = miny;
  b_box[2] = minz;
  b_box[3] = maxx;
  b_box[4] = maxy;
  b_box[5] = maxz;

  return 0;
}

int bounding_box(pcl::PointCloud<pcl::PointXYZ>::Ptr cloud, std::vector<float> &b_box){
  float minx = std::numeric_limits<double>::infinity(), miny = std::numeric_limits<double>::infinity(), minz = std::numeric_limits<double>::infinity();
  float maxx = -1 * std::numeric_limits<double>::infinity(), maxy = -1 * std::numeric_limits<double>::infinity(), maxz = -1 * std::numeric_limits<double>::infinity();

  for (uint32_t i = 0; i < cloud->size(); i += 1){
    if((*cloud)[i].x > maxx){
      maxx = (*cloud)[i].x;
    }
    if((*cloud)[i].y > maxy){
      maxy = (*cloud)[i].y;
    }
    if((*cloud)[i].z > maxz){
      maxz = (*cloud)[i].z;
    }

    if((*cloud)[i].x < minx){
      minx = (*cloud)[i].x;
    }
    if((*cloud)[i].y < miny){
      miny = (*cloud)[i].y;
    }
    if((*cloud)[i].z < minz){
      minz = (*cloud)[i].z;
    }
  }
  b_box.resize(6);
  b_box[0] = minx;
  b_box[1] = miny;
  b_box[2] = minz;
  b_box[3] = maxx;
  b_box[4] = maxy;
  b_box[5] = maxz;

  return 0;
}

int apply_transform(MeshPtr triangles, float x, float y, float z, float rx, float ry, float rz, float sc_x, float sc_y, float sc_z){
  pcl::PointCloud<pcl::PointXYZ>::Ptr cloud (new pcl::PointCloud<pcl::PointXYZ>);
  fromPCLPointCloud2(triangles->cloud, *cloud);

  Eigen::Matrix4f transform = Eigen::Matrix4f::Identity();
  Eigen::Matrix4f tmpM = Eigen::Matrix4f::Identity();
  float theta; // The angle of rotation in radians

  std::vector<float> b_box;
  b_box.resize(3);
  std::vector<float> center;
  center.resize(3);

  // scale
  for (uint32_t i = 0; i < cloud->size(); i += 1){
    (*cloud)[i].x *= sc_x;
    (*cloud)[i].y *= sc_y;
    (*cloud)[i].z *= sc_z;
  }

  // move to origin
  bounding_box(cloud, b_box);
  for (int i = 0; i < 3; i += 1){
    center[i] = (b_box[i] + b_box[i + 3]) / 2;
  }
  transform = Eigen::Matrix4f::Identity();
  transform(0, 3) = - center[0];
  transform(1, 3) = - center[1];
  transform(2, 3) = - center[2];
  pcl::transformPointCloud(*cloud, *cloud, transform);

  // rotate
  transform = Eigen::Matrix4f::Identity();
  tmpM = Eigen::Matrix4f::Identity();
  theta = rx; // The angle of rotation in radians
  tmpM(1, 1) = cos (theta); //x
  tmpM(1, 2) = -sin(theta);
  tmpM(2, 1) = sin (theta);
  tmpM(2, 2) = cos (theta);
  transform *= tmpM;

  tmpM = Eigen::Matrix4f::Identity();
  theta = ry;
  tmpM(0, 0) = cos (theta); //y
  tmpM(2, 0) = -sin(theta);
  tmpM(0, 2) = sin (theta);
  tmpM(2, 2) = cos (theta);
  transform *= tmpM;

  tmpM = Eigen::Matrix4f::Identity();
  theta = rz;
  tmpM(0, 0) = cos (theta); //z
  tmpM(0, 1) = -sin(theta);
  tmpM(1, 0) = sin (theta);
  tmpM(1, 1) = cos (theta);
  transform *= tmpM;
  pcl::transformPointCloud(*cloud, *cloud, transform);

  // move to proper position
  transform = Eigen::Matrix4f::Identity();
  transform(0, 3) = x;
  transform(1, 3) = y;
  transform(2, 3) = z;
  pcl::transformPointCloud(*cloud, *cloud, transform);

  toPCLPointCloud2(*cloud, triangles->cloud);
  return 0;
}

int find_intersect(pcl::PointXYZ &a, pcl::PointXYZ &b, float floor_v, pcl::PointXYZ &p){
  // find the intersect between line a, b and plane z=floor_v
  std::vector<float> v(3);  // vetor a->b
  v[0] = b.x - a.x;
  v[1] = b.y - a.y;
  v[2] = b.z - a.z;

  float t = (floor_v - a.z) / v[2];
  p.x = a.x + t * v[0];
  p.y = a.y + t * v[1];
  p.z = a.z + t * v[2];
  return 0;
}


int cut(MeshPtr input_mesh, MeshPtr out_mesh, float floor_v){
  pcl::PointCloud<pcl::PointXYZ>::Ptr cloud (new pcl::PointCloud<pcl::PointXYZ>);
  fromPCLPointCloud2(input_mesh->cloud, *cloud);

  pcl::Vertices v;
  v.vertices.resize(3);
  pcl::Vertices on, under;
  // consider serveral case

  for (uint32_t i = 0; i < input_mesh->polygons.size(); i += 1){
    // on.vertices.clear();
    // under.vertices.clear();
    pcl::Vertices on, under;
    for (uint32_t j = 0; j < 3; j += 1){

      if ((*cloud)[input_mesh->polygons[i].vertices[j]].z <= floor_v){
        under.vertices.push_back(input_mesh->polygons[i].vertices[j]);
      }
      else{
        on.vertices.push_back(input_mesh->polygons[i].vertices[j]);
      }
    }

    if(on.vertices.size() == 3){
      out_mesh->polygons.push_back(on);
    }
    else if(under.vertices.size() == 3){
      // do nothing
    }
    else if(under.vertices.size() == 2){
      pcl::PointXYZ upper_p;
      upper_p = (*cloud)[on.vertices[0]];
      for (int j = 0; j < 2; j += 1){
        pcl::PointXYZ lower_p, new_p;
        lower_p = (*cloud)[under.vertices[j]];
        find_intersect(upper_p, lower_p, floor_v, new_p);
        cloud -> push_back(new_p);
        on.vertices.push_back(cloud -> size() - 1);
      }
      out_mesh->polygons.push_back(on);
    }
    else if(under.vertices.size() == 1){
      pcl::PointXYZ mid;
      mid.x = ((*cloud)[on.vertices[0]].x + (*cloud)[on.vertices[1]].x) / 2;
      mid.y = ((*cloud)[on.vertices[0]].y + (*cloud)[on.vertices[1]].y) / 2;
      mid.z = ((*cloud)[on.vertices[0]].z + (*cloud)[on.vertices[1]].z) / 2;
      cloud -> push_back(mid);
      int mid_index = cloud -> size() - 1;

      pcl::PointXYZ intersect0, intersect1;
      find_intersect((*cloud)[on.vertices[0]], (*cloud)[under.vertices[0]], floor_v, intersect0);
      cloud -> push_back(intersect0);
      int intersect0_index = cloud -> size() - 1;
      find_intersect((*cloud)[on.vertices[1]], (*cloud)[under.vertices[0]], floor_v, intersect1);
      cloud -> push_back(intersect1);
      int intersect1_index = cloud -> size() - 1;

      pcl::Vertices v1, v2, v3;
      v1.vertices.push_back(on.vertices[0]);
      v1.vertices.push_back(intersect0_index);
      v1.vertices.push_back(mid_index);
      out_mesh->polygons.push_back(v1);

      v2.vertices.push_back(mid_index);
      v2.vertices.push_back(intersect0_index);
      v2.vertices.push_back(intersect1_index);
      out_mesh->polygons.push_back(v2);

      v3.vertices.push_back(mid_index);
      v3.vertices.push_back(intersect1_index);
      v3.vertices.push_back(on.vertices[1]);
      out_mesh->polygons.push_back(v3);
    }

  }

  toPCLPointCloud2(*cloud, out_mesh->cloud);
  return 0;
}

int STL_to_List(MeshPtr triangles, std::vector<std::vector< std::vector<float> > > &data){
  // point's data
  // data =[
  //           t1[p1[x, y, z], p2[x, y, z], p3[x, y, z]],
  //           t2[p1[x, y, z], p2[x, y, z], p3[x, y, z]],
  //           t3[p1[x, y, z], p2[x, y, z], p3[x, y, z]],
  //             ...
  //       ]
  pcl::PointCloud<pcl::PointXYZ>::Ptr cloud (new pcl::PointCloud<pcl::PointXYZ>);
  fromPCLPointCloud2(triangles->cloud, *cloud);

  int v0, v1, v2;

  std::vector<float> tmpvv(3, 0.0);
  std::vector< std::vector<float> > tmpv(3, tmpvv);
  data.resize(triangles->polygons.size());

  for (size_t i = 0; i < triangles->polygons.size(); i += 1){
    data[i] = tmpv;

    v0 = triangles->polygons[i].vertices[0];
    v1 = triangles->polygons[i].vertices[1];
    v2 = triangles->polygons[i].vertices[2];

    data[i][0][0] = (*cloud)[v0].x;
    data[i][0][1] = (*cloud)[v0].y;
    data[i][0][2] = (*cloud)[v0].z;

    data[i][1][0] = (*cloud)[v1].x;
    data[i][1][1] = (*cloud)[v1].y;
    data[i][1][2] = (*cloud)[v1].z;

    data[i][2][0] = (*cloud)[v2].x;
    data[i][2][1] = (*cloud)[v2].y;
    data[i][2][2] = (*cloud)[v2].z;
      // std::cout << "  polygons[" << i << "]: " <<std::endl;
  }
  return 0;
}

int STL_to_Faces(MeshPtr triangles, std::vector< std::vector<int> > &data){
  // index of faces
  // data = [ f1[p1_index, p2_index, p3_index],
  //          f2[p1_index, p2_index, p3_index], ...
  //        ]
  data.resize(triangles->polygons.size());
  for (size_t i = 0; i < triangles->polygons.size(); i += 1){
    data[i].resize(3);
    data[i][0] = triangles->polygons[i].vertices[0];
    data[i][1] = triangles->polygons[i].vertices[1];
    data[i][2] = triangles->polygons[i].vertices[2];
  }
  return 0;
}

bool sort_by_z(const pcl::PointXYZ a,const pcl::PointXYZ b)
{
   return a.z < b.z;
}
bool sort_by_second(const std::pair<int, float> a, const std::pair<int, float> b){
  if(likely(a.second != b.second)){
    return a.second < b.second;
  }
  else{
    return a.first < b.first;
  }
}

int add_support(MeshPtr input_mesh, MeshPtr out_mesh){
  //////////////////
  double m_threshold = std::numeric_limits<double>::infinity();
  //////////////////
  SupportTree support_tree;
  float alpha = M_PI / 4;
  pcl::PointCloud<pcl::PointXYZ>::Ptr P(new pcl::PointCloud<pcl::PointXYZ>);
  find_support_point(input_mesh, alpha, 1, P);
  std::cout<< "P size need to be supported:"<< P->size() << std::endl;

  sort(P -> points.begin(), P -> points.end(), sort_by_z);
  std::map<int, cone> C;

  // std::map<int, int> P_v;  // main index list for tracing cones, just use for rb-tree, value is shit
  std::vector< std::pair<int, float> > P_v;  // main index list, first = index of P, second = z-coordinate
  for (size_t i = 0; i < P -> points.size(); i += 1){
    C[i] = cone(P -> points[i].x, P -> points[i].y, P -> points[i].z, alpha);
    P_v.push_back(std::pair<int, float>(i, P -> points[i].z));
  }
  sort(P_v.begin(), P_v.end(), sort_by_second);
  // std::vector<pcl::PointXYZ, Eigen::aligned_allocator<pcl::PointXYZ> > P_v(P -> points);
  int add_points_index = P_v.size();
  // std::map<int, double, bool(*)(double, double)> S(double_fn_pt); //use this as r-b-tree, key: index, value:distance
  std::vector<std::pair <int, float> > S;  // candidate for intersecino point, index of P_v and distance
  int current_point;
  for (size_t i = 0; i < P_v.size(); i += 1){
    support_tree.tree.push_back(tree_node(-1, -1, P_v[i].first, 1));
  }

  while(P_v.size() != 0){
    current_point = P_v.back().first;  // index of P
    P_v.pop_back();  // erase smallest p, can slightly skip some if statement
    S.clear();
    // cone-cone intersection
    std::map<int, cone> tmp_C;  // cones, key = index of P_v
    // for(std::vector< std::pair<int, float> >::iterator iter = P_v.begin(); iter != P_v.end(); iter++){
    //   cone tmp_cone;
    //   S.push_back(std::pair<int, double>(iter -> first, cone_intersect(C[current_point], C[iter -> first], tmp_cone)));
    //   tmp_C[iter -> first] = tmp_cone;
    // }
    for (size_t i = 0; i < P_v.size(); i += 1){
      cone tmp_cone;
      S.push_back(std::pair<int, double>(i, cone_intersect(C[current_point], C[P_v[i].first], tmp_cone)));
      tmp_C[i] = tmp_cone;
    }

    // cone-plate intersection
    S.push_back(std::pair<int, double>(-1, P -> points[current_point].z));

    // cone-mesh intersection, use -1 to indicate it's connected to mesh
    ////////////////  TODO ////////////////////////

    S.push_back(std::pair<int, double>(-2, std::numeric_limits<double>::infinity()));
    ///////////////////////////////////////////////
    std::pair<int, float> m(S[0]);  // first: index of P_v, second: distance
    for (size_t i = 0; i < S.size(); i += 1){
      if(S[i].second < m.second){
        m = S[i];
      }
      // std::cout<< "S:" << S[i].first << " " << S[i].second << std::endl;
    }

    if(m.second > m_threshold){
        // C.erase(current_point);
    }
    else{
      // std::vector< std::pair<int, float> >::iterator iter;
      // iter = std::lower_bound(P_v.begin(), P_v.end(), std::pair<int, float>(P_v[m.first].first, P -> points[P_v[m.first].first].z));
      // std::cout<< "at:" << iter -  P_v.begin() << " " << P_v.size() << std::endl;

      if(m.first >= 0){  // cone
        // tmp_C[m.first]
        // std::cout<< "m.first " << m.first << std::endl;
        P_v.erase(P_v.begin() + m.first);

        cone c(tmp_C[m.first]);
        P_v.push_back(std::pair<int, double>(P->size(), c.pos[2]));
        C[P->size()] = c;
        P -> points.push_back(pcl::PointXYZ(c.pos[0], c.pos[1], c.pos[2]));

        support_tree.tree.push_back(tree_node(current_point, P_v[m.first].first, P->size() - 1, support_tree.tree[current_point].height + support_tree.tree[P_v[m.first].first].height));
      }
      else if(m.first == -1){ // plate
        // support_tree
        cone c;
        // P_v.push_back(std::pair<int, double>(P->size(),  ));
        P -> points.push_back(pcl::PointXYZ(P -> points[current_point].x, P -> points[current_point].y, 0));
        // C[P->size() ](c);
        support_tree.tree.push_back(tree_node(-3, current_point, P->size() - 1, support_tree.tree[current_point].height));
      }
      else if(m.first == -2){ // mesh
        //////////////////// TODO ////////////////////////////////////////
        // P -> points.push_back(pcl::PointXYZ(intersect.x, intersect.y, intersect.z));
        // support_tree.tree.push_back(tree_node(-2, current_point, P->size() - 1, support_tree.tree[current_point].height));
        ////////////////////////////////////////////////////////////
      }
      else{
        std::cout<< "GG, this shouldn't  happen"<< std::endl;
        assert(false);
      }
      // std::cout<< P_v.size() << std::endl;
    }
  }
  // for (size_t i = 0; i < support_tree.tree.size(); i += 1){
  //   std::cout << support_tree.tree[i] << std::endl;
  // }

  return 0;
}

double cone_intersect(cone a, cone b, cone &c){
  // return distance between cone a and interseciton
  // inner division point
  float dz = b.pos[2] - a.pos[2];
  float dxy = sqrt(pow(a.pos[0] - b.pos[0], 2) + pow(a.pos[1] - b.pos[1], 2));
  if(dxy == 0){
    if(dz == 0){
      c = a;
      return 0;
    }
    return -1;
  }
  float h;
  // std::cout<< "tan " << tan(a.theta) << std::endl;
  // std::cout<< "dxy " << dxy << std::endl;
  // std::cout<< "dz " << dz << std::endl;
  h = (dxy / tan(a.theta) - dz) / 2;
  // std::cout<< "h:" << h << std::endl;

  c.pos[0] = ((h + dz) * a.pos[0] + h * b.pos[0]) / (h + dz + h),
  c.pos[1] = ((h + dz) * a.pos[1] + h * b.pos[1]) / (h + dz + h),
  c.pos[2] = a.pos[2] - h;
  c.theta = a.theta;

  return sqrt(pow(a.pos[0] - c.pos[0], 2) + pow(a.pos[1] - c.pos[1], 2) + pow(a.pos[2] - c.pos[2], 2));
}

int find_support_point(MeshPtr triangles, float alpha, float sample_rate, pcl::PointCloud<pcl::PointXYZ>::Ptr P){
  // ref: http://hpcg.purdue.edu/bbenes/papers/Vanek14SGP.pdf
  // MeshPtr triangles[in]: input stl
  // float alpha[in]: angle that >= alpha need to be supported
  // float sample_rate[in]: sample reate (grid)
  // P[out]: out put the points that need to be supported

  pcl::PointCloud<pcl::PointXYZ>::Ptr cloud (new pcl::PointCloud<pcl::PointXYZ>);
  fromPCLPointCloud2(triangles->cloud, *cloud);
  std::cout << "input points " << cloud -> size() << std::endl;
  std::cout << "input faces " << triangles->polygons.size() << std::endl;
  // std::vector<float> normal_p;
  // std::vector<float> normal_vertical;
  // normal_vertical.push_back(0);
  // normal_vertical.push_back(0);
  // normal_vertical.push_back(1);
  float cos_alpha = cos((M_PI / 2.0) - alpha);
  float a[3], b[3], normal_p[3];
  float la, lb;
  const float normal_vertical[3] = {0, 0, -1};
  std::vector<int> rec;
  for (size_t i = 0; i < triangles->polygons.size(); i += 1){
  // for (size_t i = 0; i < 1; i += 1){

    a[0] = (*cloud)[(triangles->polygons[i]).vertices[1]].x - (*cloud)[(triangles->polygons[i]).vertices[0]].x;
    a[1] = (*cloud)[(triangles->polygons[i]).vertices[1]].y - (*cloud)[(triangles->polygons[i]).vertices[0]].y;
    a[2] = (*cloud)[(triangles->polygons[i]).vertices[1]].z - (*cloud)[(triangles->polygons[i]).vertices[0]].z;

    b[0] = (*cloud)[(triangles->polygons[i]).vertices[2]].x - (*cloud)[(triangles->polygons[i]).vertices[0]].x;
    b[1] = (*cloud)[(triangles->polygons[i]).vertices[2]].y - (*cloud)[(triangles->polygons[i]).vertices[0]].y;
    b[2] = (*cloud)[(triangles->polygons[i]).vertices[2]].z - (*cloud)[(triangles->polygons[i]).vertices[0]].z;

    normal_p[0] = a[1] * b[2] - a[2] * b[1];
    normal_p[1] = a[2] * b[0] - a[0] * b[2];
    normal_p[2] = a[0] * b[1] - a[1] * b[0];

    // std::cout << normal_p[2] << std::endl;
    float cos_theta = (normal_p[0] * normal_vertical[0] + normal_p[1] * normal_vertical[1] + normal_p[2] * normal_vertical[2]) / (sqrt(normal_p[0] * normal_p[0] + normal_p[1] * normal_p[1] + normal_p[2] * normal_p[2]) * sqrt(normal_vertical[0] * normal_vertical[0] + normal_vertical[1] * normal_vertical[1] + normal_vertical[2] * normal_vertical[2]));
    // std::cout << cos_theta << std::endl;
    // faces that need to be supported
    if(cos_theta >= cos_alpha){  //cosine, larger angle, smaller cosine
      rec.push_back(i);
      // std::cout << "> "<<i << std::endl;
      b[0] = (*cloud)[(triangles->polygons[i]).vertices[2]].x - (*cloud)[(triangles->polygons[i]).vertices[1]].x;
      b[1] = (*cloud)[(triangles->polygons[i]).vertices[2]].y - (*cloud)[(triangles->polygons[i]).vertices[1]].y;
      b[2] = (*cloud)[(triangles->polygons[i]).vertices[2]].z - (*cloud)[(triangles->polygons[i]).vertices[1]].z;
      la = sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]);
      lb = sqrt(b[0] * b[0] + b[1] * b[1] + b[2] * b[2]);
      int ia = (int)(la / (sample_rate / 10.));
      int ib = (int)(lb / (sample_rate / 10.));
      // std::cout<< "ia:"<< ia << " " << ib << std::endl;


      for (size_t j = 0; j < ia; j += 1){
        for (size_t k = 0; k < (float)j / ia * ib; k += 1){
          pcl::PointXYZ point;
          point.x = (*cloud)[(triangles->polygons[i]).vertices[0]].x + a[0] * j / ia + b[0] * k / ib;
          point.y = (*cloud)[(triangles->polygons[i]).vertices[0]].y + a[1] * j / ia + b[1] * k / ib;
          point.z = (*cloud)[(triangles->polygons[i]).vertices[0]].z + a[2] * j / ia + b[2] * k / ib;
          if(point.z != 0){
            P -> push_back(point);
          }
        }
      }
    }
  }
  std::cout<< "face need:"<< rec.size() << std::endl;
  std::cout<< "P size before:"<< P->size() << std::endl;
  pcl::VoxelGrid<pcl::PointXYZ> grid;
  grid.setLeafSize(sample_rate, sample_rate, sample_rate);
  grid.setInputCloud(P);
  grid.filter(*P);
  std::cout<< "P size after:"<< P->size() << std::endl;

  pcl::io::savePCDFileASCII ("tmp.pcd", *P);

  return 0;
}
