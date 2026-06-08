#include <atomic>
#include <utility>

namespace woo_rewrite {
    template <typename T>
    struct DefaultDeleter {
        void operator()(T* p) { delete p; }
    };

    struct ControlBlock {
        std::atomic<std::size_t> use_count_{1};
    };

    template <typename T, typename Deleter = DefaultDeleter<T>>
    class SharedPtr {
        public:
            SharedPtr() = default;
            explicit SharedPtr(T* p) : data_{p}, cb_{new ControlBlock{}}, deleter_{} {}
            SharedPtr(T* p, Deleter deleter) : data_{p}, cb_{new ControlBlock{}}, deleter_{deleter} {}
            ~SharedPtr() { free(); }

            SharedPtr(const SharedPtr& other) {
                cb_ = other.cb_;
                data_ = other.data_;
                deleter_ = other.deleter_;
                if (cb_) increment();
            }

            SharedPtr& operator=(const SharedPtr& other) {
                if (this == &other) {
                    return *this;
                }
                free();

                cb_ = other.cb_;
                data_ = other.data_;
                deleter_ = other.deleter_;
                if (cb_) increment();

                return *this;
            }

            SharedPtr(SharedPtr&& other) noexcept {
                cb_ = std::exchange(other.cb_, nullptr);
                data_ = std::exchange(other.data_, nullptr);
                deleter_ = std::move(other.deleter_);

            }

            SharedPtr& operator=(SharedPtr&& other) noexcept {
                if (this == &other) {
                    return *this;
                }
                free();

                cb_ = std::exchange(other.cb_, nullptr);
                data_ = std::exchange(other.data_, nullptr);
                deleter_ = std::move(other.deleter_);

                return *this;
            }



            void reset(T* p = nullptr) {
                free();
                data_ = p;
                cb_ = p ? new ControlBlock{} : nullptr;
            }

            T* get() const { return data_; }
            T* operator->() const { return data_; }
            T& operator*() const { return *data_; }

        private:
            T* data_{nullptr};
            ControlBlock* cb_{};
            Deleter deleter_{};

            void free() {
                if(cb_ && decrement() == 1) {
                    deleter_(data_);
                    delete cb_;
                }
                data_ = nullptr;
                cb_ = nullptr;
            }
            void increment() { cb_->use_count_.fetch_add(1); }
            std::size_t decrement() {
                return cb_->use_count_.fetch_sub(1);
            }
    };

    template <typename T, typename... Args>
    SharedPtr<T> make_shared(Args&&... args) {
        return SharedPtr<T>(new T(std::forward<Args>(args)...));
    }
}
