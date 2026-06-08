#include <stdexcept>
#include <utility>
#include <memory>

namespace letitgrowletitgrowwww {
    template <typename T>
    class Vector {
        public:
            Vector() = default;
            ~Vector() {
                release();
                size_ = 0;
                capacity_ = 0;
            }

            Vector(const Vector& other) {
                allocator_ = other.allocator_;
                capacity_ = other.capacity_;
                size_ = other.size_;
                data_ = allocator_.allocate(capacity_);

                for (auto i{0uz}; i < size_; i++) {
                    new (data_ + i) T(other.data_[i]);
                }
            }

            Vector& operator=(const Vector& other) {
                if (this == &other) {
                    return *this;
                }
                release();

                allocator_ = other.allocator_;
                capacity_ = other.capacity_;
                size_ = other.size_;
                data_ = allocator_.allocate(capacity_);
                for (auto i{0uz}; i < size_; i++) {
                    new (data_ + i) T(other.data_[i]);
                }

                return *this;
            }

            Vector(Vector&& other) noexcept {
                data_ = std::exchange(other.data_, nullptr);
                size_ = std::exchange(other.size_, 0);
                capacity_ = std::exchange(other.capacity_, 0);
            }

            Vector& operator=(Vector&& other) noexcept {
                if (this == &other) {
                    return *this;
                }
                release();
                data_ = std::exchange(other.data_, nullptr);
                size_ = std::exchange(other.size_, 0);
                capacity_ = std::exchange(other.capacity_, 0);

                return *this;
            }

            void push_back(const T& item) {
                if (size_ == capacity_) {
                    auto new_cap = new_capacity();
                    T* new_data_ = allocator_.allocate(new_cap);
                    auto i{0uz};
                    try {
                        for (; i < size_; i++) {
                            new (new_data_ + i) T(std::move_if_noexcept(data_[i]));
                        }
                    } catch (...) {
                        for (auto j{0uz}; j < i; j++) {
                            new_data_[j].~T(); 
                        }
                        allocator_.deallocate(new_data_, new_cap);
                        throw;
                    }

                    // I release old data before assigning so I don't leak
                    release();
                    data_ = new_data_;
                    capacity_ = new_cap;
                    // make sure this ordering is right
                }

                new (data_ + size_) T(item);
                size_++;
            }

            T& operator[](std::size_t loc) noexcept {
                return data_[loc];
            }

            const T& operator[](std::size_t loc) const noexcept {
                return data_[loc];
            }

            T& at(std::size_t loc) {
                if (loc < size_) {
                    return data_[loc];
                } else {
                    throw std::out_of_range("Bad location index");
                }
            }

            const T& at(std::size_t loc) const {
                if (loc < size_) {
                    return data_[loc];
                } else {
                    throw std::out_of_range("Bad location index");
                }
            }

            std::size_t new_capacity() const noexcept { return capacity_ ? capacity_ * 2 : 1; }


        private:
            T* data_{nullptr};
            std::size_t size_{0uz};
            std::size_t capacity_{0uz};
            std::allocator<T> allocator_{};

            void release() {
                for (auto i{0uz}; i < size_; i++) {
                    data_[i].~T();
                }
                allocator_.deallocate(data_, capacity_);
            }

    };
}
