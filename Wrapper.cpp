#include "stdafx.h"
#include <fstream>
#include "Wrapper.h"
#include "Log.h"
//#include <boost/python.hpp>
//#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

//using namespace boost;
//using namespace boost::python;

JPSPWrapper::JPSPWrapper(std::vector<bool> &bits, int w, int h) {
    this->bits = bits;
    this->w = w;
    this->h = h;
}

JPSPWrapper::~JPSPWrapper() {
    if (this->jpsPlus) {
        this->jpsPlus->~JPSPlus();
    }
}

void JPSPWrapper::Preprocess() {
    LOG("Preprocessing...\n");

    PrecomputeMap precomputeMap(w, h, bits);
    precomputeMap.CalculateMap();
    precomputeMap.ConstrutMap();
    JumpDistancesAndGoalBounds **preprocessedMap = precomputeMap.GetPreprocessedMap();
    this->jpsPlus = new JPSPlus(preprocessedMap, bits, w, h);
}

std::vector<xyLoc> *JPSPWrapper::GetPath(xyLoc &s, xyLoc &g) {
    LOG("Searching path, (%d, %d)->(%d, %d)\n", s.x, s.y, g.x, g.y);

    std::vector<xyLoc> *path = new std::vector<xyLoc>();
    bool done;

    do {
        if (s.x == g.x && s.y == g.y) {
            path->push_back(s);
            done = true;
        } else {
            done = this->jpsPlus->GetPath((xyLocJPS &) s, (xyLocJPS &) g, (std::vector<xyLocJPS> &) *path);
        }
    } while (!done);

    LOG("Search result:\n");

#if __DEBUG__
    if (path->empty()) {
        LOG("Not found.\n");
    } else {
        for (int i = 0; i < path->size(); ++i) {
            if (i % 10 == 0) {
                LOG("(%d, %d)->\n", (*path)[i].x, (*path)[i].y);
            } else if (i < path->size() - 1) {
                LOG("(%d, %d)->", (*path)[i].x, (*path)[i].y);
            } else {
                LOG("(%d, %d)", (*path)[i].x, (*path)[i].y);
            }
        }
    }
#endif

    return path;
}

//bool operator==(const xyLoc &left, const xyLoc &right) {
//    return (left.x == right.x && left.y == right.y);
//}

//namespace boost {
//    // 自定义异常处理，不然boost.python会报错
//    void throw_exception(std::exception const &e) {
//        return;
//    }
//}

//BOOST_PYTHON_MODULE (libjpsp) {
//    class_<JPSPWrapper>("JPSPWrapper")
//            .def(init<std::vector<bool> &, int, int>())
//            .def("preprocess", &JPSPWrapper::Preprocess)
//            .def("get_path", &JPSPWrapper::GetPath);
//    class_<xyLoc>("xyLoc")
//            .def(init<int, int>())
//            .def_readwrite("x", &xyLoc::x)
//            .def_readwrite("y", &xyLoc::y);
//    class_<std::vector<bool>>("VecBool")
//            .def(vector_indexing_suite<std::vector<bool> >());
//    class_<std::vector<xyLoc>>("VecLoc")
//            .def(vector_indexing_suite<std::vector<xyLoc> >());
//}