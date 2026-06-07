#include <stdexcept>
#include <print>
#include <memory>

namespace what_if_my_array_is_too_small {
    template <typename T>
    class Vector {
    public:
        Vector() = default;
        ~Vector() {
            for (auto i{0uz}; i < size_; i++) {
                data_[i].~T();
            }

            allocator_.deallocate(data_, capacity_);
        }

        void push_back(const T& item) {
            if (size_ == capacity_) {
                // this should just inline... right?
                std::size_t new_cap = new_capacity();
                T* temp_data_ = allocator_.allocate(new_cap);
                for (auto i{0uz}; i < size_; i++) {
                    new (temp_data_ + i) T(data_[i]);
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
    };
}
