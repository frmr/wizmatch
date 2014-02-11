#ifndef FRMR_VEC3F_H
#define FRMR_VEC3F_H

namespace frmr
{
	class Vec3f
	{
	private:
		float   x, y, z;
		float   length;

	public:
	    float   GetX() const;
		float   GetY() const;
		float   GetZ() const;
		float   Length();
		void    Reset();
		Vec3f   Unit();

	public:
		Vec3f 	operator+ ( const Vec3f &rhs ) const;
		Vec3f 	operator- ( const Vec3f &rhs ) const;
		Vec3f 	operator* ( const float &rhs ) const;
		Vec3f   operator/ ( const float &rhs ) const;
		Vec3f& 	operator+= ( const Vec3f &rhs );
		Vec3f& 	operator-= ( const Vec3f &rhs );
		Vec3f& 	operator*= ( const float &rhs );
		Vec3f&	operator/= ( const float &rhs );

	public:
		Vec3f( const float x, const float y, const float z );
		Vec3f();
		~Vec3f();
	};
}

#endif
