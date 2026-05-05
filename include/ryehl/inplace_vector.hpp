#ifndef RYEHL_INPLACE_VECTOR_HPP
#define RYEHL_INPLACE_VECTOR_HPP

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <ranges>
#include <type_traits>

namespace ryehl
{
    struct from_range_t
    {
        explicit from_range_t() = default;
    };

    inline constexpr from_range_t from_range{};

    namespace details
    {
        template <class R, class T>
        concept container_compatible_range =
            std::ranges::input_range<R> &&
            std::convertible_to<std::ranges::range_reference_t<R>, T>;

        template <typename T>
        concept less_than_comparable = requires
        {
            { std::declval<T>() < std::declval<T>() } -> std::convertible_to<bool>;
        };

        template <std::ranges::random_access_range Range, std::integral Index>
        static constexpr decltype(auto) index(Range&& range, Index index) noexcept
            requires(std::ranges::sized_range<Range>)
        {
            return std::begin(std::forward<Range>(range))[std::forward<Index>(index)];
        }

        namespace inplace_vector
        {
            namespace storage
            {
                template <typename T>
                struct empty
                {
                    constexpr empty() noexcept = default;
                    constexpr empty(const empty&) noexcept = default;
                    constexpr empty(empty&&) noexcept = default;
                    constexpr empty& operator=(const empty&) noexcept = default;
                    constexpr empty& operator=(empty&&) noexcept = default;

                    constexpr ~empty() noexcept = default;

                protected:
                    static constexpr T* storage_data() noexcept { return nullptr; }
                    static constexpr std::size_t storage_size() noexcept { return 0; }

                    static constexpr void
                    set_size_uninitialized_unsafe([[maybe_unused]] const std::size_t new_size) noexcept {}
                };

                template <typename T, std::size_t N>
                struct trivial
                {
                    static_assert(std::is_trivial_v<T>, "trivial<T, N> requires T to be a trivial type");
                    static_assert(N != 0, "trivial<T, N> requires N to be greater than 0. Use empty<T> instead.");

                    constexpr trivial() noexcept = default;
                    constexpr trivial(const trivial&) noexcept = default;
                    constexpr trivial(trivial&&) noexcept = default;
                    constexpr trivial& operator=(const trivial&) noexcept = default;
                    constexpr trivial& operator=(trivial&&) noexcept = default;

                    constexpr ~trivial() noexcept = default;

                protected:
                    constexpr T* storage_data() noexcept { return storage_data_.data(); }
                    constexpr const T* storage_data() const noexcept { return storage_data_.data(); }
                    [[nodiscard]] constexpr std::size_t storage_size() const noexcept { return storage_size_; }

                    constexpr void set_size_uninitialized_unsafe(const std::size_t new_size) noexcept
                    {
                        storage_size_ = new_size;
                    }

                private:
                    alignas(alignof(T)) std::array<T, N> storage_data_;
                    std::size_t storage_size_ = 0;
                };

                template <typename T, std::size_t N>
                struct non_trivial // NOLINT(cppcoreguidelines-pro-type-member-init)
                {
                    static_assert(!std::is_trivial_v<T>, "non_trivial<T, N> requires T to be a non-trivial type");
                    static_assert(N != 0, "non_trivial<T, N> requires N to be greater than 0. Use empty<T> instead.");

                    constexpr non_trivial() noexcept = default;
                    constexpr non_trivial(const non_trivial&) noexcept = default;
                    constexpr non_trivial(non_trivial&&) noexcept = default;
                    constexpr non_trivial& operator=(const non_trivial&) noexcept = default;
                    constexpr non_trivial& operator=(non_trivial&&) noexcept = default;

                    constexpr ~non_trivial() requires(std::is_trivially_destructible_v<T>) = default;
                    constexpr ~non_trivial() { std::destroy(storage_data(), storage_data() + storage_size()); }

                protected:
                    constexpr T* storage_data() noexcept { return reinterpret_cast<T*>(storage_bytes_); }

                    constexpr const T* storage_data() const noexcept
                    {
                        return reinterpret_cast<const T*>(storage_bytes_);
                    }

                    [[nodiscard]] constexpr std::size_t storage_size() const noexcept { return storage_size_; }

                    constexpr void set_size_uninitialized_unsafe(const std::size_t new_size) noexcept
                    {
                        storage_size_ = new_size;
                    }

                private:
                    alignas(T) std::byte storage_bytes_[sizeof(T) * N];
                    std::size_t storage_size_ = 0;
                };

