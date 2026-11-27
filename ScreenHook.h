#pragma once

#include <list>
#include <algorithm>
#include <windows.h>

#include "Script.h"
#include "ScriptEngine.h"
#include "D2Helpers.h"

void DrawLogo(void);

typedef unsigned short ushort;

class Genhook;

typedef std::list<Genhook*> HookList;
typedef HookList::iterator HookIterator;

struct HookClickHelper {
    int button;
    POINT point;
};

typedef bool(__fastcall* HookCallback)(Genhook*, void*, uint);

enum Align { Left, Right, Center };
enum ScreenhookState { OOG, IG, Perm };

class Genhook {
  private:
    static bool init;
    static HookList visible, invisible;
    static CRITICAL_SECTION globalSection;
    static volatile LONG lastId;

  protected:
    Script* owner;
    ScreenhookState gameState;
    Align alignment;
    // Whether HOOK_SLOT_CLICK holds a function, so the game thread can decide on a click without the JS engine;
    // written and read under globalSection.
    bool hasClick;
    // Names the hook in its click events: by the time the owner runs one, the hook may be gone, and a later hook may
    // sit at the same address.
    DWORD id;
    JSObject* self;
    bool isAutomap, isVisible;
    ushort opacity, zorder;
    POINT location;

    Genhook(const Genhook&);
    Genhook& operator=(const Genhook&);

  public:
    Genhook::Genhook(Script* nowner, JSObject* nself, uint x, uint y, ushort nopacity, bool nisAutomap = false, Align nalign = Left, ScreenhookState ngameState = Perm);
    // Hooks are deleted through Genhook* (hook_finalize, hook_remove), so the derived destructors must run.
    virtual ~Genhook(void);

    friend bool __fastcall DrawHook(Genhook* hook, void* argv, uint argc);
    friend bool __fastcall CleanHook(Genhook* hook, void* argv, uint argc);

    static void DrawAll(ScreenhookState type);

    static bool ForEachHook(HookCallback proc, void* argv, uint argc);
    static bool ForEachVisibleHook(HookCallback proc, void* argv, uint argc);
    static bool ForEachInvisibleHook(HookCallback proc, void* argv, uint argc);
    // Runs the click handlers of the visible hooks under loc; true when one of them blocks the click.
    static bool ClickVisibleHooks(int button, POINT* loc);
    // Owner script thread only, inside a request: the click handler of owner's hook with this id, or JSVAL_VOID when
    // the hook is gone. The caller roots the result before anything can run the GC.
    static jsval GetClickHandler(Script* hookOwner, DWORD hookId);
    static void Clean(Script* owner);
    static void Initialize(void) {
        InitializeCriticalSection(&globalSection);
        init = true;
    }
    static void Destroy(void) {
        init = false;
        DeleteCriticalSection(&globalSection);
    }
    // For the hang report, which reads the lock's owner without taking it.
    static CRITICAL_SECTION* SectionForReport(void) {
        return &globalSection;
    }

  protected:
    virtual void Draw(void) = 0;

  public:
    Event* ClickEvent(int button, POINT* loc);

    bool IsInRange(POINT* pt) {
        return IsInRange(pt->x, pt->y);
    }
    virtual bool IsInRange(int dx, int dy) = 0;

    void Move(POINT* dist) {
        Move(dist->x, dist->y);
    }
    void Move(uint nx, uint ny) {
        SetX(GetX() + nx);
        SetY(GetY() + ny);
    }

