
#include "Cells.h"
#include "SValue.h"

Cells::Cells( ValueT data ) : data( std::move( data ) )
{}

Cells::Cells( const Cells& other )
{
  *this = other;
}

Cells& Cells::operator=( const Cells& other )
{
  if ( this != &other )
  {
    data.reserve( other.size() );
    for ( const auto& i : other.data )
    {
      data.push_back( std::make_unique< SValue >( *i ) );
    }
  }
  return *this;
}

Cells::Cells( Cells&& other ) noexcept
{
  *this = std::move( other );
}

Cells& Cells::operator=( Cells&& other ) noexcept
{
  if ( this != &other )
  {
    data = std::move( other.data );
  }
  return *this;
}

std::size_t Cells::size() const
{
  return data.size();
}

bool Cells::isEmpty() const
{
  return data.empty();
}

const Cells::ValueT& Cells::children() const
{
  return data;
}

Cells::ValueT& Cells::children()
{
  return data;
}

void Cells::append( std::unique_ptr< SValue > v )
{
  data.push_back( std::move( v ) );
}

std::unique_ptr< SValue > Cells::takeFront()
{
  std::unique_ptr< SValue > front = std::move( data.front() );
  data.erase( data.begin() );
  return front;
}

void Cells::drop( ValueT::iterator begin, ValueT::iterator end )
{
  data.erase( begin, end );
}

void Cells::drop( ValueT::iterator pos )
{
  data.erase( pos );
}

void Cells::clear()
{
  data.clear();
}

SValue* Cells::front()
{
  return data.front().get();
}

const SValue* Cells::front() const
{
  return data.front().get();
}

SValue* Cells::back()
{
  return data.back().get();
}

const SValue* Cells::back() const
{
  return data.back().get();
}

Cells::ValueT::iterator Cells::begin()
{
  return data.begin();
}

Cells::ValueT::iterator Cells::end()
{
  return data.end();
}

SValue* Cells::operator[]( std::size_t index )
{
  return data[ index ].get();
}

const SValue* Cells::operator[]( std::size_t index ) const
{
  return data[ index ].get();
}