                template <typename T, std::size_t N>
                using storage_selector = std::conditional_t<N == 0,
                                                            empty<T>,
                                                            std::conditional_t<
                                                                std::is_trivial_v<T>,
                                                                trivial<T, N>,
                                                                non_trivial<T, N>
                                                            >>;
            }

            template <typename T, std::size_t N>
            struct inplace_vector_base : private storage::storage_selector<T, N>
            {
                static_assert(std::is_nothrow_destructible_v<T>,
                              "inplace_vector_base<T, N> requires T to be nothrow destructible");

                using value_type = T;
                using size_type = std::size_t;
                using difference_type = std::ptrdiff_t;
                using reference = value_type&;
                using const_reference = const value_type&;
                using pointer = value_type*;
                using const_pointer = const value_type*;
                using iterator = pointer;
                using const_iterator = const_pointer;
                using reverse_iterator = std::reverse_iterator<iterator>;
                using const_reverse_iterator = std::reverse_iterator<const_iterator>;

                constexpr inplace_vector_base() noexcept = default;
                constexpr inplace_vector_base(const inplace_vector_base&)
                    requires(N == 0 || std::is_trivially_copy_constructible_v<T>) = default;

                constexpr inplace_vector_base(const inplace_vector_base& other)
                    requires(N != 0 && !std::is_trivially_copy_constructible_v<T>)
                {
                    for (const auto& item : other)
                        unchecked_emplace_back(item);
                }

                constexpr inplace_vector_base(inplace_vector_base&&)
                    requires(N == 0 || std::is_trivially_move_constructible_v<T>) = default;

                constexpr inplace_vector_base(inplace_vector_base&& other) noexcept
                    requires(N != 0 && !std::is_trivially_move_constructible_v<T>)
                {
                    for (auto& item : other)
                        unchecked_emplace_back(std::move(item));
                    other.set_size_uninitialized_unsafe(0);
                }

                constexpr inplace_vector_base& operator=(const inplace_vector_base&)
                    requires(
                        std::is_trivially_destructible_v<T> &&
                        std::is_trivially_copy_constructible_v<T> &&
                        std::is_trivially_copy_assignable_v<T>) = default;

                constexpr inplace_vector_base& operator=(const inplace_vector_base& other)
                    requires(
                        !std::is_trivially_destructible_v<T> ||
                        !std::is_trivially_copy_constructible_v<T> ||
                        !std::is_trivially_copy_assignable_v<T>)
                {
                    if (this != &other)
                    {
                        clear();
                        for (const auto& item : other)
                            unchecked_emplace_back(item);
                    }
                    return *this;
                }

                constexpr inplace_vector_base& operator=(inplace_vector_base&&)
                    requires(
                        std::is_trivially_destructible_v<T> &&
                        std::is_trivially_move_constructible_v<T> &&
                        std::is_trivially_move_assignable_v<T>) = default;

                constexpr inplace_vector_base& operator=(inplace_vector_base&& other) noexcept
                    requires(
                        !std::is_trivially_destructible_v<T> ||
                        !std::is_trivially_move_constructible_v<T> ||
                        !std::is_trivially_move_assignable_v<T>)
                {
                    if (this != &other)
                    {
                        clear();
                        for (auto& item : other)
                            unchecked_emplace_back(std::move(item));
                        other.set_size_uninitialized_unsafe(0);
                    }
                    return *this;
                }

                constexpr reference operator[](size_type pos) noexcept { return details::index(*this, pos); }

                constexpr const_reference operator[](size_type pos) const noexcept
                {
                    return details::index(*this, pos);
                }

                constexpr reference front() noexcept { return details::index(*this, 0); }

                constexpr const_reference front() const noexcept
                {
                    return details::index(*this, 0);
                }

                constexpr reference back() noexcept
                {
                    return details::index(*this, size() - 1);
                }

                constexpr const_reference back() const noexcept
                {
                    return details::index(*this, size() - 1);
                }

                constexpr T* data() noexcept { return storage_data(); }
                constexpr const T* data() const noexcept { return storage_data(); }

                constexpr iterator begin() noexcept { return storage_data(); }
                constexpr const_iterator begin() const noexcept { return storage_data(); }
                constexpr const_iterator cbegin() const noexcept { return storage_data(); }

                constexpr iterator end() noexcept { return begin() + size(); }
                constexpr const_iterator end() const noexcept { return begin() + size(); }
                constexpr const_iterator cend() const noexcept { return cbegin() + size(); }

                constexpr reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
                constexpr const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(end()); }
                constexpr const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(cend()); }

                constexpr reverse_iterator rend() noexcept { return reverse_iterator(begin()); }
                constexpr const_reverse_iterator rend() const noexcept { return const_reverse_iterator(begin()); }
                constexpr const_reverse_iterator crend() const noexcept { return const_reverse_iterator(cbegin()); }

