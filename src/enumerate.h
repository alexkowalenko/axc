//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 29/8/2025.
//

#pragma once
#include <iterator>

// Custom enumerate class
template <typename Container> class Enumerate {
  public:
    explicit Enumerate( Container& container ) : container_( container ) {}

    class Iterator {
      public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = std::pair<std::size_t, typename Container::value_type&>;
        using pointer = value_type*;
        using reference = value_type&;

        Iterator( typename Container::iterator iter, std::size_t index ) : iter_( iter ), index_( index ) {}

        value_type operator*() const { return { index_, *iter_ }; }

        Iterator& operator++() {
            ++iter_;
            ++index_;
            return *this;
        }

        Iterator operator++( int ) {
            Iterator temp = *this;
            ++( *this );
            return temp;
        }

        bool operator==( const Iterator& other ) const { return iter_ == other.iter_; }

        bool operator!=( const Iterator& other ) const { return !( *this == other ); }

      private:
        typename Container::iterator iter_;
        std::size_t                  index_;
    };
    static_assert( std::input_iterator<Enumerate<Container>::Iterator> );
    static_assert( std::is_same_v<typename Container::value_type,
                                  typename std::iterator_traits<typename Container::iterator>::value_type>,
                   "Container::value_type must be the same as the value_type of the iterator" );
    static_assert( std::is_same_v<typename Container::const_iterator, decltype( std::declval<Container>().cbegin() )>,
                   "Container must have a begin() method that returns an iterator" );

    Iterator begin() const { return Iterator( container_.begin(), 0 ); }
    Iterator end() { return Iterator( container_.end(), container_.size() ); }

    Iterator cbegin() { return Iterator( container_.cbegin(), 0 ); }
    Iterator cend() { return Iterator( container_.cend(), container_.size() ); }

  private:
    Container& container_;
};

// Helper function to simplify usage
template <typename Container> Enumerate<Container> enumerate( Container& container ) {
    return Enumerate<Container>( container );
}