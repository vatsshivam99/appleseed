
//
// This source file is part of appleseed.
// Visit http://appleseedhq.net/ for additional information and resources.
//
// This software is released under the MIT license.
//
// Copyright (c) 2010-2013 Francois Beaune, Jupiter Jazz Limited
// Copyright (c) 2014 Francois Beaune, The appleseedhq Organization
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#ifndef APPLESEED_RENDERER_MODELING_INPUT_TEXTURESOURCE_H
#define APPLESEED_RENDERER_MODELING_INPUT_TEXTURESOURCE_H

// appleseed.renderer headers.
#include "renderer/global/globaltypes.h"
#include "renderer/modeling/input/inputformat.h"
#include "renderer/modeling/input/source.h"
#include "renderer/modeling/scene/textureinstance.h"

// appleseed.foundation headers.
#include "foundation/image/canvasproperties.h"
#include "foundation/image/color.h"
#include "foundation/image/colorspace.h"
#include "foundation/math/vector.h"
#include "foundation/platform/compiler.h"
#include "foundation/utility/uid.h"

// Standard headers.
#include <cstddef>

// Forward declarations.
namespace renderer      { class TextureCache; }

namespace renderer
{

//
// Texture source.
//

class TextureSource
  : public Source
{
  public:
    // Constructor.
    TextureSource(
        const foundation::UniqueID          assembly_uid,
        const TextureInstance&              texture_instance,
        const foundation::CanvasProperties& texture_props,
        const InputFormat                   input_format);

    // Retrieve the texture instance used by this source.
    const TextureInstance& get_texture_instance() const;

    // Evaluate the source at a given shading point.
    virtual void evaluate(
        TextureCache&                       texture_cache,
        const foundation::Vector2d&         uv,
        double&                             scalar) const OVERRIDE;
    virtual void evaluate(
        TextureCache&                       texture_cache,
        const foundation::Vector2d&         uv,
        foundation::Color3f&                linear_rgb) const OVERRIDE;
    virtual void evaluate(
        TextureCache&                       texture_cache,
        const foundation::Vector2d&         uv,
        Spectrum&                           spectrum) const OVERRIDE;
    virtual void evaluate(
        TextureCache&                       texture_cache,
        const foundation::Vector2d&         uv,
        Alpha&                              alpha) const OVERRIDE;
    virtual void evaluate(
        TextureCache&                       texture_cache,
        const foundation::Vector2d&         uv,
        foundation::Color3f&                linear_rgb,
        Alpha&                              alpha) const OVERRIDE;
    virtual void evaluate(
        TextureCache&                       texture_cache,
        const foundation::Vector2d&         uv,
        Spectrum&                           spectrum,
        Alpha&                              alpha) const OVERRIDE;

  private:
    const foundation::UniqueID              m_assembly_uid;
    const TextureInstance&                  m_texture_instance;
    const foundation::UniqueID              m_texture_uid;
    const foundation::CanvasProperties      m_texture_props;
    const InputFormat                       m_input_format;
    const double                            m_scalar_canvas_width;
    const double                            m_scalar_canvas_height;
    const double                            m_max_x;
    const double                            m_max_y;

    // Retrieve a given texel. Return a color in the linear RGB color space.
    foundation::Color4f get_texel(
        TextureCache&                       texture_cache,
        const size_t                        ix,
        const size_t                        iy) const;

    // Retrieve a 2x2 block of texels. Texels are expressed in the linear RGB color space.
    void get_texels_2x2(
        TextureCache&                       texture_cache,
        const int                           ix,
        const int                           iy,
        foundation::Color4f&                t00,
        foundation::Color4f&                t10,
        foundation::Color4f&                t01,
        foundation::Color4f&                t11) const;

    // Sample the texture. Return a color in the linear RGB color space.
    foundation::Color4f sample_texture(
        TextureCache&                       texture_cache,
        const foundation::Vector2d&         uv) const;

    // Compute an alpha value given a linear RGBA color and the alpha mode of the texture instance.
    void evaluate_alpha(
        const foundation::Color4f&          color,
        Alpha&                              alpha) const;
};


//
// TextureSource class implementation.
//

inline const TextureInstance& TextureSource::get_texture_instance() const
{
    return m_texture_instance;
}

inline void TextureSource::evaluate(
    TextureCache&                           texture_cache,
    const foundation::Vector2d&             uv,
    double&                                 scalar) const
{
    const foundation::Color4f color = sample_texture(texture_cache, uv);

    scalar = static_cast<double>(color[0]);
}

inline void TextureSource::evaluate(
    TextureCache&                           texture_cache,
    const foundation::Vector2d&             uv,
    foundation::Color3f&                    linear_rgb) const
{
    const foundation::Color4f color = sample_texture(texture_cache, uv);

    linear_rgb = color.rgb();
}

inline void TextureSource::evaluate(
    TextureCache&                           texture_cache,
    const foundation::Vector2d&             uv,
    Spectrum&                               spectrum) const
{
    const foundation::Color4f color = sample_texture(texture_cache, uv);

    if (m_input_format == InputFormatSpectralReflectance)
        foundation::linear_rgb_reflectance_to_spectrum(color.rgb(), spectrum);
    else foundation::linear_rgb_illuminance_to_spectrum(color.rgb(), spectrum);
}

inline void TextureSource::evaluate(
    TextureCache&                           texture_cache,
    const foundation::Vector2d&             uv,
    Alpha&                                  alpha) const
{
    const foundation::Color4f color = sample_texture(texture_cache, uv);

    evaluate_alpha(color, alpha);
}

inline void TextureSource::evaluate(
    TextureCache&                           texture_cache,
    const foundation::Vector2d&             uv,
    foundation::Color3f&                    linear_rgb,
    Alpha&                                  alpha) const
{
    const foundation::Color4f color = sample_texture(texture_cache, uv);

    linear_rgb = color.rgb();

    evaluate_alpha(color, alpha);
}

inline void TextureSource::evaluate(
    TextureCache&                           texture_cache,
    const foundation::Vector2d&             uv,
    Spectrum&                               spectrum,
    Alpha&                                  alpha) const
{
    const foundation::Color4f color = sample_texture(texture_cache, uv);

    if (m_input_format == InputFormatSpectralReflectance)
        foundation::linear_rgb_reflectance_to_spectrum(color.rgb(), spectrum);
    else foundation::linear_rgb_illuminance_to_spectrum(color.rgb(), spectrum);

    evaluate_alpha(color, alpha);
}

inline void TextureSource::evaluate_alpha(
    const foundation::Color4f&              color,
    Alpha&                                  alpha) const
{
    switch (m_texture_instance.get_effective_alpha_mode())
    {
      case TextureAlphaModeAlphaChannel:
        alpha.set(color.a);
        break;

      case TextureAlphaModeLuminance:
        alpha.set(average_value(color.rgb()));
        break;

      assert_otherwise;
    }
}

}       // namespace renderer

#endif  // !APPLESEED_RENDERER_MODELING_INPUT_TEXTURESOURCE_H
