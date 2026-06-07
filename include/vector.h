#include <stdexcept>
#include <print>
#include <memory>
#include <utility>

namespace what_if_my_array_is_too_small {
    template <typename T>
    class Vector {
    public:
        Vector() = default;
        // must do deep copy
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

            free();

            allocator_ = other.allocator_;
            capacity_ = other.capacity_;
            size_ = other.size_;

            data_ = allocator_.allocate(capacity_);
            for (auto i{0uz}; i < size_; i++) {
                new (data_ + i) T(other.data_[i]);
            }

            return *this;
        }

        // no copies, move objs
        Vector(Vector&& other) noexcept {
            allocator_ = std::move(other.allocator_);
            capacity_ = std::exchange(other.capacity_, 0);
            size_ = std::exchange(other.size_, 0);

            data_ = std::exchange(other.data_, nullptr);
        }

        Vector& operator=(Vector&& other) noexcept {
            if (this == &other) {
                return *this;
            }
            free();

            allocator_ = std::move(other.allocator_);
            capacity_ = std::exchange(other.capacity_, 0);
            size_ = std::exchange(other.size_, 0);

            data_ = std::exchange(other.data_, nullptr);

            return *this;
        }

        ~Vector() {
            for (auto i{0uz}; i < size_; i++) {
                data_[i].~T();
            }

            allocator_.deallocate(data_, capacity_);
        }

        void push_back(const T& item) {
            if (size_ == capacity_) {
                std::size_t new_cap = new_capacity();
                T* temp_data_ = allocator_.allocate(new_cap);
                auto curr{0uz};
                try {
                    for (; curr < size_; curr++) {
                        new (temp_data_ + curr) T(std::move_if_noexcept(data_[curr]));
                    }
                } catch (...) {
                    for (auto i{0uz}; i < curr; i++) {
                        temp_data_[i].~T();
                    } 
                    allocator_.deallocate(temp_data_, new_cap);
                    throw;
                }

                for (auto i{0uz}; i < size_; i++) {
                    data_[i].~T();
                }

                allocator_.deallocate(data_, capacity_);
                
                capacity_ = new_cap;
                data_ = temp_data_;
            }
            new (data_ + size_) T(item);
            size_++;
        }

        inline std::size_t new_capacity() {
            return capacity_ == 0 ? 1 : capacity_ * 2;
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
                throw std::out_of_range("Bad access");
            }
        }

        const T& at(std::size_t loc) const {
            if (loc < size_) {
                return data_[loc];
            } else {
                throw std::out_of_range("Bad access");
            }
        }

        std::size_t size() const { return size_; }
        std::size_t capacity() const { return capacity_; }
        bool empty() const { return size_ == 0; }

    private:
        T* data_{nullptr};
        std::allocator<T> allocator_{};
        std::size_t size_{0};
        std::size_t capacity_{0};

        void free() {
            for (auto i{0uz}; i < size_; i++) {
                data_[i].~T();
            }
            allocator_.deallocate(data_, capacity_);
            data_ = nullptr;
            size_ = 0;
            capacity_ = 0;
        }
    };
}
