#include <utility>
#include <atomic>

namespace sharing_is_caring {
    template <typename T> class WeakPtr;

    template <typename T>
    struct DefaultDeleter {
        void operator()(T* p) { delete p; }
    };

    struct ControlBlock {
        std::atomic<std::size_t> strong_count_{1};
        std::atomic<std::size_t> weak_count_{1};
    };

    template <typename T, typename Deleter = DefaultDeleter<T>>
    class SharedPtr {
        public:
            friend class WeakPtr<T>;

            SharedPtr() = default;
            explicit SharedPtr(T* p) : data_{p}, cb_{new ControlBlock{}} {}
            SharedPtr(T* p, Deleter deleter) : data_{p}, cb_{new ControlBlock{}}, deleter_{deleter} {}
            ~SharedPtr() {
                release();
            }

            SharedPtr(const SharedPtr& other) {
                cb_ = other.cb_;
                data_ = other.data_;
                deleter_ = other.deleter_;
                increment();
            }

            SharedPtr& operator=(const SharedPtr& other) {
                if (this == &other) {
                    return *this;
                }
                release();

                cb_ = other.cb_;
                data_ = other.data_;
                deleter_ = other.deleter_;
                increment();

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

                release();
                cb_ = std::exchange(other.cb_, nullptr);
                data_ = std::exchange(other.data_, nullptr);
                deleter_ = std::move(other.deleter_);

                return *this;
            }

            std::size_t use_count() const { return cb_ ? cb_->strong_count_ : 0; }

            T* operator->() const noexcept { return data_; }
            T* get() const noexcept { return data_; }
            T& operator*() const noexcept { return *data_; }


        private:
            T* data_{nullptr};
            ControlBlock* cb_{nullptr};
            Deleter deleter_{};

            // adopting for weakptr
            SharedPtr(T* p, ControlBlock* cb) : data_{p}, cb_{cb} {}

            void release() {
                if (cb_ && decrement() == 1) {
                    deleter_(data_);
                    if (cb_->weak_count_.fetch_sub(1) == 1) {
                        delete cb_;
                    }
                }
                data_ = nullptr;
                cb_ = nullptr;
            }

            void increment() { if (cb_) cb_->strong_count_.fetch_add(1); }

            std::size_t decrement() {
                return cb_->strong_count_.fetch_sub(1);
            }
    };

    template <typename T, typename... Args>
    SharedPtr<T> make_shared(Args&&... args) {
        return SharedPtr<T>(new T(std::forward<Args>(args)...));
    }


    template <typename T>
    class WeakPtr {
        public:
            WeakPtr() = default;
            ~WeakPtr() {
                release();
            }

            WeakPtr(const WeakPtr& other) {
                cb_ = other.cb_;
                data_ = other.data_;
                increment();
            }

            WeakPtr& operator=(const WeakPtr& other) {
                if (this == &other) {
                    return *this;
                }
                release();

                cb_ = other.cb_;
                data_ = other.data_;
                increment();
                return *this;
            }

            WeakPtr(WeakPtr&& other) noexcept {
                cb_ = std::exchange(other.cb_, nullptr);
                data_ = std::exchange(other.data_, nullptr);
            }

            WeakPtr& operator=(WeakPtr&& other) noexcept {
                if (this == &other) {
                    return *this;
                }
                release();

                cb_ = std::exchange(other.cb_, nullptr);
                data_ = std::exchange(other.data_, nullptr);
                return *this;
            }

            WeakPtr(const SharedPtr<T>& shared) {
                cb_ = shared.cb_;
                data_ = shared.data_;
                increment();
            }

            std::size_t use_count() const { return cb_ ? cb_->strong_count_ : 0; }

            bool expired() { return !cb_ || cb_->strong_count_.load() == 0; }

            SharedPtr<T> lock() {
                if (!cb_) return SharedPtr<T>{};

                std::size_t curr = cb_->strong_count_.load();
                while (curr != 0) {
                    if (cb_->strong_count_.compare_exchange_weak(curr, curr + 1)) {
                        return SharedPtr<T>(data_, cb_);
                    }
                }
                return SharedPtr<T>{};
            }

        private:
            T* data_{nullptr};
            ControlBlock* cb_{nullptr};

            void increment() { if (cb_) cb_->weak_count_.fetch_add(1); }
            std::size_t decrement() { return cb_->weak_count_.fetch_sub(1); }
            void release() {
                if (cb_ && decrement() == 1) {
                    delete cb_;
                }
            }
    };
}
