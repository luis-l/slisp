
#include "Lambda.h"
#include "SValue.h"

Lambda::Lambda() = default;

Lambda::Lambda( Environment e, std::unique_ptr< SValue > formals, std::unique_ptr< SValue > body )
: env( std::move( e ) ), formals( std::move( formals ) ), body( std::move( body ) )
{}

Lambda::Lambda( const Lambda& other )
{
  *this = other;
}

Lambda& Lambda::operator=( const Lambda& other )
{
  if ( this != &other )
  {
    env = other.env;
    formals = std::make_unique< SValue >( *other.formals );
    body = std::make_unique< SValue >( *other.body );
  }
  return *this;
}

Lambda::Lambda( Lambda&& other ) noexcept
{
  *this = std::move( other );
}

Lambda& Lambda::operator=( Lambda&& other ) noexcept
{
  if ( this != &other )
  {
    env = std::move( other.env );
    formals = std::move( other.formals );
    body = std::move( other.body );
  }
  return *this;
}
