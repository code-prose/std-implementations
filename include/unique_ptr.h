#include <utility>


namespace selfmade {
    template <typename T>
    struct DefaultDeleter {
        void operator()(T* p) const { delete p; }
    };

    template <typename T, typename Deleter = DefaultDeleter<T>>
    class UniquePtr {
    public:
        UniquePtr() = default;
        explicit UniquePtr(T* p, Deleter deleter) : data_{p}, deleter_{deleter} {}
        ~UniquePtr() { deleter_(data_); }
        UniquePtr(const UniquePtr& other) = delete;
        UniquePtr& operator=(const UniquePtr& other) = delete;
        UniquePtr(UniquePtr&& other) noexcept {
            data_ = std::exchange(other.data_, nullptr);
        }

        UniquePtr& operator=(UniquePtr&& other) noexcept {
            if (this == &other) {
                return *this;
            }
            deleter_(data_);
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
            deleter_(data_);
            data_ = p;
        }

        T& operator*() const { return *data_; };
        T* operator->() const { return data_; };


    private:
        T* data_{nullptr};
        [[no_unique_address]] Deleter deleter_;
    };

    template <typename T, typename... Args>
    UniquePtr<T> make_shared(Args&&... args) {
        return UniquePtr<T>(new T(std::forward<Args>(args)...));
    }
}
