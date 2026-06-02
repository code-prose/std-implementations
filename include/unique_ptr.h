#include <utility>

namespace selfmade {
    template <typename T>
    class UniquePtr {
    public:
        UniquePtr() = default;
        explicit UniquePtr(T* p) : data_{p} {}
        ~UniquePtr() { delete data_; }
        UniquePtr(const UniquePtr& other) = delete;
        UniquePtr& operator=(const UniquePtr& other) = delete;
        UniquePtr(UniquePtr&& other) noexcept {
            data_ = std::exchange(other.data_, nullptr);
        }

        UniquePtr& operator=(UniquePtr&& other) noexcept {
            if (this == &other) {
                return *this;
            }
            delete data_;
            data_ = std::exchange(other.data_, nullptr);
            return *this;

        }
        
        T* release() {
            T* to_return = std::exchange(data_, nullptr);
            return to_return;
        }

        T* get() const {
            return data_;
        }

        void reset(T* p = nullptr) {
            if (p == data_) {
                return;
            }
            delete data_;
            data_ = p;
        }

        T& operator*() const { return *data_; };
        T* operator->() const { return data_; };
        

    private:
        T* data_{nullptr};
    };

}
