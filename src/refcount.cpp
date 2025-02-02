#include "refcount.h"
#include <vector>
#include <iostream>

struct Thing {
    Thing(std::string name) : name{std::move(name)}, stuff{} {}
    std::string name{};
    std::vector<RefCounted<Thing>> stuff{};

    ~Thing() {
        std::cout << name << " deleted" << std::endl;
    }
};

int main() {
    RefCountSet<Thing> elems{};

    {
        RefCounted<Thing> a{new Thing{"a"}, elems};
        RefCounted<Thing> b{new Thing{"b"}, elems};
        RefCounted<Thing> c{new Thing{"c"}, elems};
        RefCounted<Thing> d{new Thing{"d"}, elems};
        a->stuff.push_back(b);
        b->stuff.push_back(a);

    }
    std::cout << "Scope ended" << std::endl;
    elems = RefCountSet<Thing>();
    {
        RefCounted<Thing> a{new Thing{"a"}, elems};
        RefCounted<Thing> b{new Thing{"b"}, elems};
        RefCounted<Thing> c{new Thing{"c"}, elems};
        RefCounted<Thing> d{new Thing{"d"}, elems};
        a->stuff.push_back(b);
        b->stuff.push_back(a);
        c->stuff.push_back(a);
        d->stuff.push_back(a);
        b->stuff.push_back(b);
        b->stuff.push_back(c);
        b->stuff.push_back(d);
    }
    std::cout << "Scope ended" << std::endl;
    //RefCounted<Thing>::release(elems);
}