                [[nodiscard]] constexpr bool empty() const noexcept { return size() == 0; }
                [[nodiscard]] constexpr size_type size() const noexcept { return storage_size(); }
                [[nodiscard]] static constexpr size_type max_size() noexcept { return N; }
                [[nodiscard]] static constexpr size_type capacity() noexcept { return N; }
                static constexpr void shrink_to_fit() noexcept {}

                template <typename... Args>
                constexpr pointer try_emplace_back(Args&&... args)
                    requires(std::constructible_from<T, Args...>)
                {
                    if (size() == capacity()) [[unlikely]]
                        return nullptr;
                    return std::addressof(unchecked_emplace_back(std::forward<Args>(args)...));
                }

                template <typename... Args>
                constexpr reference unchecked_emplace_back(Args&&... args)
                    requires(std::constructible_from<T, Args...>)
                {
                    assert(size() < capacity());
                    std::construct_at(end(), std::forward<Args>(args)...);
                    set_size_uninitialized_unsafe(size() + 1);
                    return back();
                }

                constexpr pointer try_push_back(const T& value)
                    requires(std::constructible_from<T, const T&>)
                {
                    return try_emplace_back(value);
                }

                constexpr pointer try_push_back(T&& value)
                    requires(std::constructible_from<T, T&&>)
                {
                    return try_emplace_back(std::move(value));
                }

                constexpr void pop_back()
                {
                    assert(size() > 0);
                    std::destroy_at(std::addressof(back()));
                    set_size_uninitialized_unsafe(size() - 1);
                }

                template <container_compatible_range<T> R>
                constexpr std::ranges::borrowed_iterator_t<R> try_append_range(R&& rg)
                    requires(std::constructible_from<T, std::ranges::range_reference_t<R>>)
                {
                    auto it = std::ranges::begin(rg);
                    const auto end_it = std::ranges::end(rg);
                    for (; size() != capacity() && it != end_it; ++it)
                        unchecked_emplace_back(*it);
                    return it;
                }

                constexpr void clear() noexcept
                {
                    std::destroy(begin(), end());
                    set_size_uninitialized_unsafe(0);
                }

                constexpr iterator erase(const_iterator pos)
                    requires(std::movable<T>)
                {
                    assert_iterator_in_range(pos);
                    iterator non_const_pos = begin() + (pos - cbegin());
                    std::move(non_const_pos + 1, end(), non_const_pos);
                    std::destroy_at(std::addressof(back()));
                    set_size_uninitialized_unsafe(size() - 1);
                    return non_const_pos;
                }

                constexpr iterator erase(const_iterator first, const_iterator last)
                    requires(std::movable<T>)
                {
                    assert_iterator_pair_in_range(first, last);
                    iterator non_const_first = begin() + (first - cbegin());
                    if (first != last)
                    {
                        std::destroy(std::move(non_const_first + (last - first), end(), non_const_first), end());
                        set_size_uninitialized_unsafe(size() - static_cast<size_type>(last - first));
                    }
                    return non_const_first;
                }

                constexpr void swap(inplace_vector_base& other) noexcept(
                    N == 0 || (std::is_nothrow_swappable_v<T> &&
                        std::is_nothrow_move_constructible_v<T>))
                    requires(std::movable<T>)
                {
                    auto tmp = std::move(other);
                    other = std::move(*this);
                    *this = std::move(tmp);
                }

                constexpr friend bool operator==(const inplace_vector_base& lhs, const inplace_vector_base& rhs)
                {
                    return lhs.size() == rhs.size() && std::ranges::equal(lhs, rhs);
                }

