#include <atomic>
#include <utility>

namespace sharing_is_caring {
    template <typename T>
    struct DefaultDeleter {
        void operator()(T* p) { delete p; }
    };

    struct ControlBlock {
        std::atomic<size_t> use_count_{1};
    };

    template <typename T, typename Deleter = DefaultDeleter<T>>
    class SharedPtr {
        public:
            SharedPtr() = default;
            explicit SharedPtr(T* p) : data_{p}, deleter_{}, cb_{new ControlBlock{}} {}
            SharedPtr(T* p, Deleter deleter) : data_{p}, deleter_{deleter}, cb_{new ControlBlock{}} {}
            ~SharedPtr() { 
                release();
            }

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
                release();

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
            } // move ctor.. nothing exists before

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

            void increment() {
                cb_->use_count_.fetch_add(1); 
            }

            T* operator->() const { return data_; }
            T* get() const { return data_; }
            T& operator*() const { return *data_; }
            // I might want to use a memory ordering to read this
            std::size_t use_count() const { return std::atomic_load_explicit(&cb_->use_count_, std::memory_order_seq_cst); };

            void release() {
                if (cb_ && cb_->use_count_.fetch_sub(1) == 1) {
                    deleter_(data_);
                    delete cb_;
                }
            }

        private:
            T* data_{nullptr};
            ControlBlock* cb_{};
            Deleter deleter_{};
    };
}
