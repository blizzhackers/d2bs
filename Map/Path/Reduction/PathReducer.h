#pragma once

#include "..\..\Map.h"

namespace Mapping {
namespace Pathing {
namespace Reducing {

class PathReducer {
  public:
    PathReducer() {
    }
    // getPath deletes reducers through this base, so each subclass destructor must run.
    virtual ~PathReducer() {
    }
    PathReducer(const PathReducer&);
    PathReducer& operator=(const PathReducer&);
    virtual void Reduce(PointList const& in, PointList& out, bool abs) = 0;
    virtual void GetOpenNodes(Point const& center, PointList& out, Point const& endpoint) = 0;
    virtual bool Reject(Point const& pt, bool abs) = 0;
    virtual int GetPenalty(Point const& pt, bool abs) = 0;
    virtual void MutatePoint(Point& pt, bool abs) = 0;
    // True once a reducer's script callback failed; getPath then fails with the error the callback left pending.
    virtual bool Failed() const {
        return false;
    }
    // True once a reducer stopped the search; getPath then returns an empty path, unless Failed() is true as well.
    virtual bool Stopped() const {
        return false;
    }
};

} // namespace Reducing
} // namespace Pathing
} // namespace Mapping