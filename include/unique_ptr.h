#include <utility>

namespace woo_rewrite_uniq {
    template <typename T>
    struct DefaultDeleter {
        void operator()(T* p) { delete p; }
    };

    template <typename T, typename Deleter = DefaultDeleter<T>>
    class UniquePtr {
        public:
            UniquePtr() = default;
            explicit UniquePtr(T* p) : data_{p} {}
            UniquePtr(T* p, Deleter deleter) : data_{p}, deleter_{deleter} {}
            ~UniquePtr() { deleter_(data_); }

            UniquePtr(const UniquePtr& other) = delete;
            UniquePtr& operator=(const UniquePtr& other) = delete;

            UniquePtr(UniquePtr&& other) noexcept {
                data_ = std::exchange(other.data_, nullptr);
                deleter_ = std::move(other.deleter_);
            }

            UniquePtr& operator=(UniquePtr&& other) noexcept {
                if (this == &other) {
                    return *this;
                }
                deleter_(data_);
                data_ = std::exchange(other.data_, nullptr);
                deleter_ = std::move(other.deleter_);
                return *this;
            }


            void reset(T* p = nullptr) {
                if (p == data_) { 
                    return;
                }
                T* temp = data_;
                data_ = p;
                deleter_(data_);
            }

            T* get() const noexcept { return data_; }
            T* operator->() const noexcept { return data_; }
            T& operator*() const noexcept { return *data_; }
            T* release() noexcept { return std::exchange(data_, nullptr); }

        private:
            T* data_{nullptr};
            Deleter deleter_{};

    };

    template <typename T, typename... Args>
    UniquePtr<T> make_unique(Args&&... args) {
        return UniquePtr<T>(new T(std::forward<Args>(args)...));
    }

}
