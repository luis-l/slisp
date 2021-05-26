#pragma once

#include <memory>
#include <vector>

class SValue;

// Cells are Semi-Regular type.
class Cells
{
public:
  using ValueT = std::vector< std::unique_ptr< SValue > >;

  Cells() = default;
  Cells( ValueT data );

  Cells( const Cells& other );
  Cells& operator=( const Cells& other );

  Cells( Cells&& other ) noexcept;
  Cells& operator=( Cells&& other ) noexcept;

  std::size_t size() const;
  bool isEmpty() const;

  const ValueT& children() const;
  ValueT& children();

  void append( std::unique_ptr< SValue > v );

  std::unique_ptr< SValue > takeFront();
  void drop( ValueT::iterator begin, ValueT::iterator end );
  void drop( ValueT::iterator pos );
  void clear();

  SValue* operator[]( std::size_t index );
  const SValue* operator[]( std::size_t index ) const;

  SValue* front();
  const SValue* front() const;

  SValue* back();
  const SValue* back() const;

  ValueT::iterator begin();
  ValueT::iterator end();

private:
  ValueT data;
};