    void SetX(uint x) {
        Lock();
        location.x = x;
        Unlock();
    }
    void SetY(uint y) {
        Lock();
        location.y = y;
        Unlock();
    }
    void SetAlign(Align nalign) {
        Lock();
        alignment = nalign;
        Unlock();
    }
    void SetOpacity(ushort nopacity) {
        Lock();
        opacity = nopacity;
        Unlock();
    }
    void SetIsVisible(bool nisVisible) {
        Lock();
        EnterCriticalSection(&globalSection);
        if (!nisVisible) {
            if (isVisible) {
                visible.remove(this);
                invisible.push_back(this);
            }
        } else {
            if (!isVisible) {
                invisible.remove(this);
                visible.push_back(this);
            }
        }
        isVisible = nisVisible;
        Unlock();
        LeaveCriticalSection(&globalSection);
    }
    // Publishes the hook to the draw and click lists; each hook constructor native calls it last, once the derived
    // object is complete, because the game thread calls the hook's virtual Draw and IsInRange from the lists.
    void Register(void);
    // The game thread sorts the lists by zorder under globalSection, and its sort needs the order stable while it runs.
    void SetZOrder(ushort norder) {
        EnterCriticalSection(&globalSection);
        zorder = norder;
        LeaveCriticalSection(&globalSection);
    }
    // Owner script thread only (ctors and setters): stores into the hook object's reserved slot; SetClickHandler also
    // updates hasClick.
    void SetClickHandler(JSContext* cx, jsval handler);
    void SetHoverHandler(JSContext* cx, jsval handler);

    POINT GetLocation(void) const {
        return location;
    }
    uint GetX(void) const {
        return location.x;
    }
    uint GetY(void) const {
        return location.y;
    }
    Align GetAlign(void) const {
        return alignment;
    }
    ushort GetOpacity(void) const {
        return opacity;
    }
    ScreenhookState GetGameState(void) const {
        return gameState;
    }
    bool GetIsAutomap(void) const {
        return isAutomap;
    }
    bool GetIsVisible(void) const {
        return isVisible;
    }
    ushort GetZOrder(void) const {
        return zorder;
    }
    static void EnterGlobalSection() {
        EnterCriticalSection(&globalSection);
    }
    static void LeaveGlobalSection() {
        LeaveCriticalSection(&globalSection);
    }

    void Lock() { /* EnterCriticalSection(&hookSection);*/
    }
    void Unlock() { /* LeaveCriticalSection(&hookSection);*/
    }
};

class TextHook : public Genhook {
  private:
    wchar_t* text;
    ushort font, color;

    TextHook(const TextHook&);
    TextHook& operator=(const TextHook&);

  public:
    TextHook(Script* owner, JSObject* nself, const wchar_t* text, uint x, uint y, ushort nfont, ushort ncolor, bool automap = false, Align align = Left,
             ScreenhookState state = Perm)
        : Genhook(owner, nself, x, y, 0, automap, align, state), text(NULL), font(nfont), color(ncolor) {
        // A failed copy leaves the string NULL; the constructor native checks it and deletes the hook.
        this->text = _wcsdup(text);
    }
    ~TextHook(void) {
        free(text);
    }

  protected:
    void Draw(void);

  public:
    bool IsInRange(int dx, int dy);

    void SetFont(ushort nfont) {
        Lock();
        font = nfont;
        Unlock();
    }
    void SetColor(ushort ncolor) {
        Lock();
        color = ncolor;
        Unlock();
    }
    bool SetText(const wchar_t* ntext);

    ushort GetFont(void) const {
        return font;
    }
    ushort GetColor(void) const {
        return color;
    }
    const wchar_t* GetText(void) const {
        return text;
    }
};

class ImageHook : public Genhook {
  private:
    wchar_t* location;
    CellFile* image;
    ushort color;

    ImageHook(const ImageHook&);
    ImageHook& operator=(const ImageHook&);

  public:
    ImageHook(Script* owner, JSObject* nself, const wchar_t* nloc, uint x, uint y, ushort ncolor, bool automap = false, Align align = Left, ScreenhookState state = Perm,
              bool fromFile = true)
        : Genhook(owner, nself, x, y, 0, automap, align, state), color(ncolor), image(NULL), location(NULL) {
        // A failed copy leaves the string NULL; the constructor native checks it and deletes the hook.
        location = _wcsdup(nloc);
        if (location)
            image = LoadCellFile(location, 3);
    }
    // The cell belongs to LoadCellFile's cache for the process lifetime; only the path copy is this hook's.
    ~ImageHook(void) {
        free(location);
    }