                constexpr friend auto operator<=>(const inplace_vector_base& lhs, const inplace_vector_base& rhs)
                    requires(std::three_way_comparable<T> || details::less_than_comparable<T>)
                {
                    if constexpr (std::three_way_comparable<T>)
                        return std::lexicographical_compare_three_way(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
                    for (auto [l, r] = std::pair(lhs.begin(), rhs.begin()); l != lhs.end() && r != rhs.end(); ++l, ++r)
                    {
                        if (*l < *r)
                            return std::strong_ordering::less;
                        if (*r < *l)
                            return std::strong_ordering::greater;
                    }
                    return lhs.size() <=> rhs.size();
                }

            protected:
                using base_type = storage::storage_selector<T, N>;
                using base_type::storage_data;
                using base_type::storage_size;
                using base_type::set_size_uninitialized_unsafe;

                constexpr void assert_iterator_in_range([[maybe_unused]] const_iterator it) const noexcept
                {
                    assert(it >= cbegin() && it <= cend());
                }

                constexpr void assert_iterator_pair_in_range([[maybe_unused]] const_iterator first,
                                                             [[maybe_unused]] const_iterator last) const noexcept
                {
                    assert_iterator_in_range(first);
                    assert_iterator_in_range(last);
                    assert(first <= last);
                }
            };
        }
    }

    template <typename T, std::size_t N>
    struct inplace_vector : details::inplace_vector::inplace_vector_base<T, N>
    {
    private:
        using base_type = details::inplace_vector::inplace_vector_base<T, N>;

    public:
        using value_type = T;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        using reference = value_type&;
        using const_reference = const value_type&;
        using pointer = value_type*;
        using const_pointer = const value_type*;
        using iterator = pointer;
        using const_iterator = const_pointer;
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

        constexpr inplace_vector() noexcept = default;

        constexpr explicit inplace_vector(size_type count)
            requires(std::constructible_from<T, T&&> && std::default_initializable<T>)
        {
            for (size_type i = 0; i < count; ++i)
                emplace_back();
        }

        constexpr inplace_vector(size_type count, const T& value)
            requires(std::constructible_from<T, const T&> && std::copyable<T>)
        {
            for (size_type i = 0; i < count; ++i)
                emplace_back(value);
        }

        template <typename InputIt>
        constexpr inplace_vector(InputIt first, InputIt last)
            requires(std::constructible_from<T, std::iter_reference_t<InputIt>> && std::movable<T>)
        {
            for (; first != last; ++first)
                emplace_back(std::move(*first));
        }

        template <details::container_compatible_range<T> R>
        constexpr inplace_vector(from_range_t, R&& rg)
            requires(std::constructible_from<T, std::ranges::range_reference_t<R>> && std::movable<T>)
        {
            assign_range(rg);
        }

        constexpr inplace_vector(std::initializer_list<T> init)
            requires(
                std::constructible_from<T, std::ranges::range_reference_t<std::initializer_list<T>>> &&
                std::movable<T>)
        {
            assign_range(init);
        }

        constexpr inplace_vector& operator=(std::initializer_list<T> init)
            requires(std::constructible_from<T, std::ranges::range_reference_t<std::initializer_list<T>>> &&
                std::movable<T>)
        {
            assign_range(init);
            return *this;
        }

        constexpr void assign(const size_type count, const T& value)
            requires(std::constructible_from<T, const T&> && std::copyable<T>)
        {
            if (count > this->capacity()) [[unlikely]]
                throw std::bad_alloc{};
            this->clear();
            insert(this->begin(), count, value);
        }

        template <typename InputIt>
        constexpr void assign(InputIt first, InputIt last)
            requires(std::constructible_from<T, std::iter_reference_t<InputIt>> && std::movable<T>)
        {
            if constexpr (std::random_access_iterator<InputIt>)
                if (static_cast<size_type>(last - first) > this->capacity()) [[unlikely]]
                    throw std::bad_alloc{};
            this->clear();
            insert(this->begin(), first, last);
        }

        constexpr void assign(std::initializer_list<T> il)
            requires(
                std::constructible_from<T, std::ranges::range_reference_t<std::initializer_list<T>>> &&
                std::movable<T>)
        {
            assign(il.begin(), il.end());
        }

        template <details::container_compatible_range<T> R>
        constexpr void assign_range(R&& rg)
            requires(std::constructible_from<T, std::ranges::range_reference_t<R>> && std::movable<T>)
        {
            if (static_cast<size_type>(std::ranges::distance(rg)) > this->capacity()) [[unlikely]]
                throw std::bad_alloc{};
            this->clear();
            insert_range(this->begin(), rg);
        }

        static constexpr void reserve(size_type new_cap)
        {
            if (new_cap > base_type::capacity()) [[unlikely]]
                throw std::bad_alloc{};
        }

        constexpr reference at(size_type pos)
        {
            if (pos >= this->size()) [[unlikely]]
                throw std::out_of_range{"inplace_vector::at: index out of range"};
            return (*this)[pos];
        }

        constexpr const_reference at(size_type pos) const
        {
            if (pos >= this->size()) [[unlikely]]
                throw std::out_of_range{"inplace_vector::at: index out of range"};
            return (*this)[pos];
        }

        constexpr void resize(size_type count)
            requires(std::constructible_from<T, T&&> && std::default_initializable<T>)
        {
            if (count > this->capacity()) [[unlikely]]
                throw std::bad_alloc{};
            if (count == this->size())
                return;
            if (count > this->size())
            {
                const size_type to_insert = count - this->size();
                for (size_type i = 0; i < to_insert; ++i)
                    emplace_back();
            }
            else
            {
                std::destroy(this->begin() + count, this->end());
                this->set_size_uninitialized_unsafe(count);
            }
        }

        constexpr void resize(size_type count, const_reference value)
            requires(std::constructible_from<T, const T&> && std::copyable<T>)
        {
            if (count > this->capacity()) [[unlikely]]
                throw std::bad_alloc{};
            if (count == this->size())
                return;
            if (count > this->size())
            {
                const size_type to_insert = count - this->size();
                for (size_type i = 0; i < to_insert; ++i)
                    emplace_back(value);
            }
            else
            {
                std::destroy(this->begin() + count, this->end());
                this->set_size_uninitialized_unsafe(count);
            }
        }

        constexpr iterator insert(const_iterator pos, const T& value)
            requires(std::constructible_from<T, const T&> && std::copyable<T>)
        {
            return insert(pos, 1, value);
        }

        constexpr iterator insert(const_iterator pos, T&& value)
            requires(std::constructible_from<T, T&&> && std::movable<T>)
        {
            return emplace(pos, std::move(value));
        }

        constexpr iterator insert(const_iterator pos, const size_type count, const T& value)
            requires(std::constructible_from<T, const T&> && std::copyable<T>)
        {
            this->assert_iterator_in_range(pos);
            auto old_end = this->end();
            for (size_type i = 0; i < count; ++i)
                emplace_back(value);
            iterator non_const_pos = this->begin() + (pos - this->cbegin());
            std::rotate(non_const_pos, old_end, this->end());
            return non_const_pos;
        }

        template <typename InputIt>
        constexpr iterator insert(const_iterator pos, InputIt first, InputIt last)
            requires(std::constructible_from<T, std::iter_reference_t<InputIt>> && std::movable<T>)
        {
            this->assert_iterator_in_range(pos);
            if constexpr (std::random_access_iterator<InputIt>)
                if (this->size() + static_cast<size_type>(last - first) > this->capacity()) [[unlikely]]
                    throw std::bad_alloc{};
            auto old_end = this->end();
            for (; first != last; ++first)
                emplace_back(std::move(*first));
            iterator non_const_pos = this->begin() + (pos - this->cbegin());
            std::rotate(non_const_pos, old_end, this->end());
            return non_const_pos;
        }

        constexpr iterator insert(const_iterator pos, std::initializer_list<T> il)
            requires(
                std::constructible_from<T, std::ranges::range_reference_t<std::initializer_list<T>>> &&
                std::movable<T>)
        {
            return insert_range(pos, il.begin(), il.end());
        }

        template <details::container_compatible_range<T> R>
        constexpr iterator insert_range(const_iterator pos, R&& rg)
            requires(std::constructible_from<T, std::ranges::range_reference_t<R>> && std::movable<T>)
        {
            return insert(pos, std::ranges::begin(rg), std::ranges::end(rg));
        }

        template <typename... Args>
        constexpr iterator emplace(const_iterator position, Args&&... args)
            requires(std::constructible_from<T, Args...> && std::movable<T>)
        {
            this->assert_iterator_in_range(position);
            auto old_end = this->end();
            emplace_back(std::forward<Args>(args)...);
            iterator non_const_position = this->begin() + (position - this->cbegin());
            std::rotate(non_const_position, old_end, this->end());
            return non_const_position;
        }

        template <typename... Args>
        constexpr reference emplace_back(Args&&... args)
            requires(std::constructible_from<T, Args...>)
        {
            if (this->try_emplace_back(std::forward<Args>(args)...) == nullptr) [[unlikely]]
                throw std::bad_alloc{};
            return this->back();
        }

        constexpr reference push_back(const T& value)
            requires(std::constructible_from<T, const T&>)
        {
            return emplace_back(value);
        }

        constexpr reference push_back(T&& value)
            requires(std::constructible_from<T, T&&>)
        {
            return emplace_back(std::move(value));
        }

        template <details::container_compatible_range<T> R>
        constexpr void append_range(R&& rg)
            requires(std::constructible_from<T, std::ranges::range_reference_t<R>>)
        {
            if (this->size() + std::ranges::size(rg) > this->capacity()) [[unlikely]]
                throw std::bad_alloc{};
            for (auto&& item : rg)
            {
                if (this->size() == this->capacity()) [[unlikely]]
                    throw std::bad_alloc{};
                emplace_back(std::move(item));
            }
        }
    };
}

#endif //RYEHL_INPLACE_VECTOR_HPP
