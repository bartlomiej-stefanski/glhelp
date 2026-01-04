#pragma once

#include <array>
#include <cstdint>
#include <stdexcept>

#include <glad/gl.h>

namespace glhelp {

constexpr std::array< GLuint, 32 > GL_TEXTURE_UNITS{
    GL_TEXTURE0, GL_TEXTURE1, GL_TEXTURE2, GL_TEXTURE3,
    GL_TEXTURE4, GL_TEXTURE5, GL_TEXTURE6, GL_TEXTURE7,
    GL_TEXTURE8, GL_TEXTURE9, GL_TEXTURE10, GL_TEXTURE11,
    GL_TEXTURE12, GL_TEXTURE13, GL_TEXTURE14, GL_TEXTURE15,
    GL_TEXTURE16, GL_TEXTURE17, GL_TEXTURE18, GL_TEXTURE19,
    GL_TEXTURE20, GL_TEXTURE21, GL_TEXTURE22, GL_TEXTURE23,
    GL_TEXTURE24, GL_TEXTURE25, GL_TEXTURE26, GL_TEXTURE27,
    GL_TEXTURE28, GL_TEXTURE29, GL_TEXTURE30, GL_TEXTURE31};

template< GLuint TextureType >
class Texture {
public:
  [[nodiscard]] auto id() const noexcept -> GLuint { return texture_id; }
  void load_to_texture_unit(std::uint8_t texture_unit_id)
  {
    if (texture_unit_id >= GL_TEXTURE_UNITS.size()) {
      throw std::out_of_range("Texture unit ID out of range");
    }

    last_texture_unit = texture_unit_id;
    glActiveTexture(GL_TEXTURE_UNITS[texture_unit_id]);
    glBindTexture(TextureType, texture_id);
  }

  [[nodiscard]] auto get_last_texture_unit() const noexcept -> GLuint { return last_texture_unit; }

protected:
  GLuint texture_id{};
  GLuint last_texture_unit{};
};

template< typename >
struct IsTexture : std::false_type {};

template< GLuint T >
struct IsTexture< Texture< T > > : std::true_type {};

/// Creates a persistent PBO buffer for texture data uploads.
/// Using this class allows for efficient texture data transfers to the GPU via DMA.
template< typename DataT, unsigned Width, unsigned Heigth, unsigned Layers >
class PBOTextureBuffer {
public:
  PBOTextureBuffer()
      : total_size(Width * Heigth * Layers * sizeof(DataT))
  {
    glGenBuffers(1, &pbo_id);
    if (pbo_id == 0) {
      throw std::runtime_error("Failed to generate PBO");
    }

    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo_id);
    const GLbitfield flags{GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT};

    glBufferStorage(GL_PIXEL_UNPACK_BUFFER, total_size, nullptr, flags);
    data = reinterpret_cast< decltype(data) >(glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, 0, total_size, flags));
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
  }

  [[nodiscard]] auto get_layer(unsigned layer) -> std::array< std::array< DataT, Width >, Heigth >&
  {
    return data->at(layer);
  }
  [[nodiscard]] auto id() const noexcept -> GLuint { return pbo_id; }

private:
  GLuint pbo_id;
  size_t total_size;
  std::array< std::array< std::array< DataT, Width >, Heigth >, Layers >* data;
};

} // namespace glhelp