  protected:
    void Draw(void);

  public:
    bool IsInRange(int dx, int dy);

    bool SetImage(const wchar_t* nimage);
    void SetColor(ushort ncolor) {
        Lock();
        color = ncolor;
        Unlock();
    }

    const wchar_t* GetImage(void) const {
        return location;
    }
    ushort GetColor(void) const {
        return color;
    }
};

class LineHook : public Genhook {
  private:
    uint x2, y2;
    ushort color;

    LineHook(const LineHook&);
    LineHook& operator=(const LineHook&);

  public:
    LineHook(Script* owner, JSObject* nself, uint x, uint y, uint nx2, uint ny2, ushort ncolor, bool automap = false, Align align = Left, ScreenhookState state = Perm)
        : Genhook(owner, nself, x, y, 0, automap, align, state), x2(nx2), y2(ny2), color(ncolor) {
    }
    ~LineHook(void) {
    }

  protected:
    void Draw(void);

  public:
    bool IsInRange(int dx, int dy) {
        return false;
    }

    void SetX2(uint nx2) {
        Lock();
        x2 = nx2;
        Unlock();
    }
    void SetY2(uint ny2) {
        Lock();
        y2 = ny2;
        Unlock();
    }
    void SetColor(ushort ncolor) {
        Lock();
        color = ncolor;
        Unlock();
    }

    uint GetX2(void) const {
        return x2;
    }
    uint GetY2(void) const {
        return y2;
    }
    ushort GetColor(void) const {
        return color;
    }
};

class BoxHook : public Genhook {
  private:
    uint xsize, ysize;
    ushort color;

    BoxHook(const BoxHook&);
    BoxHook& operator=(const BoxHook&);

  public:
    BoxHook(Script* owner, JSObject* nself, uint x, uint y, uint nxsize, uint nysize, ushort ncolor, ushort opacity, bool automap = false, Align align = Left,
            ScreenhookState state = Perm)
        : Genhook(owner, nself, x, y, opacity, automap, align, state), xsize(nxsize), ysize(nysize), color(ncolor) {
    }
    ~BoxHook(void) {
    }

  protected:
    void Draw(void);

  public:
    bool IsInRange(int dx, int dy);

    void SetXSize(uint nxsize) {
        Lock();
        xsize = nxsize;
        Unlock();
    }
    void SetYSize(uint nysize) {
        Lock();
        ysize = nysize;
        Unlock();
    }
    void SetColor(ushort ncolor) {
        Lock();
        color = ncolor;
        Unlock();
    }

    uint GetXSize(void) const {
        return xsize;
    }
    uint GetYSize(void) const {
        return ysize;
    }
    ushort GetColor(void) const {
        return color;
    }
};

class FrameHook : public Genhook {
  private:
    uint xsize, ysize;

    FrameHook(const FrameHook&);
    FrameHook& operator=(const FrameHook&);

  public:
    FrameHook(Script* owner, JSObject* nself, uint x, uint y, uint nxsize, uint nysize, bool automap = false, Align align = Left, ScreenhookState state = Perm)
        : Genhook(owner, nself, x, y, 0, automap, align, state), xsize(nxsize), ysize(nysize) {
    }
    ~FrameHook(void) {
    }

  protected:
    void Draw(void);

  public:
    bool IsInRange(int dx, int dy);

    void SetXSize(uint nxsize) {
        Lock();
        xsize = nxsize;
        Unlock();
    }
    void SetYSize(uint nysize) {
        Lock();
        ysize = nysize;
        Unlock();
    }

    uint GetXSize(void) const {
        return xsize;
    }
    uint GetYSize(void) const {
        return ysize;
    }
};
