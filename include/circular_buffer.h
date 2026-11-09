#include <utility>
#include <memory>

namespace around_the_world {
    template <typename T, std::size_t N>
    class CircularBuffer {
        public:
            CircularBuffer() { 
                auto clamped = std::bit_ceil(N);
                mask_ = clamped - 1;
                data_ = std::make_unique<T[]>(clamped);

            }
            ~CircularBuffer() = default;

            void write(const T& item) {
                if (full()) {
                    read_idx_ = (read_idx_ + 1) & mask_;
                } else {
                    size_++;
                }
                data_[write_idx_] = item;
                write_idx_ = (write_idx_ + 1) & mask_;
            }

            void write(T&& item) {
                if (full()) {
                    read_idx_ = (read_idx_ + 1) & mask_;
                } else {
                    size_++;
                }
                data_[write_idx_] = std::move(item);
                write_idx_ = (write_idx_ + 1) & mask_;
            }

            bool try_read(T& out) {
                if (empty()) {
                    return false;
                }
                out = data_[read_idx_];
                read_idx_ = (read_idx_ + 1) % N;
                size_--;
                return true;
            }

            bool empty() const noexcept { return size_ == 0; }
            bool full() const noexcept { return size_ == capacity_; }
            std::size_t capacity() const noexcept { return capacity_; }
            std::size_t size() const noexcept { return size_; }
        private:
            std::unique_ptr<T[]> data_;
            std::size_t size_{0};
            std::size_t capacity_{0};
            std::size_t mask_{0};
            std::size_t read_idx_{0};
            std::size_t write_idx_{0};
    };
}
