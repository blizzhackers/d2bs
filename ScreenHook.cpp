#include "ScreenHook.h"
#include "JSScreenHook.h"
#include "D2BS.h"
#include "Console.h"
#include "D2Ptrs.h"

#include <vector>

using namespace std;

bool Genhook::init = false;
HookList Genhook::visible = HookList();
HookList Genhook::invisible = HookList();
CRITICAL_SECTION Genhook::globalSection = {0};
ULONGLONG Genhook::lastId = 0;

void DrawLogo(void) {
    static wchar_t version[] = L"D2BS " D2BS_SHORT_VERSION;
    static int len = CalculateTextLen(version, 0).x;
    int dx = GetScreenSize().x - len - 1;
    int dy = GetScreenSize().y - 1;
    myDrawText(version, dx, dy, 4, 0);
}

bool zOrderSort(Genhook* first, Genhook* second) {
    return first->GetZOrder() < second->GetZOrder();
}

bool __fastcall DrawHook(Genhook* hook, void* argv, uint argc) {
    if ((hook->GetGameState() == (ScreenhookState)(int)argv || hook->GetGameState() == Perm) &&
        (!hook->GetIsAutomap() || (hook->GetIsAutomap() && *p_D2CLIENT_AutomapOn)))
        hook->Draw();
    return true;
}

bool __fastcall CleanHook(Genhook* hook, void* argv, uint argc) {
    if (hook->owner == (Script*)argv)
        hook->SetIsVisible(false);
    return true;
}

void Genhook::DrawAll(ScreenhookState type) {
    if (!init)
        return;
    ForEachVisibleHook(DrawHook, (void*)type, 1);
}

bool Genhook::ForEachHook(HookCallback proc, void* argv, uint argc) {
    // iterate the visible ones, then the invisible ones
    EnterCriticalSection(&globalSection);

    bool result = false;
    bool result2 = false;
    std::vector<Genhook*> list;
    for (HookIterator it = visible.begin(); it != visible.end(); it++)
        list.push_back(*it);
    int count = list.size();

    for (int i = 0; i < count; i++)
        if (proc(list[i], argv, argc))
            result = true;

    // if(!result)
    //{
    list.clear();
    for (HookIterator it = invisible.begin(); it != invisible.end(); it++)
        list.push_back(*it);
    count = list.size();

    for (int i = 0; i < count; i++)
        if (proc(list[i], argv, argc))
            result2 = true;
    //}

    LeaveCriticalSection(&globalSection);
    return (result ? true : (result2 ? true : false));
}

bool Genhook::ForEachVisibleHook(HookCallback proc, void* argv, uint argc) {
    // iterate the visible hooks
    EnterCriticalSection(&globalSection);

    bool result = false;
    std::vector<Genhook*> list;
    for (HookIterator it = visible.begin(); it != visible.end(); it++)
        list.push_back(*it);

    std::sort(list.begin(), list.end(), zOrderSort);
    int count = list.size();
    for (int i = 0; i < count; i++)
        if (proc(list[i], argv, argc))
            result = true;

    LeaveCriticalSection(&globalSection);
    return result;
}
bool Genhook::ForEachInvisibleHook(HookCallback proc, void* argv, uint argc) {
    // iterate the invisible hooks
    EnterCriticalSection(&globalSection);

    bool result = false;
    std::vector<Genhook*> list;
    for (HookIterator it = invisible.begin(); it != invisible.end(); it++)
        list.push_back(*it);
    int count = list.size();

    for (int i = 0; i < count; i++)
        if (proc(list[i], argv, argc))
            result = true;

    LeaveCriticalSection(&globalSection);
    return result;
}

