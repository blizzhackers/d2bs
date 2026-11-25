#pragma once

#ifndef __AUTOROOT_H__
#define __AUTOROOT_H__

#include "js32.h"

class AutoRoot {
  private:
    jsval var;
    int count;
    JSContext* cx;
    bool rooted;
    AutoRoot(const AutoRoot&);
    AutoRoot& operator=(const AutoRoot&);

  public:
    AutoRoot(JSContext* cx, jsval var);
    ~AutoRoot();
    void Take();
    void Release();
    // False when the value could not be rooted (out of memory, which the engine may not have reported): the caller
    // must delete this AutoRoot rather than keep the value.
    bool ok() const {
        return rooted;
    }
    jsval* value() {
        return &var;
    }
    jsval operator*() {
        return *value();
    }
    bool operator==(AutoRoot& other) {
        return value() == other.value();
    }
};

#endif
