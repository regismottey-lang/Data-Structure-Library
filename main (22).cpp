// Generic Data Structures Library: LinkedList, HashMap, BST + self-contained tests
// Demonstrates: class templates, iterators, move semantics, hashing, recursion, testing.
#include <algorithm>
#include <cassert>
#include <functional>
#include <iostream>
#include <list>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

// ======================= LinkedList<T> (singly linked) =======================
template <typename T>
class LinkedList {
    struct Node {
        T value;
        std::unique_ptr<Node> next;
        explicit Node(T v) : value(std::move(v)) {}
    };

public:
    class Iterator {
    public:
        explicit Iterator(Node* n) : n_(n) {}
        T& operator*() const { return n_->value; }
        Iterator& operator++() { n_ = n_->next.get(); return *this; }
        bool operator!=(const Iterator& o) const { return n_ != o.n_; }
    private:
        Node* n_;
    };

    LinkedList() = default;
    LinkedList(const LinkedList&) = delete;
    LinkedList& operator=(const LinkedList&) = delete;
    LinkedList(LinkedList&&) noexcept = default;
    LinkedList& operator=(LinkedList&&) noexcept = default;

    // Iterative teardown avoids stack overflow from recursive unique_ptr destruction.
    ~LinkedList() { while (head_) head_ = std::move(head_->next); }

    void pushFront(T v) {
        auto n = std::make_unique<Node>(std::move(v));
        n->next = std::move(head_);
        head_ = std::move(n);
        ++size_;
    }

    void pushBack(T v) {
        auto n = std::make_unique<Node>(std::move(v));
        std::unique_ptr<Node>* cur = &head_;
        while (*cur) cur = &(*cur)->next;  // walk to the empty tail slot
        *cur = std::move(n);
        ++size_;
    }

    void reverse() {
        std::unique_ptr<Node> prev;
        while (head_) {
            auto next = std::move(head_->next);
            head_->next = std::move(prev);
            prev = std::move(head_);
            head_ = std::move(next);
        }
        head_ = std::move(prev);
    }

    std::size_t size() const { return size_; }
    Iterator begin() const { return Iterator(head_.get()); }
    Iterator end() const { return Iterator(nullptr); }

private:
    std::unique_ptr<Node> head_;
    std::size_t size_ = 0;
};

// ======================= HashMap<K,V> (separate chaining) =======================
template <typename K, typename V, typename Hash = std::hash<K>>
class HashMap {
public:
    explicit HashMap(std::size_t buckets = 8) : buckets_(buckets) {}

    void put(const K& key, V value) {
        auto& existing = buckets_[index(key, buckets_.size())];
        for (auto& kv : existing)
            if (kv.first == key) { kv.second = std::move(value); return; }
        if (static_cast<double>(size_ + 1) / buckets_.size() > 0.75) rehash(buckets_.size() * 2);
        buckets_[index(key, buckets_.size())].emplace_back(key, std::move(value));
        ++size_;
    }

    std::optional<V> get(const K& key) const {
        for (const auto& kv : buckets_[index(key, buckets_.size())])
            if (kv.first == key) return kv.second;
        return std::nullopt;
    }

    bool erase(const K& key) {
        auto& chain = buckets_[index(key, buckets_.size())];
        for (auto it = chain.begin(); it != chain.end(); ++it)
            if (it->first == key) { chain.erase(it); --size_; return true; }
        return false;
    }

    std::size_t size() const { return size_; }

private:
    std::size_t index(const K& key, std::size_t n) const { return Hash{}(key) % n; }

    void rehash(std::size_t newCount) {
        std::vector<std::list<std::pair<K, V>>> fresh(newCount);
        for (auto& chain : buckets_)
            for (auto& kv : chain)
                fresh[index(kv.first, newCount)].push_back(std::move(kv));
        buckets_ = std::move(fresh);
    }

    std::vector<std::list<std::pair<K, V>>> buckets_;
    std::size_t size_ = 0;
};

// ======================= BST<T> =======================
template <typename T, typename Compare = std::less<T>>
class BST {
    struct Node {
        T value;
        std::unique_ptr<Node> left, right;
        explicit Node(T v) : value(std::move(v)) {}
    };

public:
    bool insert(T v) { return insert(root_, std::move(v)); }

    bool contains(const T& v) const {
        const Node* n = root_.get();
        while (n) {
            if (cmp_(v, n->value)) n = n->left.get();
            else if (cmp_(n->value, v)) n = n->right.get();
            else return true;
        }
        return false;
    }

    std::vector<T> inOrder() const {
        std::vector<T> out;
        walk(root_.get(), out);
        return out;
    }

    int height() const { return height(root_.get()); }

private:
    bool insert(std::unique_ptr<Node>& n, T v) {
        if (!n) { n = std::make_unique<Node>(std::move(v)); return true; }
        if (cmp_(v, n->value)) return insert(n->left, std::move(v));
        if (cmp_(n->value, v)) return insert(n->right, std::move(v));
        return false;  // duplicate
    }

    void walk(const Node* n, std::vector<T>& out) const {
        if (!n) return;
        walk(n->left.get(), out);
        out.push_back(n->value);
        walk(n->right.get(), out);
    }

    int height(const Node* n) const {
        return n ? 1 + std::max(height(n->left.get()), height(n->right.get())) : 0;
    }

    std::unique_ptr<Node> root_;
    Compare cmp_;
};

// ======================= Tests =======================
void testLinkedList() {
    LinkedList<int> l;
    l.pushBack(2);
    l.pushBack(3);
    l.pushFront(1);
    assert(l.size() == 3);
    std::vector<int> v;
    for (int x : l) v.push_back(x);
    assert((v == std::vector<int>{1, 2, 3}));
    l.reverse();
    v.clear();
    for (int x : l) v.push_back(x);
    assert((v == std::vector<int>{3, 2, 1}));
    std::cout << "LinkedList OK\n";
}

void testHashMap() {
    HashMap<std::string, int> m;
    for (int i = 0; i < 100; ++i) m.put("key" + std::to_string(i), i);  // forces rehashing
    assert(m.size() == 100);
    assert(m.get("key42").value() == 42);
    assert(!m.get("missing").has_value());
    m.put("key42", -1);
    assert(m.get("key42").value() == -1 && m.size() == 100);
    assert(m.erase("key42"));
    assert(!m.erase("key42"));
    assert(m.size() == 99);
    std::cout << "HashMap OK\n";
}

void testBST() {
    BST<int> t;
    for (int x : {5, 3, 8, 1, 4, 7, 9}) assert(t.insert(x));
    assert(!t.insert(5));
    assert(t.contains(4) && !t.contains(6));
    assert((t.inOrder() == std::vector<int>{1, 3, 4, 5, 7, 8, 9}));
    assert(t.height() == 3);
    BST<int, std::greater<int>> desc;  // custom comparator
    for (int x : {1, 2, 3}) desc.insert(x);
    assert((desc.inOrder() == std::vector<int>{3, 2, 1}));
    std::cout << "BST OK\n";
}

int main() {
    testLinkedList();
    testHashMap();
    testBST();
    std::cout << "All tests passed\n";
}


