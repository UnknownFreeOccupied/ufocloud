/*!
 * UFOMap: An Efficient Probabilistic 3D Mapping Framework That Embraces the Unknown
 *
 * @author Daniel Duberg (dduberg@kth.se)
 * @see https://github.com/UnknownFreeOccupied/ufomap
 * @version 1.0
 * @date 2022-05-13
 *
 * @copyright Copyright (c) 2022, Daniel Duberg, KTH Royal Institute of Technology
 *
 * BSD 3-Clause License
 *
 * Copyright (c) 2022, Daniel Duberg, KTH Royal Institute of Technology
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *     list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *     contributors may be used to endorse or promote products derived from
 *     this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef UFO_PCL_CLOUD_HPP
#define UFO_PCL_CLOUD_HPP

// UFO
#include <ufo/utility/proxy_arrow_result.hpp>
#include <ufo/utility/type_traits.hpp>

// STL
#include <algorithm>
#include <iostream>
#include <tuple>
#include <utility>
#include <vector>

namespace ufo
{
template <class T, class... Rest>
class Cloud
{
	static_assert(is_unique_v<T, Rest...>);

 private:
	template <class Derived, class E>
	struct CloudElementConvert {
		operator E() const { return static_cast<Derived*>(this)->template get<E>(); }

		operator E const&() const { return static_cast<Derived*>(this)->template get<E>(); }

		operator E const*() const { return &static_cast<Derived*>(this)->template get<E>(); }

		operator E&() { return static_cast<Derived*>(this)->template get<E>(); }

		operator E*() { return &static_cast<Derived*>(this)->template get<E>(); }

		Derived& operator=(E const& other)
		{
			static_cast<Derived*>(this)->template set<E>(other);
			return *static_cast<Derived*>(this);
		}

		// Derived& operator+=(E const& other)
		// {
		// 	static_cast<Derived*>(this)->template get<E>() += other;
		// 	return *static_cast<Derived*>(this);
		// }

		// Derived& operator-=(E const& other)
		// {
		// 	static_cast<Derived*>(this)->template get<E>() -= other;
		// 	return *static_cast<Derived*>(this);
		// }

		// Derived& operator*=(E const& other)
		// {
		// 	static_cast<Derived*>(this)->template get<E>() *= other;
		// 	return *static_cast<Derived*>(this);
		// }

		// Derived& operator/=(E const& other)
		// {
		// 	static_cast<Derived*>(this)->template get<E>() /= other;
		// 	return *static_cast<Derived*>(this);
		// }
	};

 public:
	using data_type       = std::tuple<std::vector<T>, std::vector<Rest>...>;
	using size_type       = typename std::vector<T>::size_type;
	using difference_type = typename std::vector<T>::difference_type;

	class value_type
	    : public T
	    , public Rest...
	    , public CloudElementConvert<value_type, T>
	    , public CloudElementConvert<value_type, Rest>...
	{
	 public:
		value_type() = default;

		value_type(T const& first, Rest const&... rest) : T(first), Rest(rest)... {}

		using CloudElementConvert<value_type, T>::operator=;
		using CloudElementConvert<value_type, Rest>::operator=...;

		// using CloudElementConvert<value_type, T>::operator+=;
		// using CloudElementConvert<value_type, Rest>::operator+=...;

		template <class E>
		[[nodiscard]] E& get()
		{
			return static_cast<std::decay_t<E>&>(*this);
		}

		template <class E>
		[[nodiscard]] E const& get() const
		{
			return static_cast<std::decay_t<E> const&>(*this);
		}

		template <class E, class Arg>
		E& set(Arg&& arg)
		{
			E& e = get<E>();
			e    = std::forward<Arg>(arg);
			return e;
		}

		template <class E, class... Args>
		E& set(Args&&... args)
		{
			E& e = get<E>();
			e    = E(std::forward<Args>(args)...);
			return e;
		}

		friend std::ostream& operator<<(std::ostream& out, value_type const& value)
		{
			out << value.template get<T>();
			if constexpr (0 < sizeof...(Rest)) {
				return ((out << ' ' << value.template get<Rest>()), ...);
			} else {
				return out;
			}
		}
	};

	class value_type_ref
	    : public CloudElementConvert<value_type_ref, T>
	    , public CloudElementConvert<value_type_ref, Rest>...
	{
	 private:
		using data_type = std::tuple<T&, Rest&...>;

	 public:
		using CloudElementConvert<value_type_ref, T>::operator=;
		using CloudElementConvert<value_type_ref, Rest>::operator=...;

		// using CloudElementConvert<value_type_ref, T>::operator+=;
		// using CloudElementConvert<value_type_ref, Rest>::operator+=...;

		value_type_ref& operator=(value_type const& value) { return set(value); }

		value_type_ref& operator=(value_type_ref const& value) = default;

		operator value_type() const
		{
			return std::apply([](auto&&... args) { return value_type(args...); }, data_);
		}

		template <class E>
		[[nodiscard]] E& get()
		{
			return std::get<E&>(data_);
		}

		template <class E>
		[[nodiscard]] E const& get() const
		{
			return std::get<E&>(data_);
		}

		template <class E, class Arg>
		E& set(Arg&& arg)
		{
			E& e = get<E>();
			e    = std::forward<Arg>(arg);
			return e;
		}

		template <class E, class... Args>
		E& set(Args&&... args)
		{
			E& e = get<E>();
			e    = E(std::forward<Args>(args)...);
			return e;
		}

		value_type_ref& set(value_type const& value)
		{
			std::apply(
			    [this, &value](auto&&... args) {
				    ((get<decltype(args)>() = value.template get<decltype(args)>()), ...);
			    },
			    data_);
			return *this;
		}

		value_type_ref& set(value_type_ref const& value)
		{
			data_ = value.data;
			return *this;
		}

		friend std::ostream& operator<<(std::ostream& out, value_type_ref const& value)
		{
			out << value.template get<T>();
			if constexpr (0 < sizeof...(Rest)) {
				return ((out << ' ' << value.template get<Rest>()), ...);
			} else {
				return out;
			}
		}

	 private:
		value_type_ref(T& first, Rest&... rest) : data_(first, rest...) {}

	 private:
		data_type data_;

		friend class Cloud;
	};

	class iterator
	{
	 public:
		// Tags
		using iterator_category = std::random_access_iterator_tag;
		using difference_type   = std::ptrdiff_t;
		using value_type        = Cloud::value_type_ref;
		using reference         = value_type;
		using pointer           = proxy_arrow_result<reference>;

	 public:
		constexpr iterator& operator++()
		{
			++index_;
			return *this;
		}

		constexpr iterator& operator--()
		{
			--index_;
			return *this;
		}

		constexpr iterator operator++(int)
		{
			iterator tmp(*this);
			++index_;
			return tmp;
		}

		constexpr iterator operator--(int)
		{
			iterator tmp(*this);
			--index_;
			return tmp;
		}

		constexpr iterator operator+(difference_type n)
		{
			iterator tmp(*this);
			tmp.index_ += n;
			return tmp;
		}

		constexpr iterator operator-(difference_type n)
		{
			iterator tmp(*this);
			tmp.index_ -= n;
			return tmp;
		}

		constexpr iterator& operator+=(difference_type n)
		{
			index_ += n;
			return *this;
		}

		constexpr iterator& operator-=(difference_type n)
		{
			index_ -= n;
			return *this;
		}

		[[nodiscard]] constexpr reference operator[](difference_type pos) const
		{
			return data_->operator[](index_ + pos);
		}

		[[nodiscard]] constexpr reference operator*() const { return operator[](0); }

		[[nodiscard]] constexpr pointer operator->() const { return **this; }

		constexpr difference_type operator-(iterator const& rhs) const
		{
			return index_ - rhs.index_;
		}

		[[nodiscard]] constexpr bool operator==(iterator other) const
		{
			return index_ == other.index_ && data_ == other.data_;
		}

		[[nodiscard]] constexpr bool operator!=(iterator other) const
		{
			return !(*this == other);
		}

		[[nodiscard]] constexpr bool operator<(iterator other) const
		{
			return index_ < other.index_;
		}

		[[nodiscard]] constexpr bool operator<=(iterator other) const
		{
			return index_ <= other.index_;
		}

		[[nodiscard]] constexpr bool operator>(iterator other) const
		{
			return index_ > other.index_;
		}

		[[nodiscard]] constexpr bool operator>=(iterator other) const
		{
			return index_ >= other.index_;
		}

	 private:
		iterator(Cloud* data, std::size_t index = 0) : data_(data), index_(index) {}

	 private:
		Cloud*      data_  = nullptr;
		std::size_t index_ = 0;

		friend class Cloud;
		friend class const_iterator;
	};

	class const_iterator
	{
	 public:
		// Tags
		using iterator_category = std::random_access_iterator_tag;
		using difference_type   = std::ptrdiff_t;
		// FIXME: Should be a const Cloud::value_type_ref
		using value_type = Cloud::value_type;
		using reference  = value_type;
		using pointer    = proxy_arrow_result<reference>;

	 public:
		const_iterator() = default;

		const_iterator(iterator const& other) : data_(other.data_), index_(other.index_) {}

		const_iterator& operator=(iterator const& other)
		{
			data_  = other.data_;
			index_ = other.index_;
			return *this;
		}

		constexpr const_iterator& operator++()
		{
			++index_;
			return *this;
		}

		constexpr const_iterator& operator--()
		{
			--index_;
			return *this;
		}

		constexpr const_iterator operator++(int)
		{
			const_iterator tmp(*this);
			++index_;
			return tmp;
		}

		constexpr const_iterator operator--(int)
		{
			const_iterator tmp(*this);
			--index_;
			return tmp;
		}

		constexpr const_iterator operator+(difference_type n)
		{
			const_iterator tmp(*this);
			tmp.index_ += n;
			return tmp;
		}

		constexpr const_iterator operator-(difference_type n)
		{
			const_iterator tmp(*this);
			tmp.index_ -= n;
			return tmp;
		}

		constexpr const_iterator& operator+=(difference_type n)
		{
			index_ += n;
			return *this;
		}

		constexpr const_iterator& operator-=(difference_type n)
		{
			index_ -= n;
			return *this;
		}

		[[nodiscard]] constexpr reference operator[](difference_type pos) const
		{
			return data_->operator[](index_ + pos);
		}

		[[nodiscard]] constexpr reference operator*() const { return operator[](0); }

		[[nodiscard]] constexpr pointer operator->() const { return **this; }

		constexpr difference_type operator-(const_iterator const& rhs) const
		{
			return index_ - rhs.index_;
		}

		[[nodiscard]] constexpr bool operator==(const_iterator other) const
		{
			return index_ == other.index_ && data_ == other.data_;
		}

		[[nodiscard]] constexpr bool operator!=(const_iterator other) const
		{
			return !(*this == other);
		}

		[[nodiscard]] constexpr bool operator<(const_iterator other) const
		{
			return index_ < other.index_;
		}

		[[nodiscard]] constexpr bool operator<=(const_iterator other) const
		{
			return index_ <= other.index_;
		}

		[[nodiscard]] constexpr bool operator>(const_iterator other) const
		{
			return index_ > other.index_;
		}

		[[nodiscard]] constexpr bool operator>=(const_iterator other) const
		{
			return index_ >= other.index_;
		}

	 private:
		const_iterator(Cloud const* data, std::size_t index = 0) : data_(data), index_(index)
		{
		}

	 private:
		Cloud const* data_  = nullptr;
		std::size_t  index_ = 0;

		friend class Cloud;
	};

	using reverse_iterator       = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;

	Cloud() = default;

	Cloud(size_type count, value_type const& value)
	{
		std::apply(
		    [count, &value](auto&&... data) {
			    ((data = std::decay_t<decltype(data)>(count, value)), ...);
		    },
		    data_);
	}

	explicit Cloud(size_type count)
	{
		std::apply(
		    [count](auto&&... data) { ((data = std::decay_t<decltype(data)>(count)), ...); },
		    data_);
	}

	template <class InputIt>
	Cloud(InputIt first, InputIt last)
	{
		reserve(std::distance(first, last));
		for (; first != last; ++first) {
			push_back(*first);
		}
	}

	Cloud(Cloud const& other) = default;

	Cloud(Cloud&& other) noexcept = default;

	Cloud(std::initializer_list<value_type> init) : Cloud(std::begin(init), std::end(init))
	{
	}

	~Cloud() = default;

	Cloud& operator=(Cloud const& other) = default;

	Cloud& operator=(Cloud&& other) = default;

	Cloud& operator=(std::initializer_list<value_type> ilist)
	{
		clear();
		reserve(ilist.size());
		for (auto const& v : ilist) {
			push_back(v);
		}
		return *this;
	}

	void assign(size_type count, value_type const& value)
	{
		clear();
		reserve(count);
		for (std::size_t i{}; count > i; ++i) {
			push_back(value);
		}
	}

	template <class InputIt>
	void assign(InputIt first, InputIt last)
	{
		clear();
		reserve(std::distance(first, last));
		for (; first != last; ++first) {
			push_back(*first);
		}
	}

	void assign(std::initializer_list<value_type> ilist)
	{
		assign(std::begin(ilist), std::end(ilist));
	}

	[[nodiscard]] value_type_ref at(std::size_t index)
	{
		if (size() <= index) {
			throw std::out_of_range("Cloud: this->size() <= index");
		}
		return operator[](index);
	}

	[[nodiscard]] value_type at(std::size_t index) const
	{
		if (size() <= index) {
			throw std::out_of_range("Cloud: this->size() <= index");
		}
		return operator[](index);
	}

	[[nodiscard]] value_type_ref operator[](std::size_t index)
	{
		std::tuple<T&, Rest&...> elements = std::apply(
		    [&elements, index](auto&&... args) { return std::tie(args[index]...); }, data_);
		return std::apply([](auto&&... args) { return value_type_ref(args...); }, elements);
	}

	[[nodiscard]] value_type operator[](std::size_t index) const
	{
		std::tuple<T const&, Rest const&...> elements = std::apply(
		    [&elements, index](auto&&... args) { return std::tie(args[index]...); }, data_);
		return std::apply([](auto&&... args) { return value_type(args...); }, elements);
	}

	[[nodiscard]] value_type_ref front() { return operator[](0); }

	[[nodiscard]] value_type front() const { return operator[](0); }

	[[nodiscard]] value_type_ref back() { return operator[](size() - 1); }

	[[nodiscard]] value_type back() const { return operator[](size() - 1); }

	[[nodiscard]] data_type* data() noexcept { return &data_; }

	[[nodiscard]] data_type const* data() const { return &data_; }

	[[nodiscard]] iterator begin() { return iterator(this); }

	[[nodiscard]] const_iterator begin() const { return const_iterator(this); }

	[[nodiscard]] const_iterator cbegin() const { return begin(); }

	[[nodiscard]] iterator end() { return iterator(this, size()); }

	[[nodiscard]] const_iterator end() const { return const_iterator(this, size()); }

	[[nodiscard]] const_iterator cend() const { return end(); }

	[[nodiscard]] reverse_iterator rbegin() { return std::reverse_iterator(end()); }

	[[nodiscard]] const_reverse_iterator rbegin() const
	{
		return std::reverse_iterator(end());
	}

	[[nodiscard]] const_reverse_iterator crbegin() const { return rbegin(); }

	[[nodiscard]] reverse_iterator rend() { return std::reverse_iterator(begin()); }

	[[nodiscard]] const_reverse_iterator rend() const
	{
		return std::reverse_iterator(begin());
	}

	[[nodiscard]] const_reverse_iterator crend() const { return rend(); }

	[[nodiscard]] bool empty() const noexcept { return std::get<0>(data_).empty(); }

	[[nodiscard]] std::size_t size() const noexcept { return std::get<0>(data_).size(); }

	[[nodiscard]] std::size_t max_size() const noexcept
	{
		return std::apply([](auto&&... data) { return std::min({data.max_size()...}); },
		                  data_);
	}

	void reserve(std::size_t new_cap)
	{
		std::apply([new_cap](auto... data) { (data.reserve(new_cap), ...); }, data_);
	}

	[[nodiscard]] std::size_t capacity() const noexcept
	{
		return std::apply([](auto&&... data) { return std::min({data.capacity()...}); },
		                  data_);
	}

	void shrink_to_fit()
	{
		std::apply([](auto... data) { (data.shrink_to_fit(), ...); }, data_);
	}

	void clear() noexcept
	{
		std::apply([](auto... data) { (data.clear(), ...); }, data_);
	}

	iterator insert(const_iterator pos, value_type const& value)
	{
		std::apply(
		    [pos, &value](auto&&... data) {
			    (data.insert(
			         std::begin(data) + pos.index_,
			         value.template get<typename std::decay_t<decltype(data)>::value_type>()),
			     ...);
		    },
		    data_);
		return iterator(this, pos.index_);
	}

	iterator insert(const_iterator pos, value_type&& value)
	{
		std::apply(
		    [pos, &value](auto&&... data) {
			    (data.insert(std::begin(data) + pos.index_,
			                 std::move(value.template get<
			                           typename std::decay_t<decltype(data)>::value_type>())),
			     ...);
		    },
		    data_);
		return iterator(this, pos.index_);
	}

	iterator insert(const_iterator pos, size_type count, value_type const& value)
	{
		std::apply(
		    [pos, count, &value](auto&&... data) {
			    (data.insert(
			         std::begin(data) + pos.index_, count,
			         value.template get<typename std::decay_t<decltype(data)>::value_type>()),
			     ...);
		    },
		    data_);

		return iterator(this, pos.index_);
	}

	template <class InputIt>
	iterator insert(const_iterator pos, InputIt first, InputIt last)
	{
		// TODO: Optimize

		auto index = pos.index_;

		for (; first != last; ++first, ++pos) {
			insert(pos, *first);
		}

		return iterator(this, index);
	}

	iterator insert(const_iterator pos, std::initializer_list<value_type> ilist)
	{
		return insert(pos, std::begin(ilist), std::end(ilist));
	}

	iterator emplace(const_iterator pos, T const& arg, Rest const&... args)
	{
		std::apply(
		    [pos, &arg, &args...](auto&&... data) {
			    std::apply(
			        [pos, &data...](auto&&... a) {
				        (data.emplace(std::begin(data) + pos.index_, a), ...);
			        },
			        std::tie(arg, args...));
		    },
		    data_);
		return iterator(this, pos.index_);
	}

	iterator erase(const_iterator pos)
	{
		std::apply(
		    [pos](auto&&... data) { (data.erase(std::begin(data) + pos.index_), ...); },
		    data_);
		return iterator(this, pos.index_);
	}

	iterator erase(const_iterator first, const_iterator last)
	{
		std::apply(
		    [first, last](auto&&... data) {
			    (data.erase(std::begin(data) + first.index_, std::begin(data) + last.index_),
			     ...);
		    },
		    data_);
		return iterator(this, first.index_);
	}

	void push_back(value_type const& value)
	{
		std::apply(
		    [&value](auto&&... data) {
			    (data.push_back(
			         value.template get<typename std::decay_t<decltype(data)>::value_type>()),
			     ...);
		    },
		    data_);
	}

	void push_back(value_type&& value)
	{
		std::apply(
		    [&value](auto&&... data) {
			    (data.push_back(std::move(
			         value.template get<typename std::decay_t<decltype(data)>::value_type>())),
			     ...);
		    },
		    data_);
	}

	template <class... Args>
	value_type_ref emplace_back(Args&&... args)
	{
		std::apply(
		    [&args...](auto&&... data) {
			    std::tuple<T, Rest...> t;
			    if constexpr (0 < sizeof...(args)) {
				    addToTuple<0>(t, std::forward<Args>(args)...);
			    }
			    std::apply([&data...](auto&&... a) { (data.push_back(a), ...); }, t);
		    },
		    data_);
		return operator[](size() - 1);
	}

	void pop_back()
	{
		std::apply([](auto... data) { (data.pop_back(), ...); }, data_);
	}

	void resize(std::size_t count)
	{
		std::apply([count](auto... data) { (data.resize(count), ...); }, data_);
	}

	void resize(std::size_t count, value_type const& value)
	{
		std::apply(
		    [count, &value](auto... data) {
			    (data.resize(
			         count,
			         value.template get<typename std::decay_t<decltype(data)>::value_type>()),
			     ...);
		    },
		    data_);
	}

	void swap(Cloud& other) noexcept(noexcept(data_.swap(other.data_)))
	{
		data_.swap(other.data_);
	}

	// Non-member functions

	friend bool operator==(Cloud<T, Rest...> const& lhs, Cloud<T, Rest...> const& rhs)
	{
		return lhs.data_ == rhs.data_;
	}

	friend bool operator!=(Cloud<T, Rest...> const& lhs, Cloud<T, Rest...> const& rhs)
	{
		return lhs.data_ != rhs.data_;
	}

	friend bool operator<(Cloud<T, Rest...> const& lhs, Cloud<T, Rest...> const& rhs)
	{
		return lhs.data_ < rhs.data_;
	}

	friend bool operator<=(Cloud<T, Rest...> const& lhs, Cloud<T, Rest...> const& rhs)
	{
		return lhs.data_ <= rhs.data_;
	}

	friend bool operator>(Cloud<T, Rest...> const& lhs, Cloud<T, Rest...> const& rhs)
	{
		return lhs.data_ > rhs.data_;
	}

	friend bool operator>=(Cloud<T, Rest...> const& lhs, Cloud<T, Rest...> const& rhs)
	{
		return lhs.data_ >= rhs.data_;
	}

	friend void swap(Cloud<T, Rest...>& lhs,
	                 Cloud<T, Rest...>& rhs) noexcept(noexcept(lhs.swap(rhs)))
	{
		lhs.swap(rhs);
	}

	friend size_type erase(Cloud<T, Rest...>& c, value_type const& value)
	{
		auto it = std::remove(std::begin(c), std::end(c), value);
		auto r  = std::end(c) - it;
		c.erase(it, std::end(c));
		return r;
	}

	template <class Pred>
	friend size_type erase_if(Cloud<T, Rest...>& c, Pred pred)
	{
		auto it = std::remove_if(std::begin(c), std::end(c), pred);
		auto r  = std::end(c) - it;
		c.erase(it, std::end(c));
		return r;
	}

	// Extra functions

	// Const at
	auto cat(std::size_t index) { return std::as_const(this)->at(index); }

	// Const operator[]
	auto value(std::size_t index) { return std::as_const(this)->operator[](index); }

	template <class E>
	std::vector<E>& get()
	{
		return std::get<std::vector<std::decay_t<E>>>(data_);
	}

	template <class E>
	std::vector<E> const& get() const
	{
		return std::get<std::vector<std::decay_t<E>>>(data_);
	}

	template <class E>
	auto data()
	{
		return get<E>().data();
	}

	template <class E>
	auto data() const
	{
		return get<E>().data();
	}

 private:
	template <std::size_t I, class Arg, class... Args>
	static void addToTuple(std::tuple<T, Rest...>& t, Arg&& first, Args&&... rest)
	{
		std::get<I>(t) = std::forward<Arg>(first);
		if constexpr (0 < sizeof...(rest)) {
			addToTuple<I + 1>(t, std::forward<Args>(rest)...);
		}
	}

 private:
	data_type data_;
};

template <class E, class T, class... Rest>
std::vector<E>& get(Cloud<T, Rest...>& cloud)
{
	return cloud.template get<E>();
}

template <class E, class T, class... Rest>
std::vector<E> const& get(Cloud<T, Rest...> const& cloud)
{
	return cloud.template get<E>();
}

//
// Type traits
//

template <class T, class... Ts>
struct contains_type<T, Cloud<Ts...>> : contains_type<T, Ts...> {
};
}  // namespace ufo

#endif  // UFO_PCL_CLOUD_HPP