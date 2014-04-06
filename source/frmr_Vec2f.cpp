#include "frmr_Vec2f.h"
#include "math.h"

array<float, 2>	frmr::Vec2f::GetArray() const
{
	return point;
}

float frmr::Vec2f::GetX() const
{
	return point[0];
}

float frmr::Vec2f::GetY() const
{
	return point[1];
}

float frmr::Vec2f::Length()
{
	if ( length == -1.0f )
	{
		length = sqrt( point[0] * point[0] + point[1] * point[1] );
	}
	return length;
}

void frmr::Vec2f::Reset()
{
	Set( 0.0f, 0.0f );
    length = 0.0f;
}

void frmr::Vec2f::Set( const float x, const float y )
{
	point.at( 0 ) = x;
	point.at( 1 ) = y;
	length = -1.0f;
}

frmr::Vec2f frmr::Vec2f::Unit()
{
	Length();
	if ( length > 0.001f )
    {
        point.at( 0 ) /= length;
        point.at( 1 ) /= length;
        length = 1.0f;
    }
    else
    {
        Reset();
    }
	return *this;
}

frmr::Vec2f frmr::Vec2f::operator+ ( const Vec2f &rhs ) const
{
	return Vec2f( point[0] + rhs.GetX(), point[1] + rhs.GetY() );
}

frmr::Vec2f frmr::Vec2f::operator- ( const Vec2f &rhs ) const
{
	return Vec2f( point[0] - rhs.GetX(), point[1] - rhs.GetY() );
}

frmr::Vec2f frmr::Vec2f::operator* ( const float &rhs ) const
{
	return Vec2f( point[0] * rhs, point[1] * rhs );
}

frmr::Vec2f frmr::Vec2f::operator/ ( const float &rhs ) const
{
	return Vec2f( point[0] / rhs, point[1] / rhs );
}

frmr::Vec2f& frmr::Vec2f::operator+= ( const Vec2f &rhs )
{
	point.at( 0 ) += rhs.GetX();
	point.at( 1 ) += rhs.GetY();
	length = -1.0f;
	return *this;
}

frmr::Vec2f& frmr::Vec2f::operator-= ( const Vec2f &rhs )
{
	point.at( 0 ) -= rhs.GetX();
	point.at( 1 ) -= rhs.GetY();
	length = -1.0f;
	return *this;
}

frmr::Vec2f& frmr::Vec2f::operator*= ( const float &rhs )
{
	point.at( 0 ) *= rhs;
	point.at( 1 ) *= rhs;
	length = -1.0f;
	return *this;
}

frmr::Vec2f& frmr::Vec2f::operator/= ( const float &rhs )
{
	point.at( 0 ) /= rhs;
	point.at( 1 ) /= rhs;
	length = -1.0f;
	return *this;
}

frmr::Vec2f::Vec2f( const float x, const float y )
	: point({{ x, y }}), length( -1.0f )
{
}

frmr::Vec2f::Vec2f()
	: point({{ 0.0f, 0.0f }}), length( 0.0f )
{
}

frmr::Vec2f::~Vec2f()
{
}
