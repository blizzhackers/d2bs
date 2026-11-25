#pragma once

#include "PathReducer.h"
#include "../Path.h"
#include "../../Diablo_II/ActMap.h"
#include "../AStarPath.h"
#include "../../../js32.h"
#include <map>

namespace Mapping {
namespace Pathing {
namespace Reducing {

#pragma warning(disable : 4512)

class JSPathReducer : public PathReducer {
  private:
    struct LevelRect {
        DWORD x0, y0, x1, y1;
    };
    ActMap* map;
    JSContext* cx;
    // The functions and this stay rooted as getPath's own arguments, so a reducer must not outlive that call.
    JSObject* obj;
    jsval reject, reduce, mutate;
    // The current act's levels, copied under the game lock: the search never leaves them, as types 0 and 1 can't.
    std::vector<LevelRect> levels;
    bool failed_;
    // Where the search started, captured under the game lock before any callback.
    SearchOrigin origin_;
    // Set when no origin could be captured, the player left the search's origin during a callback, or the start or end
    // lies outside the act's levels after mutate: no further script call is made and getPath returns an empty path.
    // A stop leaves nothing pending; when a callback also failed, failed_ is set too and its error wins.
    bool stopped_;
    // reject's answer for each point it was asked about: it is asked at most once per point, which the docs promise.
    std::map<Point, bool> verdicts_;
    bool pausedLogged_;

    // DWORD compare, as ActMap::isPointInLevel.
    bool InLevels(Point const& pt) const {
        DWORD x = pt.first, y = pt.second;
        for (std::vector<LevelRect>::const_iterator it = levels.begin(); it != levels.end(); it++)
            if (x >= it->x0 && y >= it->y0 && x < it->x1 && y < it->y1)
                return true;
        return false;
    }
    // The game lock is given up only through CleanUp, which first unloads the rooms this map loaded and empties its
    // caches, so the game never runs with them loaded and a callback's own getPath starts from the game's state. A thread
    // that held the lock before getPath still holds it here: the game stays paused through the callbacks, and is logged once.
    void ReleaseGame() {
        map->CleanUp();
        if (!pausedLogged_ && HoldsGameLock()) {
            pausedLogged_ = true;
            Log(L"getPath: reject, mutate and reduce run with the game paused (this thread already held the game lock)");
        }
    }
    // The game may have run during the callback: a player who left the search's origin ends the search.
    void RetakeGame() {
        map->actCrit->EnterSection();
        if (!stopped_ && !SearchOriginHolds(origin_))
            stopped_ = true;
    }
    // A native passed as a callback can report an error and still return success, so an exception that appears
    // during a call fails it too; one already pending came from an earlier statement and is left to it.
    bool Raised(bool pendingBefore) const {
        return !pendingBefore && JS_IsExceptionPending(cx);
    }
    static const JSErrorFormatString* MutateError(void*, const char*, const unsigned) {
        static const JSErrorFormatString format = {"getPath mutate must return [x, y] with two numbers in the 32-bit integer range", 0, (int16_t)JSEXN_TYPEERR};
        return &format;
    }
    // Runs without the search's hold on the game lock: the element reads run getters and the conversions run valueOf,
    // both script.
    bool CallMutate(int& x, int& y) {
        jsval rval = JSVAL_VOID;
        jsval argv[] = {INT_TO_JSVAL(x), INT_TO_JSVAL(y)};
        const bool pending = !!JS_IsExceptionPending(cx);
        if (!JS_CallFunctionValue(cx, obj, mutate, 2, argv, &rval) || Raised(pending))
            return false;
        // A result that isn't an object keeps the point.
        if (JSVAL_IS_PRIMITIVE(rval))
            return true;
        JSObject* res = JSVAL_TO_OBJECT(rval);
        double d[2];
        for (uint32_t i = 0; i < 2; i++) {
            jsval v = JSVAL_VOID;
            if (!JS_GetElement(cx, res, i, &v) || !JS_ValueToNumber(cx, v, &d[i]) || Raised(pending))
                return false;
            // NaN and the infinities fail this test too; a double outside int range has no defined cast.
            if (!(d[i] > -2147483649.0 && d[i] < 2147483648.0)) {
                JS_ReportErrorNumber(cx, MutateError, NULL, 0);
                return false;
            }
        }
        x = (int)d[0];
        y = (int)d[1];
        return true;
    }

