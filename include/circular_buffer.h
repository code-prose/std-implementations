#include <memory>

namespace circ_buffer {
    template <typename T, std::size_t N>
    class CircularBuffer {
    public:
        CircularBuffer() : data_{std::make_unique<T[]>(N)} {}
       
        // could write a overload (T&& other)
        void write(const T& item) {
            if (full()) {
                read_idx_ = (read_idx_ + 1) % N;
            } else {
                size_++;
            }
            data_[write_idx_] = item;
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

        std::size_t size() const { return size_; }
        std::size_t capacity() const { return N; }
        bool full() const { return size_ == N; }
        bool empty() const { return size_ == 0; }
    private:
        std::unique_ptr<T[]> data_;
        std::size_t size_{0};
        std::size_t write_idx_{0};
        std::size_t read_idx_{0};
    };
}