// Unlists the hooks of the ending script and of every aborted script, in both lists alike, so the draw and click paths
// stop reaching them (a normally ended cached script keeps its hooks drawn); their finalizers delete them. A listed
// hook's owner is always live: the runtime's finalizers delete its hooks before ~Script frees the Script.
void Genhook::Clean(Script* owner) {
    if (!init)
        return;

    EnterCriticalSection(&globalSection);
    HookIterator it = visible.begin();
    while (it != visible.end()) {
        if ((*it)->owner == owner || (*it)->owner->IsAborted())
            it = visible.erase(it);
        else
            it++;
    }

    it = invisible.begin();
    while (it != invisible.end()) {
        if ((*it)->owner == owner || (*it)->owner->IsAborted())
            it = invisible.erase(it);
        else
            it++;
    }
    LeaveCriticalSection(&globalSection);
}

Genhook::Genhook(Script* nowner, JSObject* nself, uint x, uint y, ushort nopacity, bool nisAutomap, Align nalign, ScreenhookState ngameState)
    : owner(nowner), isAutomap(nisAutomap), isVisible(true), alignment(nalign), opacity(nopacity), gameState(ngameState), zorder(1) {
    hasClick = false;
    id = 0;
    self = nself;
    SetX(x);
    SetY(y);
}

void Genhook::Register(void) {
    EnterCriticalSection(&globalSection);
    id = ++lastId;
    if (isVisible)
        visible.push_back(this);
    else
        invisible.push_back(this);
    LeaveCriticalSection(&globalSection);
}

Genhook::~Genhook(void) {
    Lock();
    EnterCriticalSection(&globalSection);

    owner = NULL;
    location.x = -1;
    location.y = -1;
    if (isVisible)
        visible.remove(this);
    else
        invisible.remove(this);

    LeaveCriticalSection(&globalSection);

    Unlock();
}

static void FreeClickArgs(Event* evt) {
    delete (DWORD*)evt->arg1;
    delete (DWORD*)evt->arg2;
    delete (DWORD*)evt->arg3;
    delete (DWORD*)evt->arg4;
    delete (ULONGLONG*)evt->arg5;
}

// Runs on the game thread under globalSection, so it touches nothing of the owner's JS runtime: the owner's thread
// may be collecting garbage or adding roots at the same moment. The handler is looked up when the owner runs the event.
Event* Genhook::ClickEvent(int button, POINT* loc) {
    // A paused or ending owner runs no events, so the click would only wait out the time limit.
    if (!hasClick || !IsInRange(loc) || !owner || !owner->IsRunning())
        return NULL;

    Event* evt = new Event;
    evt->owner = owner;
    evt->argc = 3;
    evt->name = "ScreenHookClick";
    evt->arg1 = new DWORD((DWORD)button);
    evt->arg2 = new DWORD((DWORD)loc->x);
    evt->arg3 = new DWORD((DWORD)loc->y);
    evt->arg4 = new DWORD(false);
    evt->arg5 = new ULONGLONG(id);
    evt->freeArgs = FreeClickArgs;
    return evt;
}

// The lists hold hook objects weakly. A listed hook's object is still alive here because each collection of the owner's
// runtime runs to the end on this thread (no slice budget is set, so no GC is incremental) and its hook_finalize
// unlists every hook it collects; a sliced GC would need a weak-read check here.
jsval Genhook::GetClickHandler(Script* hookOwner, ULONGLONG hookId) {
    jsval handler = JSVAL_VOID;
    EnterCriticalSection(&globalSection);
    Genhook* hook = NULL;
    for (HookIterator it = visible.begin(); !hook && it != visible.end(); it++)
        if ((*it)->id == hookId && (*it)->owner == hookOwner)
            hook = *it;
    for (HookIterator it = invisible.begin(); !hook && it != invisible.end(); it++)
        if ((*it)->id == hookId && (*it)->owner == hookOwner)
            hook = *it;
    if (hook)
        handler = JS_GetReservedSlot(hook->self, HOOK_SLOT_CLICK);
    LeaveCriticalSection(&globalSection);
    return handler;
}

