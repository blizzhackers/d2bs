#include "AutoRoot.h"
#include "ScriptEngine.h"

AutoRoot::AutoRoot(JSContext* ncx, jsval nvar) : var(nvar), count(0), cx(ncx), rooted(false) {
    Take();
}
// Removes the root even when ok() is false: removing a root never added does nothing.
AutoRoot::~AutoRoot() {
    if (count < 0) {
        fprintf(stderr, "AutoRoot failed: Count is still %i, but the root is being destroyed", count);
        DebugBreak();
        exit(3);
    }
    // JS_RemoveRoot(cx, &var);
    JS_BeginRequest(cx);
    JS_RemoveValueRoot(cx, &var);
    JS_EndRequest(cx);
}

void AutoRoot::Take() {
    count++;
    rooted = JS_AddNamedValueRoot(cx, &var, "AutoRoot") != JS_FALSE;
}

void AutoRoot::Release() {
    count--;
    if (count < 0) {
        fprintf(stderr, "Improper AutoRoot usage: Count is less than 0");
        DebugBreak();
        exit(3);
    }
}
