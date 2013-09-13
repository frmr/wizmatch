#ifndef LIGHT_H
#define LIGHT_H

#include <stdint.h>
#include <SFML/OpenGL.hpp>
#include <vector>

#include "frmr_Vec3f.h"

using std::vector;

class Light
{
private:
    class ShadowVolume
    {
    private:
        int16_t zone;
        GLuint  displayList;
    };
private:
    static constexpr float  linearAttenuation = 0.3f;
    static constexpr float  quadraticAttenuation = 0.01f;
    static constexpr float  intensityLowerBound = 0.003f;
    frmr::Vec3f             position;
    frmr::Vec3f             color;
    float                   radius;

    vector<ShadowVolume>    staticShadowVolumes;
    //vector<vector<

private:
    float CalculateRadius() const;

public:
    frmr::Vec3f GetColor() const;
    float       GetLinearAttenuation() const;
    frmr::Vec3f GetPosition() const;
    float       GetRadius() const;
    float       GetQuadraticAttenuation() const;

public:
    Light( const frmr::Vec3f &position, const frmr::Vec3f &color );
};

#endif // LIGHT_H