// The events are fired under globalSection, which keeps each hook and its owner alive, and waited on after leaving
// it: the wait lets go of the game lock, and retaking that while holding globalSection would invert the draw path's
// order (game lock, then globalSection) against a handler that touches a hook.
bool Genhook::ClickVisibleHooks(int button, POINT* loc) {
    std::vector<Event*> fired;
    EnterCriticalSection(&globalSection);
    std::vector<Genhook*> list(visible.begin(), visible.end());
    std::sort(list.begin(), list.end(), zOrderSort);
    for (size_t i = 0; i < list.size(); i++) {
        Event* evt = list[i]->ClickEvent(button, loc);
        if (evt) {
            FireBlocker(evt);
            fired.push_back(evt);
        }
    }
    LeaveCriticalSection(&globalSection);

    bool block = false;
    for (size_t i = 0; i < fired.size(); i++)
        if (WaitForBlocker(fired[i], 3000))
            block = true;
    return block;
}

namespace {
// A function sets a handler and null or undefined clears it; any other value is ignored without a throw.
bool IsHandlerValue(JSContext* cx, jsval v) {
    return JSVAL_IS_NULL(v) || JSVAL_IS_VOID(v) || JSVAL_IS_FUNCTION(cx, v);
}
} // namespace

void Genhook::SetClickHandler(JSContext* cx, jsval handler) {
    if (!IsHandlerValue(cx, handler))
        return;
    bool isFunction = JSVAL_IS_FUNCTION(cx, handler);
    JS_SetReservedSlot(self, HOOK_SLOT_CLICK, handler);
    EnterGlobalSection();
    hasClick = isFunction;
    LeaveGlobalSection();
}

void Genhook::SetHoverHandler(JSContext* cx, jsval handler) {
    if (!IsHandlerValue(cx, handler))
        return;
    JS_SetReservedSlot(self, HOOK_SLOT_HOVER, handler);
}

void TextHook::Draw(void) {
    Lock();
    if (GetIsVisible() && GetX() != -1 && GetY() != -1 && text) {
        uint x = GetX(), y = GetY(), w = CalculateTextLen(text, font).x;
        x -= (alignment != Center ? (alignment != Right ? 0 : w) : w / 2);
        POINT loc = {x, y};
        if (GetIsAutomap()) {
            loc = ScreenToAutomap(x, y);
        }
        EnterCriticalSection(&Vars.cTextHookSection);
        myDrawText((const wchar_t*)text, loc.x, loc.y, color, font);
        LeaveCriticalSection(&Vars.cTextHookSection);
    }
    Unlock();
}

bool TextHook::IsInRange(int dx, int dy) {
    Lock();
    POINT size = CalculateTextLen(text, font);
    int x = GetX(), y = GetY(), w = size.x, h = size.y, xp = x - (alignment != Center ? (alignment != Right ? 0 : w) : w / 2);
    Unlock();
    return (xp < dx && y > dy && (xp + w) > dx && (y - h) < dy);
}

// Copies before the swap so a failed copy keeps the current text; Draw reads the pointer under globalSection.
bool TextHook::SetText(const wchar_t* ntext) {
    wchar_t* copy = _wcsdup(ntext ? ntext : L"");
    if (!copy)
        return false;
    EnterGlobalSection();
    wchar_t* old = text;
    text = copy;
    LeaveGlobalSection();
    free(old);
    return true;
}

void ImageHook::Draw(void) {
    Lock();
    if (GetIsVisible() && GetX() != -1 && GetY() != -1 && GetImage() != NULL && image != NULL) {
        uint x = GetX(), y = GetY(), w = image->cells[0]->width;
        x += (alignment != Left ? (alignment != Right ? 0 : -1 * (w / 2)) : w / 2);
        POINT loc = {x, y};
        if (GetIsAutomap()) {
            loc = ScreenToAutomap(x, y);
        }
        EnterCriticalSection(&Vars.cImageHookSection);
        myDrawAutomapCell(image, loc.x, loc.y, (BYTE)color);
        LeaveCriticalSection(&Vars.cImageHookSection);
    }
    Unlock();
}

