#pragma once

#include "Path.h"
#include "..\Map.h"
#include "Reduction\PathReducer.h"
#include "D2Structs.h"
#include <vector>
#include <set>
#include <queue>
#include <cmath>

// Declared in JSParty.h, which this Map-layer header does not include.
bool OpenGameSession(void);

namespace Mapping {
namespace Pathing {

inline int __fastcall Manhattan(Point const& start, Point const& end) {
    return 10 * (std::abs(start.first - end.first) + std::abs(start.second - end.second));
}

inline int __fastcall DiagonalShortcut(Point const& start, Point const& end) {
    int xdist = std::abs(start.first - end.first);
    int ydist = std::abs(start.second - end.second);
    return ((xdist > ydist) ? 14 * ydist + 10 * (xdist - ydist) : 14 * xdist + 10 * (ydist - xdist));
}

inline int __fastcall Chebyshev(Point const& start, Point const& end) {
    int xdist = (start.first - end.first);
    int ydist = (start.second - end.second);
    return (xdist > ydist ? xdist : ydist);
}

inline int __fastcall Euclidean(Point const& start, Point const& end) {
    double dx = (double)(end.first - start.first);
    double dy = (double)(end.second - start.second);
    dx = pow(dx, 2);
    dy = pow(dy, 2);
    return sqrt(dx + dy) * 10;
}

inline int __fastcall Slope(Point const& start, Point const& end) {
    double dx = (double)(end.first - start.first);
    double dy = (double)(end.second - start.second);
    return dy/dx;
}

inline bool __fastcall checkFlag(int flag) {
    // avoid is a bitwise superset of the other flags, don't need this check
    return (/*((ActMap::Avoid & flag) == ActMap::Avoid) |*/ (((ActMap::BlockWalk | ActMap::BlockPlayer) & flag) > 0));
}

inline int __fastcall EstimateDistance(const Map* m, const Point& point, const Point& end) {
    return DiagonalShortcut(point, end);
}

// Where a search started. A type-3 callback runs without the game lock, so the game may move on during one and the
// search must stop when the player is no longer there.
struct SearchOrigin {
    DWORD session;
    DWORD levelNo;
    const Act* act;
};

// The player's rooms are walked only while its act is the loaded act, checked before GameReady() as GetLevel does:
// an act change frees the old act before the player's own pointer moves on. 0 means the search must stop.
inline DWORD SearchPlayerLevelNo(DWORD session) {
    UnitAny* player = D2CLIENT_GetPlayerUnit();
    Act* act = *p_D2CLIENT_CurrentAct;
    if (!player || !act || player->pAct != act || !GameReady() || Vars.dwGameSession != session)
        return 0;
    if (!player->pPath || !player->pPath->pRoom1 || !player->pPath->pRoom1->pRoom2 || !player->pPath->pRoom1->pRoom2->pLevel)
        return 0;
    return player->pPath->pRoom1->pRoom2->pLevel->dwLevelNo;
}

// Call with the game lock held. False when there is no open game session or the player is not in a loaded level.
inline bool CaptureSearchOrigin(SearchOrigin& origin) {
    if (!OpenGameSession())
        return false;
    origin.session = Vars.dwGameSession;
    origin.act = *p_D2CLIENT_CurrentAct;
    origin.levelNo = SearchPlayerLevelNo(origin.session);
    return origin.levelNo != 0;
}

// The act is compared by address: a trip to another act and back frees the act the search started in. A trip away
// and back within the same act, or a new act allocated at the old address, is not seen.
inline bool SearchOriginHolds(const SearchOrigin& origin) {
    return *p_D2CLIENT_CurrentAct == origin.act && SearchPlayerLevelNo(origin.session) == origin.levelNo;
}

#pragma warning(disable : 4512)

class Node {
  public:
    Node* const parent;
    Point point;
    int g, h;