  public:
    JSPathReducer(const JSPathReducer&);
    JSPathReducer& operator=(const JSPathReducer&);
    JSPathReducer(ActMap* m, JSContext* _cx, JSObject* _obj, jsval _reject, jsval _reduce, jsval _mutate)
        : map(m), cx(_cx), obj(_obj), reject(_reject), reduce(_reduce), mutate(_mutate), failed_(false), stopped_(false), pausedLogged_(false) {
        // No origin means GetPath returns before any callback too; stopping here keeps the two in step.
        if (!CaptureSearchOrigin(origin_))
            stopped_ = true;
        const Level* level = m->GetLevel();
        if (!level || !level->pMisc)
            return;
        for (const Level* lvl = level->pMisc->pLevelFirst; lvl; lvl = lvl->pNextLevel) {
            if (lvl->dwPosX == (DWORD)-1 || lvl->dwPosY == (DWORD)-1)
                continue;
            LevelRect r;
            r.x0 = lvl->dwPosX * 5;
            r.y0 = lvl->dwPosY * 5;
            r.x1 = r.x0 + lvl->dwSizeX * 5;
            r.y1 = r.y0 + lvl->dwSizeY * 5;
            levels.push_back(r);
        }
    }

    bool Failed() const {
        return failed_;
    }
    bool Stopped() const {
        return stopped_;
    }

    void Reduce(PointList const& in, PointList& out, bool abs) {
        // The last game-lock use of the search: the points are built and the callbacks run without the search's hold.
        ReleaseGame();
        if (failed_ || stopped_)
            return;

        // create the initial array to pass to the js function
        int count = in.size();

        // The points stay rooted through the callbacks: building a later point can run the GC, and a callback may
        // empty the array it is passed.
        JS::AutoValueVector vec(cx);
        if (!vec.resize(count)) {
            failed_ = true;
            return;
        }
        for (int i = 0; i < count; i++) {
            jsval x = INT_TO_JSVAL(in[i].first), y = INT_TO_JSVAL(in[i].second);

            JSObject* pt = BuildObject(cx);
            if (!pt) {
                failed_ = true;
                return;
            }
            vec[i] = OBJECT_TO_JSVAL(pt);
            if (!JS_SetProperty(cx, pt, "x", &x) || !JS_SetProperty(cx, pt, "y", &y)) {
                failed_ = true;
                return;
            }
        }
        JSObject* arr = JS_NewArrayObject(cx, count, vec.begin());
        if (!arr) {
            failed_ = true;
            return;
        }

        jsval argv[] = {JSVAL_NULL, JSVAL_ZERO, OBJECT_TO_JSVAL(arr)};
        for (int i = 0; i < count; i++) {
            jsval rval = JSVAL_FALSE;
            argv[0] = vec[i];
            argv[1] = INT_TO_JSVAL(i);
            const bool pending = !!JS_IsExceptionPending(cx);
            if (!JS_CallFunctionValue(cx, obj, reduce, 3, argv, &rval) || Raised(pending)) {
                failed_ = true;
                return;
            }
            if (JS::ToBoolean(rval))
                out.push_back(in[i]);
        }
    }
    bool Reject(Point const& pt, bool abs) {
        // After a failure or a stop every point is blocked, so the search ends without calling script again.
        if (failed_ || stopped_ || !InLevels(pt))
            return true;
        std::map<Point, bool>::const_iterator known = verdicts_.find(pt);
        if (known != verdicts_.end())
            return known->second;
        jsval rval = JSVAL_FALSE;
        jsval argv[] = {INT_TO_JSVAL(pt.first), INT_TO_JSVAL(pt.second)};
        ReleaseGame();
        const bool pending = !!JS_IsExceptionPending(cx);
        bool ok = JS_CallFunctionValue(cx, obj, reject, 2, argv, &rval) && !Raised(pending);
        RetakeGame();
        if (!ok) {
            failed_ = true;
            return true;
        }
        if (stopped_)
            return true;
        const bool blocked = !!JS::ToBoolean(rval);
        verdicts_[pt] = blocked;
        return blocked;
    }
    void GetOpenNodes(Point const& center, PointList& out, Point const& endpoint) {
        for (int i = 1; i >= -1; i--) {
            for (int j = 1; j >= -1; j--) {
                if (i == 0 && j == 0)
                    continue;
                out.push_back(Point(center.first + i, center.second + j));
            }
        }
    }
    int GetPenalty(Point const& pt, bool abs) {
        return 0;
    }
    void MutatePoint(Point& pt, bool abs) {
        if (failed_ || stopped_)
            return;
        int x = pt.first, y = pt.second;
        ReleaseGame();
        bool ok = CallMutate(x, y);
        RetakeGame();
        if (!ok) {
            failed_ = true;
            return;
        }
        if (stopped_)
            return;
        // A start or end outside the current act's levels, moved there or kept, has no path to or from it.
        if (!InLevels(Point(x, y))) {
            stopped_ = true;
            return;
        }
        pt.first = x;
        pt.second = y;
    }
};

#pragma warning(default : 4512)

} // namespace Reducing
} // namespace Pathing
} // namespace Mapping
