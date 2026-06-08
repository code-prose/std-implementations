#include <utility>
#include <memory>

namespace around_the_world {
    template <typename T, std::size_t N>
    class CircularBuffer {
        public:
            CircularBuffer() { data_ = std::make_unique<T[]>(N); }
            ~CircularBuffer() = default;

            void write(const T& item) {
                if (full()) {
                    read_idx_ = (read_idx_ + 1) % N;
                } else {
                    size_++;
                }
                data_[write_idx_] = item;
                write_idx_ = (write_idx_ + 1) % N;
            }

            void write(T&& item) {
                if (full()) {
                    read_idx_ = (read_idx_ + 1) % N;
                } else {
                    size_++;
                }
                data_[write_idx_] = std::move(item);
                write_idx_ = (write_idx_ + 1) % N;
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
            bool full() const noexcept { return size_ == N; }
            std::size_t capacity() const noexcept { return N; }
            std::size_t size() const noexcept { return size_; }
        private:
            std::unique_ptr<T[]> data_;
            std::size_t size_{0};
            std::size_t read_idx_{0};
            std::size_t write_idx_{0};
    };
}