    Node() : parent(0) {
    }
    Node(Point s, Node* p, int _g, int _h) : point(s), parent(p), g(_g), h(_h) {
    }
};
struct NodeComparer {
    inline bool __fastcall operator()(const Node* left, const Node* right) const {
        return (left->g + left->h) > (right->g + right->h);
    }
};

template <class Allocator = std::allocator<Node>> class AStarPath : public MapPath {
  private:
    Allocator alloc;

    Map* map;
    Reducing::PathReducer* reducer;
    Estimator estimate;
    Distance distance;

    inline void ReverseList(Node* node, PointList& list) {
        list.clear();
        Node* current = node;
        do {
            list.insert(list.begin(), current->point);
        } while ((current = current->parent) != NULL);
    }

    void FindPath(Point const& start, Point const& end, Node** result, std::vector<Node*>& nodes, bool abs, const SearchOrigin& origin) {
        std::priority_queue<Node*, std::vector<Node*>, NodeComparer> open;
        std::set<Point> closed;
        PointList newNodes;
        // Checked before begin is allocated: GetPath frees only what nodes holds, so a return between the allocation
        // and the push below would leak it.
        if (!SearchOriginHolds(origin))
            return;
        Node* begin = alloc.allocate(1);

        // if we don't get a valid node, just return
        if (!begin)
            return;

        alloc.construct(begin, Node(start, NULL, 0, estimate(map, start, end)));
        nodes.push_back(begin);
        open.push(begin);
        DWORD ticks = GetTickCount();
        while (!open.empty()) {
            Node* current = open.top();
            open.pop();

            if (closed.find(current->point) != closed.end())
                continue;

            // this is to make d2 not completly freeze on long paths.
            if (GetTickCount() - ticks > 500) {
                map->AllowCritSpace();
                ticks = GetTickCount();
            }
            // Checked before the goal test too: the game may have moved on during the last expansion's callbacks, or
            // during AllowCritSpace just above.
            if (!SearchOriginHolds(origin)) {
                Log(L"Pather lvl change while pathing");
                return;
            }
            if (Vars.bQuitting || reducer->Failed())
                return;

            if (current->point == end) {
                *result = current;
                return;
            }

            bool result = closed.insert(current->point).second;
            assert(result == true);
            (void)(result); // shut up compiler about unused variable warning

            // getOpenNodes should be in map along with a filter
            reducer->GetOpenNodes(current->point, newNodes, end);
            while (!newNodes.empty()) {
                Point point = newNodes.back();
                newNodes.pop_back();

                if (point != end && reducer->Reject(point, abs)) {
                    if (reducer->Failed())
                        return;
                    closed.insert(point);
                    continue;
                }
                Node* next = alloc.allocate(1);
                // if we don't get a valid node, just return
                if (!next)
                    return;
                int pointPenalty = reducer->GetPenalty(point, abs);
                alloc.construct(next, Node(point, current, current->g + distance(current->point, point) + pointPenalty, estimate(map, point, end)));
                nodes.push_back(next);
                open.push(next);
            }
        }
    }

  public:
    AStarPath(const AStarPath&);
    AStarPath& operator=(const AStarPath&);
    AStarPath(Map* _map, Reducing::PathReducer* _reducer, Estimator _estimate = EstimateDistance, Distance _distance = DiagonalShortcut)
        : map(_map), reducer(_reducer), estimate(_estimate), distance(_distance), alloc(Allocator()) {
    }

    inline Allocator const& GetAllocator() {
        return alloc;
    }
    inline void SetMap(Map* map) {
        this->map = map;
    }
    inline void SetPathReducer(Reducing::PathReducer* reducer) {
        this->reducer = reducer;
    }

    void GetPath(Point const& _start, Point const& _end, PointList& list, bool abs = true) {
        Node* result = NULL;
        Point start = _start, end = _end;

        // Recorded before the first callback: a type-3 callback runs without the game lock, so a rejoin or act change
        // inside reject or mutate of the start or end must still stop the search.
        SearchOrigin origin;
        if (!CaptureSearchOrigin(origin))
            return;

        // if we don't have a valid start and end, try mutating the points
        if (reducer->Reject(start, abs))
            reducer->MutatePoint(start, abs);
        if (reducer->Reject(end, abs))
            reducer->MutatePoint(end, abs);

        // if they still get rejected, forget it
        // if(!map->IsValidPoint(start, abs) || !map->IsValidPoint(end, abs)) return;

        std::vector<Node*> nodes;
        FindPath(start, end, &result, nodes, abs, origin);
        if (result) {
            PointList in;
            ReverseList(result, in);
            reducer->Reduce(in, list, abs);
        } else
            list = PointList();

        std::vector<Node*>::iterator lbegin = nodes.begin(), lend = nodes.end();
        for (std::vector<Node*>::iterator it = lbegin; it != lend; it++) {
            alloc.destroy((*it));
            alloc.deallocate(*it, 1);
        }
    }
};

#pragma warning(default : 4512)

} // namespace Pathing
} // namespace Mapping
