//
// Created by boreas on 18-6-5.
//

#include <vector>
#include "Wrapper.h"
#include "Definition.h"
#include "Log.h"


// 对Vector<xyLoc>的操作
extern "C" {
std::vector<xyLoc> *new_vector_loc() {
    return new std::vector<xyLoc>();
}

std::vector<xyLoc> *new_vector_loc_arg(unsigned int size, xyLoc &value) {
    return new std::vector<xyLoc>(size, value);
}

std::vector<xyLoc> *copy_vector_loc(std::vector<xyLoc> &v) {
    return new std::vector<xyLoc>(v);
}

void delete_vector_loc(std::vector<xyLoc> *v) {
    LOG("destructor called in C++ for %d", *v);
    delete v;
}

void clear_vector_loc(std::vector<xyLoc> *v) {
    v->clear();
}

int vector_loc_size(std::vector<xyLoc> *v) {
    return static_cast<int>(v->size());
}

xyLoc *vector_loc_get(std::vector<xyLoc> *v, int i) {
    return &(v->at(static_cast<unsigned long>(i)));
}

void vector_loc_set(std::vector<xyLoc> *v, int i, xyLoc &value) {
    (*v)[i] = value;
}

void vector_loc_push_back(std::vector<xyLoc> *v, xyLoc *loc) {
    v->push_back(*loc);
}
}

// 对Vector<bool>的操作
extern "C" {
std::vector<bool> *new_vector_bool() {
    return new std::vector<bool>();
}

std::vector<bool> *new_vector_bool_arg(unsigned int size, bool value) {
    return new std::vector<bool>(size, value);
}

std::vector<bool> *copy_vector_bool(std::vector<bool> &v) {
    return new std::vector<bool>(v);
}

void delete_vector_bool(std::vector<bool> *v) {
    LOG("destructor called in C++ for %d", v);
    delete v;
}

void clear_vector_bool(std::vector<bool> *v) {
    v->clear();
}

int vector_bool_size(std::vector<bool> *v) {
    return static_cast<int>(v->size());
}

bool vector_bool_get(std::vector<bool> *v, int i) {
    return v->at(static_cast<unsigned long>(i));
}

void vector_bool_set(std::vector<bool> *v, int i, bool value) {
    (*v)[i] = value;
}

void vector_bool_push_back(std::vector<bool> *v, bool b) {
    v->push_back(b);
}
}

//extern "C++" {
//template<class T>
//std::vector<T> *new_vector() {
//    return new std::vector<T>();
//}
//
//template<class T>
//std::vector<T> *copy_vector(std::vector<T> &v) {
//    return new std::vector<T>(v);
//}
//
//template<class T>
//void delete_vector(std::vector<T> *v) {
//    LOG("destructor called in C++ for %d", v);
//    delete v;
//}
//
//template<class T>
//void clear_vector(std::vector<T> *v) {
//    v->clear();
//}
//
//template<class T>
//int vector_size(std::vector<T> *v) {
//    return static_cast<int>(v->size());
//}
//
//template<class T>
//xyLoc *vector_get(std::vector<T> *v, int i) {
//    return &(v->at(static_cast<unsigned long>(i)));
//}
//
//template<class T>
//void vector_push_back(std::vector<T> *v, xyLoc *loc) {
//    v->push_back(*loc);
//}
//}

// 对xyLoc的操作
extern "C" {
xyLoc *new_xy_loc(int x, int y) {
    return new xyLoc(x, y);
}

xyLoc *copy_xy_loc(xyLoc &loc) {
    return new xyLoc(loc.x, loc.y);
}

int get_x(xyLoc &loc) {
    return loc.x;
}

int get_y(xyLoc &loc) {
    return loc.y;
}
}

// 对JPSWrapper操作
extern "C" {
JPSPWrapper *new_jpsp_wrapper(std::vector<bool> &bits, int w, int h) {
    return new JPSPWrapper(bits, w, h);
}

void preprocess(JPSPWrapper &obj) {
    obj.Preprocess();
}

std::vector<xyLoc> *get_path(JPSPWrapper &obj, xyLoc &s, xyLoc &g) {
    LOG("start:%d, goal:%d\n", s, g);
    LOG("Enter api, start:(%d, %d), goal:(%d, %d)\n", s.x, s.y, g.x, g.y);
    return obj.GetPath(s, g);
}
}
