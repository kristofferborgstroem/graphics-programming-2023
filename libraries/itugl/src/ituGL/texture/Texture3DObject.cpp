//
// Created by Kristoffer Borgstrøm on 23/03/2023.
//

#include <ituGL/texture/Texture3DObject.h>
#include <cassert>
#include <iostream>

Texture3DObject::Texture3DObject()
{
}

template<>
void Texture3DObject::SetImage<std::byte>(GLint level, GLsizei width, GLsizei height, GLsizei depth, Format format,
                               InternalFormat internalFormat, std::span<const std::byte> data, Data::Type type)
{
    assert(IsBound());
    assert(data.empty() || type != Data::Type::None);
    assert(IsValidFormat(format, internalFormat));
    assert(data.empty() || data.size_bytes() == width * height * depth * GetDataComponentCount(internalFormat) * Data::GetTypeSize(type));
    glTexImage3D(GetTarget(), level, internalFormat, width, height, depth, 0, format, static_cast<GLenum>(type), data.data());

}