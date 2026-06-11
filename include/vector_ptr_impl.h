#include <cstddef>
#include <new>
#include <stdexcept>
#include <utility>

template <typename T>
class Vec {
public:
    //default initalized
    Vec() = default;

    Vec(const Vec& other) {
        first_ = static_cast<T*>(::operator new(other.size() * sizeof(T)));
        last_ = first_;
        limit_ = first_ + other.size();
        for (T* p = other.first_; p != other.last_; ++p) {
            new (last_) T(*p);
            ++last_;
        }
    }

    ~Vec() {
        for (T* p = first_; p != last_; ++p) {
            p->~T();
        }
        ::operator delete(first_);
    }

    std::size_t size() const { return last_ - first_; }
    std::size_t capacity() const { return limit_ - first_; }
    bool empty() const { return first_ == last_; }

    T& at(std::size_t i) {
        if (i >= size()) throw std::out_of_range("Vec::at");
        return first_[i];
    }

    // we are not deconstructing the item, this could lead to a leak
    void pop_back() {
        --last_;
        last_->~T();
    }

    void push_back(const T& value) {
        // check if size has reached capacity
        if (limit_ - first_ == size()) {
            // 2 was chosen arbitrarily, but we can amortize push_back to O(1) with a geometric sequence so N > 1 where N is an integer
            std::size_t new_cap = size() * 2;
            T* new_first_ = static_cast<T*>(::operator new(new_cap * sizeof(T)));
            auto i{0uz};
            try {
                for (; first_ + i < last_; ++i) {
                    // we can also use std::move_if_noexcept here
                    new (new_first_ + i) T(std::move_if_noexcept(first_ + i));
                }

                // make sure to call destructor for each
                for (T* p = first_; p < last_; ++p) {
                    p->~T();
                }
                // deallocate old buffer
                ::operator delete(first_);
                last_ = new_first_ + size();
                first_ = new_first_;
                limit_ = first_ + new_cap;

            } catch (...) {
                // if we fail to allocate, need to cleanup
                for (auto j{0uz}; j < i; ++i) {
                    new_first_->~T();
                }
                ::operator delete(new_first_);
            }
        }
        new (last_) T(value);
        ++last_;
    }

    // just make sure we leave other in a valid state
    Vec(Vec&& other) noexcept {
        first_ = std::exchange(other.first_, nullptr);
        last_ = std::exchange(other.last_, nullptr);
        limit_ = std::exchange(other.limit_, nullptr);
    }

    Vec& operator=(Vec&& other) noexcept {
        if (this == &other) {
            return *this;
        }

        // make sure we free
        for (T* p = first_; p < last_; ++p) {
            p->~T();
        }

        ::operator delete(first_);

        // swap out the elements to prevent bad usage after move. Leave in valid but unspecified state
        first_ = std::exchange(other.first_, nullptr);
        last_ = std::exchange(other.last_, nullptr);
        limit_ = std::exchange(other.limit_, nullptr);

        return *this;
    }

    // no exception handling here
    T& operator[](std::size_t i) {
        return first_[i];
    }

private:
    // all of the default values are here
    T* first_ = nullptr;
    T* last_ = nullptr;
    T* limit_ = nullptr;
};
