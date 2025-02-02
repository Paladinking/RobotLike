#include <cstdint>
#include <utility>
#include <memory>


template<class T>
struct RefCountNode {
    std::unique_ptr<T> ptr;
    int64_t ref_count;
};

template<class T>
class RefCountSet {
public:
    using value_type = RefCountNode<T>;

    class Node {
        friend RefCountSet;

        Node(value_type value, Node* next, Node* prev) : value{std::move(value)}, next{next}, prev{prev} {}
        Node(): value{}, next{nullptr}, prev{nullptr} {}
    public:
        Node* next;
        Node* prev;
        value_type value {};
        void unlink() {
            prev->next = next; // prev is never nullptr
            if (next != nullptr) {
                next->prev = prev;
            }
        }
    };
private:
    Node* head;

    void release() {
        auto* first = head->next;
        while (first != nullptr) {
            first->value.ref_count = 0; // Disable ref_count
            first = first->next;
        }
        first = head->next;
        while (first != nullptr) {
            first->value.ptr.reset(); // Delete
            first = first->next;
        }

        Node* next = head;
        do {
            Node* n = next->next;
            delete next;
            next = n;
        } while (next != nullptr);
    }
public:
    RefCountSet() : head{new Node{}} {}

    Node* insert(value_type t) {
        Node* next = head->next;
        head->next = new Node{std::move(t), next, head};
        if (next != nullptr) {
            next->prev = head->next;
        }
        return head->next;
    }

    ~RefCountSet() {
        release();
    }

    Node* front() {
        return head->next;
    }

    void clear() {
        release();
        head = new Node{};
    }

    RefCountSet(const RefCountSet& other) = delete;
    RefCountSet(RefCountSet&& other) : head{other.head} {
        other.head = new Node{};
    }
    RefCountSet& operator=(const RefCountSet& other) = delete;
    RefCountSet& operator=(RefCountSet&& other) {
        if (this != &other) {
            release();
            head = other.head;
            other.head = new Node{}; 
        }
        return *this;
    }
};

template<class T>
class RefCounted {
    T* ptr;
    typename RefCountSet<T>::Node* node;

    void free() {
        if (node->value.ref_count > 0) {
            node->value.ref_count -= 1;
            if (node->value.ref_count == 0) {
                node->unlink();
                delete node;
            }
        }
    }
public:
    RefCounted(T* ptr, RefCountSet<T>& list) : ptr{ptr}, node{nullptr} {
        node = list.insert(RefCountNode<T>{std::unique_ptr<T>{ptr}, 1});
    }

    RefCounted(const RefCounted& other): ptr{other.ptr}, node{other.node} {
        node->value.ref_count += 1;
    }

    RefCounted(RefCounted&& other): ptr{other.ptr}, node{other.node} {
        node->value.ref_count += 1;
    }

    RefCounted& operator=(const RefCounted& other) {
        if (this != &other) {
            free();
            node = other.node;
            ptr = other.ptr;
            node->value.ref_count += 1;
        }
        return *this;
    }

    RefCounted& operator=(RefCounted&& other) {
        return *this = other;
    }

    ~RefCounted() {
        free();
    }

    T& operator*() {
        return *ptr;
    }
    const T& operator*() const {
        return *ptr;
    }
    T* operator->() {
        return ptr;
    }
    const T* operator->() const {
        return ptr;
    }
    T* get() {
        return ptr;
    }
    const T* get() const {
        return ptr;
    }
};

