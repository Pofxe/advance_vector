#pragma once
#include <cassert>
#include <memory>
#include <algorithm>
#include <cstdlib>
#include <new>
#include <utility>
#include <iterator>

template <typename T>
class RawMemory
{
public:

    RawMemory() = default;

    explicit RawMemory(size_t capacity_) : buffer(Allocate(capacity_)), capacity(capacity_) {}

    RawMemory(RawMemory&& other_) noexcept : buffer(std::exchange(other_.buffer, nullptr)), capacity(std::exchange(other_.capacity, 0)) {}

    RawMemory(const RawMemory&) = delete;

    ~RawMemory()
    {
        Deallocate(buffer);
    }

    RawMemory& operator=(const RawMemory& rhs) = delete;

    RawMemory& operator=(RawMemory&& rhs_) noexcept
    {
        if (this != &rhs_)
        {
            buffer = std::move(rhs_.buffer);
            capacity = std::move(rhs_.capacity);
            rhs_.buffer = nullptr;
            rhs_.capacity = 0;
        }
        return *this;
    }

    T* operator+(size_t offset_) noexcept
    {
        assert(offset_ <= capacity);
        return buffer + offset_;
    }

    const T* operator+(size_t offset_) const noexcept
    {
        return const_cast<RawMemory&>(*this) + offset_;
    }

    const T& operator[](size_t index_) const noexcept
    {
        return const_cast<RawMemory&>(*this)[index_];
    }

    T& operator[](size_t index_) noexcept
    {
        assert(index_ < capacity);
        return buffer[index_];
    }

    void Swap(RawMemory& other_) noexcept
    {
        std::swap(buffer, other_.buffer);
        std::swap(capacity, other_.capacity);
    }

    const T* GetAddress() const noexcept
    {
        return buffer;
    }

    T* GetAddress() noexcept
    {
        return buffer;
    }

    size_t Capacity() const
    {
        return capacity;
    }

private:

    T* buffer = nullptr;
    size_t capacity = 0;

    static T* Allocate(size_t n_)
    {
        return n_ != 0 ? static_cast<T*>(operator new(n_ * sizeof(T))) : nullptr;
    }

    static void Deallocate(T* buf_) noexcept
    {
        operator delete(buf_);
    }
};

template <typename T>
class Vector
{
public:

    using Iterator = T*;
    using ConstIterator = const T*;
    using ReverseIterator = std::reverse_iterator<Iterator>;
    using ConstReverseIterator = std::reverse_iterator<ConstIterator>;

    //-------------------------------------------------------------Constructors & Destructor----------------------------------------------------

    Vector() = default;

    explicit Vector(size_t size_) : data(size_), size(size_)
    {
        std::uninitialized_value_construct_n(data.GetAddress(), size);
    }

    Vector(const Vector& other_) : data(other_.size), size(other_.size)
    {
        std::uninitialized_copy_n(other_.data.GetAddress(), size, data.GetAddress());
    }

    Vector(Vector&& other_) noexcept : data(std::move(other_.data)), size(std::exchange(other_.size, 0)) {}

    Vector(std::initializer_list<T> init_list_) : data(init_list_.size()), size(init_list_.size())
    {
        std::uninitialized_copy(init_list_.begin(), init_list_.end(), data.GetAddress());
    }

    ~Vector()
    {
        std::destroy_n(data.GetAddress(), size);
    }

    //----------------------------------------------------------------------Operators----------------------------------------------------------

    Vector& operator=(const Vector& other_)
    {
        if (this != &other_)
        {
            if (other_.size <= data.Capacity())
            {

                if (size <= other_.size)
                {
                    std::copy(other_.data.GetAddress(), other_.data.GetAddress() + size, data.GetAddress());
                    std::uninitialized_copy_n(other_.data.GetAddress() + size, other_.size - size, data.GetAddress() + size);
                }
                else
                {
                    std::copy(other_.data.GetAddress(), other_.data.GetAddress() + other_.size, data.GetAddress());
                    std::destroy_n(data.GetAddress() + other_.size, size - other_.size);
                }
                size = other_.size;
            }
            else
            {
                Vector other_copy(other_);
                Swap(other_copy);
            }
        }
        return *this;
    }

    Vector& operator=(Vector&& other_) noexcept
    {
        Swap(other_);
        return *this;
    }

    const T& operator[](size_t index_) const noexcept
    {
        return const_cast<Vector&>(*this)[index_];
    }
    T& operator[](size_t index_) noexcept
    {
        assert(index_ < size);
        return data[index_];
    }
    //------------------------------------------------------------------------Iterators---------------------------------------------------------
    Iterator begin() noexcept
    {
        return data.GetAddress();
    }
    Iterator end() noexcept
    {
        return size + data.GetAddress();
    }
    ConstIterator cbegin() const noexcept
    {
        return data.GetAddress();
    }
    ConstIterator cend() const noexcept
    {
        return size + data.GetAddress();
    }
    ConstIterator begin() const noexcept
    {
        return cbegin();
    }
    ConstIterator end() const noexcept
    {
        return cend();
    }
    ReverseIterator rbein() noexcept
    {
        return ReverseIterator(begin());
    }
    ReverseIterator rend() noexcept
    {
        return ReverseIterator(end());
    }
    ConstReverseIterator crbegin() const noexcept
    {
        return ConstReverseIterator(cend());
    }

