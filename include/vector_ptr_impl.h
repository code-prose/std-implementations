#include <cstddef>
#include <new>
#include <stdexcept>
#include <utility>

template <typename T>
class Vec {
public:
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

    const T& at(std::size_t i) const {
        if (i >= size()) throw std::out_of_range("Vec::at");
        return first_[i];
    }

    // int, size_t, std::string
    void pop_back() {
        --last_;
        last_->~T();
    }

    void cleanup() {
        for (T* p = first_; p != last_; ++p) {
            p->~T();
        }
        ::operator delete(first_);
    }

    void grow() {
        if (last_ == limit_) {
            std::size_t new_capacity = capacity() ? capacity() * 2 : 1;
            T* new_first_ = static_cast<T*>(::operator new(new_capacity * sizeof(T)));
            auto i{0uz};
            try {
                for (; first_ + i < last_; ++i) {
                    new (new_first_ + i) T(std::move_if_noexcept(first_[i]));
                }

                cleanup();
                last_ = new_first_ + size();
                first_ = new_first_;
                limit_ = first_ + new_capacity;

            } catch (...) {
               for (auto j{0uz}; j < i; ++j) {
                   new_first_[j].~T();
               } 
               ::operator delete(new_first_);
               throw;
            }
        }
    }

    void push_back(const T& value) {
        grow();
        new (last_) T(value);
        ++last_;
    }

    void push_back(T&& value) {
        grow();
        new (last_) T(std::move(value));
        ++last_;
    }

    template <typename... Args>
    void emplace_back(Args&&... args) {
        grow();
        new (last_) T(std::forward<Args>(args)...);
        ++last_;
    }

    Vec(Vec&& other) noexcept {
        first_ = std::exchange(other.first_, nullptr);
        last_ = std::exchange(other.last_, nullptr);
        limit_ = std::exchange(other.limit_, nullptr);
    }

    Vec& operator=(Vec&& other) noexcept {
        if (this == &other) {
            return *this;
        }


        cleanup();

        first_ = std::exchange(other.first_, nullptr);
        last_ = std::exchange(other.last_, nullptr);
        limit_ = std::exchange(other.limit_, nullptr);
        return *this;
    }

    T& operator[](std::size_t i) noexcept {
        return first_[i];
    }

    const T& operator[](std::size_t i) const noexcept {
        return first_[i];
    }

private:
    T* first_ = nullptr;
    T* last_ = nullptr;
    T* limit_ = nullptr;
};