bool ImageHook::IsInRange(int dx, int dy) {
    if (image) {
        Lock();
        int x = GetX();
        int y = GetY();
        int w = image->cells[0]->width;
        int h = image->cells[0]->height;
        int xp = x - (alignment != Left ? (alignment != Right ? w / 2 : w) : -1 * w);
        int yp = y - (h / 2);
        Unlock();
        return (xp < dx && yp < dy && (xp + w) > dx && (yp + h) > dy);
    }

    return false;
}

// The new name and cell are made before taking globalSection, and both pointers swap under it: Draw and IsInRange
// read them there on the game thread. The cache owns every cell, so only the old name is freed.
bool ImageHook::SetImage(const wchar_t* nimage) {
    wchar_t* newLocation = _wcsdup(nimage);
    if (!newLocation)
        return false;
    CellFile* newImage = LoadCellFile(newLocation, 3);
    EnterGlobalSection();
    wchar_t* oldLocation = location;
    location = newLocation;
    image = newImage;
    LeaveGlobalSection();
    free(oldLocation);
    return true;
}

void LineHook::Draw(void) {
    Lock();
    if (GetIsVisible() && GetX() != -1 && GetY() != -1) {
        uint x = GetX(), y = GetY(), x2 = GetX2(), y2 = GetY2();
        POINT loc = {x, y};
        POINT sz = {x2, y2};
        if (GetIsAutomap()) {
            loc = ScreenToAutomap(x, y);
            sz = ScreenToAutomap(x2, y2);
        }
        EnterCriticalSection(&Vars.cLineHookSection);
        D2GFX_DrawLine(loc.x, loc.y, sz.x, sz.y, color, 0xFF);
        LeaveCriticalSection(&Vars.cLineHookSection);
    }
    Unlock();
}

void BoxHook::Draw(void) {
    Lock();
    if (GetIsVisible() && GetX() != -1 && GetY() != -1) {
        uint x = GetX(), y = GetY(), x2 = GetXSize(), y2 = GetYSize();
        if (alignment == Center) {
            x -= x2 / 2;
        } else if (alignment == Right) {
            x += x2 / 2;
        }
        POINT loc = {x, y};
        POINT sz = {x + x2, y + y2};
        if (GetIsAutomap()) {
            loc = ScreenToAutomap(x, y);
            sz = ScreenToAutomap(x + x2, y + y2);
        }
        EnterCriticalSection(&Vars.cBoxHookSection);
        D2GFX_DrawRectangle(loc.x, loc.y, sz.x, sz.y, color, opacity);
        LeaveCriticalSection(&Vars.cBoxHookSection);
    }
    Unlock();
}

bool BoxHook::IsInRange(int dx, int dy) {
    Lock();
    int x = GetX(), y = GetY(), x2 = GetXSize(), y2 = GetYSize();
    Unlock();
    return (x < dx && y < dy && (x + x2) > dx && (y + y2) > dy);
}

void FrameHook::Draw(void) {
    Lock();
    if (GetIsVisible() && GetX() != -1 && GetY() != -1) {
        uint x = GetX(), y = GetY(), x2 = GetXSize(), y2 = GetYSize();
        if (alignment == Center) {
            x -= x2 / 2;
        } else if (alignment == Right) {
            x += x2 / 2;
        }
        RECT rect = {x, y, x + x2, y + y2};
        EnterCriticalSection(&Vars.cFrameHookSection);
        D2GFX_DrawFrame(&rect);
        LeaveCriticalSection(&Vars.cFrameHookSection);
    }
    Unlock();
}

bool FrameHook::IsInRange(int dx, int dy) {
    Lock();
    int x = GetX(), y = GetY(), x2 = GetXSize(), y2 = GetYSize();
    Unlock();
    return (x < dx && y < dy && (x + x2) > dx && (y + y2) > dy);
}