    ConstReverseIterator crend() const noexcept
    {
        return ConstReverseIterator(cbegin());
    }

    ConstReverseIterator rbegin() const noexcept
    {
        return crbegin();
    }

    ConstReverseIterator rend() const noexcept
    {
        return crend();
    }

    //-----------------------------------------------------------------Non-Template Methods------------------------------------------------------

    size_t Size() const noexcept
    {
        return size;
    }

    size_t Capacity() const noexcept
    {
        return data.Capacity();
    }

    size_t MaxSize() const
    {
        return std::numeric_limits<size_t>::max / sizeof(T);
    }

    bool IsEmpty() const noexcept
    {
        return size == 0;
    }

    void Clear() noexcept
    {
        std::destroy_n(data.GetAddress(), size);
        size = 0;
    }

    T& Front() noexcept
    {
        assert(size > 0);
        return data[0];
    }

    const T& Front() const noexcept
    {
        assert(size > 0);
        return data[0];
    }

    T& Back() noexcept
    {
        assert(size > 0);
        return data[size - 1];
    }

    const T& Back() const noexcept
    {
        assert(size > 0);
        return data[size - 1];
    }

    Iterator Data() noexcept
    {
        return data.GetAddress();
    }

    ConstIterator Data() const noexcept
    {
        return data.GetAddress();
    }

    T& At(size_t index_)
    {
        assert(index_ < size);
        return data[index_];
    }

    const T& At(size_t index_) const
    {
        assert(index_ < size);
        return data[index_];
    }

    void Swap(Vector& other_) noexcept
    {
        data.Swap(other_.data), std::swap(size, other_.size);
    }

    void Reserve(size_t new_capacity_)
    {

        if (new_capacity_ <= data.Capacity())
        {
            return;
        }

        RawMemory<T> new_data(new_capacity_);

        if constexpr (std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>)
        {
            std::uninitialized_move_n(data.GetAddress(), size, new_data.GetAddress());
        }
        else
        {
            std::uninitialized_copy_n(data.GetAddress(), size, new_data.GetAddress());
        }

        std::destroy_n(data.GetAddress(), size);
        data.Swap(new_data);
    }

    void Resize(size_t new_size_)
    {
        if (new_size_ < size)
        {
            std::destroy_n(data.GetAddress() + new_size_, size - new_size_);
        }
        else
        {
            if (new_size_ > data.Capacity())
            {
                const size_t new_capacity = std::max(data.Capacity() * 2, new_size_);

                Reserve(new_capacity);
            }
            std::uninitialized_value_construct_n(data.GetAddress() + size, new_size_ - size);
        }
        size = new_size_;
    }

    void ShrinkToFit()
    {
        if (size == data.Capacity())
        {
            return;
        }
        if (size == 0)
        {
            data = RawMemory<T>();
        }
        else
        {
            RawMemory<T> new_data(size);
            std::uninitialized_move_n(data.GetAddress(), size, new_data.GetAddress());
            data.Swap(new_data);
        }
    }

    void Assign(std::initializer_list<T> ilist_)
    {
        Assign(ilist_.begin(), ilist_.end());
    }

    Iterator Insert(ConstIterator pos_, const T& item_)
    {
        return Emplace(pos_, item_);
    }
    Iterator Insert(ConstIterator pos_, T&& item_)
    {
        return Emplace(pos_, std::move(item_));
    }

    Iterator Erase(ConstIterator pos_)
    {
        assert(pos_ >= begin() && pos_ <= end());
        int position = pos_ - begin();

        std::move(begin() + position + 1, end(), begin() + position);
        std::destroy_at(end() - 1);
        size -= 1;

        return (begin() + position);
    }

    void PopBack()
    {
        assert(size);
        std::destroy_at(data.GetAddress() + size - 1);
        --size;
    }

    //------------------------------------------------------------------Template Metods----------------------------------------------------------

    template <typename... Args>
    T& EmplaceBack(Args&&... args_);

    template <typename... Args>
    Iterator Emplace(ConstIterator pos_, Args&&... args_);

    template <typename Type>
    void PushBack(Type&& value_);

    template <typename InputIt>
    void Assign(InputIt first_, InputIt last_);

private:

    RawMemory<T> data;
    size_t size = 0;
};


//----------------------------------------------------------------Implementing Template Methods--------------------------------------------
template <typename T>
template <typename Type>
void Vector<T>::PushBack(Type&& value_)
{
    if (data.Capacity() <= size)
    {
        RawMemory<T> new_data(size == 0 ? 1 : size * 2);

        new (new_data.GetAddress() + size) T(std::forward<Type>(value_));

        if constexpr (std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>)
        {
            std::uninitialized_move_n(data.GetAddress(), size, new_data.GetAddress());
        }
        else
        {
            std::uninitialized_copy_n(data.GetAddress(), size, new_data.GetAddress());
        }
        std::destroy_n(data.GetAddress(), size);
        data.Swap(new_data);
    }
    else
    {
        new (data.GetAddress() + size) T(std::forward<Type>(value_));
    }
    size++;
}

template <typename T>
template <typename... Args>
T& Vector<T>::EmplaceBack(Args&&... args_)
{
    if (data.Capacity() <= size)
    {
        RawMemory<T> new_data(size == 0 ? 1 : size * 2);

        new (new_data.GetAddress() + size) T(std::forward<Args>(args_)...);

        if constexpr (std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>)
        {
            std::uninitialized_move_n(data.GetAddress(), size, new_data.GetAddress());
        }
        else
        {
            std::uninitialized_copy_n(data.GetAddress(), size, new_data.GetAddress());
        }
        std::destroy_n(data.GetAddress(), size);
        data.Swap(new_data);
    }
    else
    {
        new (data.GetAddress() + size) T(std::forward<Args>(args_)...);
    }
    return data[size++];
}

template <typename T>
template <typename... Args>
typename Vector<T>::Iterator Vector<T>::Emplace(ConstIterator pos_, Args&&... args_)
{
    assert(pos_ >= begin() && pos_ <= end());
    int position = pos_ - begin();

    if (data.Capacity() <= size)
    {
        RawMemory<T> new_data(size == 0 ? 1 : size * 2);

        new (new_data.GetAddress() + position) T(std::forward<Args>(args_)...);

        if constexpr (std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>)
        {
            std::uninitialized_move_n(data.GetAddress(), position, new_data.GetAddress());
            std::uninitialized_move_n(data.GetAddress() + position, size - position, new_data.GetAddress() + position + 1);
        }
        else
        {
            std::uninitialized_copy_n(data.GetAddress(), position, new_data.GetAddress());
            std::uninitialized_copy_n(data.GetAddress() + position, size - position, new_data.GetAddress() + position + 1);
        }
        std::destroy_n(data.GetAddress(), size);
        data.Swap(new_data);
    }
    else
    {
        try
        {
            if (pos_ != end())
            {
                T new_s(std::forward<Args>(args_)...);
                new (end()) T(std::forward<T>(data[size - 1]));

                std::move_backward(begin() + position, end() - 1, end());
                *(begin() + position) = std::forward<T>(new_s);
            }
            else
            {
                new (end()) T(std::forward<Args>(args_)...);
            }
        }
        catch (...)
        {
            operator delete (end());
            throw;
        }
    }
    size++;
    return begin() + position;
}

template<typename T>
template <typename InputIt>
void Vector<T>::Assign(InputIt first_, InputIt last_)
{
    Clear();

    size_t new_size = std::distance(first_, last_);
    if (new_size > data.Capacity())
    {
        RawMemory<T> new_data(new_size);
        std::uninitialized_copy(first_, last_, new_data.GetAddress());
        data.Swap(new_data);
    }
    else
    {
        std::uninitialized_copy(first_, last_, data.GetAddress());
    }
    size = new_size;
}

//-----------------------------------------------------------Implementation of Comparison Operators------------------------------------------

template<typename T>
inline bool operator==(const Vector<T>& lhs_, const Vector<T>& rhs_)
{
    return std::equal(lhs_.begin(), lhs_.end(), rhs_.begin(), rhs_.end());
}
template<typename T>
inline bool operator!=(const Vector<T>& lhs_, const Vector<T>& rhs_)
{
    return !(lhs_ == rhs_);
}
template<typename T>
inline bool operator<(const Vector<T>& lhs_, const Vector<T>& rhs_)
{
    return std::lexicographical_compare(lhs_.begin(), lhs_.end(), rhs_.begin(), rhs_.end());
}
template<typename T>
inline bool operator<=(const Vector<T>& lhs_, const Vector<T>& rhs_)
{
    return !(rhs_ < lhs_);
}
template<typename T>
inline bool operator>(const Vector<T>& lhs_, const Vector<T>& rhs_)
{
    return (lhs_ <= rhs_);
}
template<typename T>
inline bool operator>=(const Vector<T>& lhs_, const Vector<T>& rhs_)
{
    return !(lhs_ < rhs_);
}
template<typename T>
constexpr auto operator<=>(const Vector<T>& lhs_, const Vector<T>& rhs_) /* return std::strong_ordering | std::weak_ordering | std::partial_ordering */
{
    return std::lexicographical_compare_three_way(lhs_.begin(), lhs_.end(), rhs_.begin(), rhs_.end());
}