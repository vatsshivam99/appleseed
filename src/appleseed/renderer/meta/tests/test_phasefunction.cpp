
//
// This source file is part of appleseed.
// Visit http://appleseedhq.net/ for additional information and resources.
//
// This software is released under the MIT license.
//
// Copyright (c) 2017 Artem Bishev, The appleseedhq Organization
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

// appleseed.renderer headers.
#include "renderer/global/globaltypes.h"
#include "renderer/kernel/intersection/intersector.h"
#include "renderer/kernel/lighting/tracer.h"
#include "renderer/kernel/rendering/rendererservices.h"
#include "renderer/kernel/shading/oslshadergroupexec.h"
#include "renderer/kernel/shading/shadingcontext.h"
#include "renderer/kernel/texturing/texturecache.h"
#include "renderer/kernel/texturing/texturestore.h"
#include "renderer/modeling/entity/onframebeginrecorder.h"
#include "renderer/modeling/input/scalarsource.h"
#include "renderer/modeling/phasefunction/henyeyphasefunction.h"
#include "renderer/modeling/phasefunction/phasefunction.h"
#include "renderer/modeling/scene/containers.h"
#include "renderer/modeling/scene/scene.h"
#include "renderer/modeling/texture/texture.h"
#include "renderer/utility/paramarray.h"
#include "renderer/utility/testutils.h"

// appleseed.foundation headers.
#include "foundation/math/rng/mersennetwister.h"
#include "foundation/math/sampling/mappings.h"
#include "foundation/math/scalar.h"
#include "foundation/math/vector.h"
#include "foundation/utility/arena.h"
#include "foundation/utility/autoreleaseptr.h"
#include "foundation/utility/gnuplotfile.h"
#include "foundation/utility/test.h"

// Boost headers.
#include "boost/bind.hpp"

// Standard headers.
#include <vector>

using namespace foundation;
using namespace renderer;

TEST_SUITE(Renderer_Modeling_PhaseFunction)
{
    struct Fixture
      : public TestFixtureBase
    {
        // Number of MC samples to do integrations.
        static const int NumberOfSamples = 200000;
        // Number of samples to draw.
        static const int NumberOfSamplesPlot = 100;

        ParamArray base_parameters;
        const HenyeyPhaseFunctionFactory phase_function_factory;

        Fixture()
          : phase_function_factory()
        {
            base_parameters
                .insert("scattering", "0.5")
                .insert("scattering_multiplier", "1.0")
                .insert("extinction", "0.5")
                .insert("extinction_coefficient", "1.0");
        }

        template <typename ReturnType, typename Procedure>
        ReturnType setup_environment_and_evaluate(Procedure procedure)
        {
            TextureStore texture_store(m_scene);
            TextureCache texture_cache(texture_store);

            boost::shared_ptr<OIIO::TextureSystem> texture_system(
                OIIO::TextureSystem::create(),
                boost::bind(&OIIO::TextureSystem::destroy, _1));

            RendererServices renderer_services(
                m_project,
                *texture_system);

            boost::shared_ptr<OSL::ShadingSystem> shading_system(
                new OSL::ShadingSystem(&renderer_services, texture_system.get()));

            Intersector intersector(
                m_project.get_trace_context(),
                texture_cache);

            Arena arena;
            OSLShaderGroupExec sg_exec(*shading_system, arena);

            Tracer tracer(
                m_scene,
                intersector,
                texture_cache,
                sg_exec);

            ShadingContext shading_context(
                intersector,
                tracer,
                texture_cache,
                *texture_system,
                sg_exec,
                arena,
                0);

            return procedure(shading_context, arena);
        }

        // Integrate PDF of phase function (direction sampling) using straightforward Monte-Carlo approach.
        static float integrate_phase_function_direction_pdf(
            PhaseFunction& phase_function,
            ShadingContext& shading_context,
            Arena& arena)
        {
            ShadingRay shading_ray;
            shading_ray.m_org = Vector3d(0.0f, 0.0f, 0.0f);
            shading_ray.m_dir = Vector3d(1.0f, 0.0f, 0.0f);
            void* data = phase_function.evaluate_inputs(shading_context, shading_ray);
            phase_function.prepare_inputs(arena, shading_ray, data);

            MersenneTwister rng;
            SamplingContext sampling_context(rng, SamplingContext::RNGMode);

            float integral = 0.0f;
            for (int i = 0; i < NumberOfSamples; ++i)
            {
                sampling_context.split_in_place(2, 1);
                Vector2f s = sampling_context.next2<Vector2f>();
                Vector3f incoming = sample_sphere_uniform<float>(s);
                integral += phase_function.evaluate(shading_ray, data, 0.5f, incoming);
            }

            return integral * foundation::FourPi<float>() / NumberOfSamples;
        }

        // Check if probabilistic sampling is consistent with the returned PDF values.
        static Vector3f check_direction_sampling_consistency(
            PhaseFunction& phase_function,
            ShadingContext& shading_context,
            Arena& arena)
        {
            ShadingRay shading_ray;
            shading_ray.m_org = Vector3d(0.0f, 0.0f, 0.0f);
            shading_ray.m_dir = Vector3d(1.0f, 0.0f, 0.0f);
            void* data = phase_function.evaluate_inputs(shading_context, shading_ray);
            phase_function.prepare_inputs(arena, shading_ray, data);

            MersenneTwister rng;
            SamplingContext sampling_context(rng, SamplingContext::RNGMode);

            Vector3f bias = Vector3f(0.0f);
            for (int i = 0; i < NumberOfSamples; ++i)
            {
                Vector3f incoming;
                const float pdf = phase_function.sample(
                    sampling_context, shading_ray, data, 0.5f, incoming);
                bias += incoming / pdf;
            }

            return bias / NumberOfSamples;
        }

        static float get_aposteriori_average_cosine(
            PhaseFunction& phase_function,
            ShadingContext& shading_context,
            Arena& arena)
        {
            ShadingRay shading_ray;
            shading_ray.m_org = Vector3d(0.0f, 0.0f, 0.0f);
            shading_ray.m_dir = Vector3d(1.0f, 0.0f, 0.0f);
            void* data = phase_function.evaluate_inputs(shading_context, shading_ray);
            phase_function.prepare_inputs(arena, shading_ray, data);

            MersenneTwister rng;
            SamplingContext sampling_context(rng, SamplingContext::RNGMode);

            Vector3f bias = Vector3f(0.0f);
            for (int i = 0; i < NumberOfSamples; ++i)
            {
                Vector3f incoming;
                phase_function.sample(
                    sampling_context, shading_ray, data, 0.5f, incoming);
                bias += incoming;
            }

            return bias.x / NumberOfSamples;
        }

        static std::vector<Vector2f> generate_samples_for_plot(
            PhaseFunction& phase_function,
            ShadingContext& shading_context,
            Arena& arena)
        {
            ShadingRay shading_ray;
            shading_ray.m_org = Vector3d(0.0f, 0.0f, 0.0f);
            shading_ray.m_dir = Vector3d(1.0f, 0.0f, 0.0f);
            void* data = phase_function.evaluate_inputs(shading_context, shading_ray);
            phase_function.prepare_inputs(arena, shading_ray, data);

            MersenneTwister rng;
            SamplingContext sampling_context(rng, SamplingContext::RNGMode);

            std::vector<Vector2f> points;
            points.reserve(NumberOfSamples);
            for (int i = 0; i < NumberOfSamplesPlot; ++i)
            {
                Vector3f incoming;
                const float pdf = phase_function.sample(
                    sampling_context, shading_ray, data, 0.5f, incoming);
                points.emplace_back(incoming.x * pdf, incoming.y * pdf);
            }

            return points;
        }
    };

    TEST_CASE_F(CheckHenyeyDirectionPdfIntegratesToOne, Fixture)
    {
        static const float G[4] = { -0.5f, 0.0f, +0.3f, +0.8f };
        for (int i = 0; i < countof(G); ++i)
        {
            auto_release_ptr<PhaseFunction> phase_function =
                phase_function_factory.create("phase_function", base_parameters);
            phase_function->get_inputs().find("average_cosine").bind(new ScalarSource(G[i]));

            const float integral = setup_environment_and_evaluate<float>(
                boost::bind(
                    &Fixture::integrate_phase_function_direction_pdf,
                    boost::ref(*phase_function.get()), _1, _2));

            EXPECT_FEQ_EPS(1.0f, integral, 0.05f);
        }
    }

    TEST_CASE_F(CheckHenyeyDirectionSamplingConsistency, Fixture)
    {
        static const float G[4] = { -0.5f, 0.0f, +0.3f, +0.8f };
        for (int i = 0; i < countof(G); ++i)
        {
            auto_release_ptr<PhaseFunction> phase_function =
                phase_function_factory.create("phase_function", base_parameters);
            phase_function->get_inputs().find("average_cosine").bind(new ScalarSource(G[i]));

            const Vector3f bias = setup_environment_and_evaluate<Vector3f>(
                boost::bind(
                    &Fixture::check_direction_sampling_consistency,
                    boost::ref(*phase_function.get()), _1, _2));

            EXPECT_FEQ_EPS(0.0f, bias.x, 0.05f);
            EXPECT_FEQ_EPS(0.0f, bias.y, 0.05f);
            EXPECT_FEQ_EPS(0.0f, bias.z, 0.05f);
        }
    }

    TEST_CASE_F(CheckHenyeyAverageCosine, Fixture)
    {
        static const float G[4] = { -0.5f, 0.0f, +0.3f, +0.8f };
        for (int i = 0; i < countof(G); ++i)
        {
            auto_release_ptr<PhaseFunction> phase_function =
                phase_function_factory.create("phase_function", base_parameters);
            phase_function->get_inputs().find("average_cosine").bind(new ScalarSource(G[i]));

            const float average_cosine = setup_environment_and_evaluate<float>(
                boost::bind(
                    &Fixture::get_aposteriori_average_cosine,
                    boost::ref(*phase_function.get()), _1, _2));

            EXPECT_FEQ_EPS(G[i], average_cosine, 0.05f);
        }
    }

    TEST_CASE_F(PlotHenyeySamples, Fixture)
    {
        GnuplotFile plotfile;
        plotfile.set_title(
            "Samples of Henyey-Greenstein Phase Function"
            " (multiplied to PDF)");
        plotfile.set_xlabel("X");
        plotfile.set_ylabel("Y");
        plotfile.set_xrange(-0.6, +0.6);
        plotfile.set_yrange(-0.3, +0.3);

        static const char* Colors[3] =
        {
            "blue",
            "red",
            "magenta",
        };
        static const float G[3] = { -0.5f, +0.3f, 0.0f };
        for (int i = 0; i < countof(G); ++i)
        {
            auto_release_ptr<PhaseFunction> phase_function =
                phase_function_factory.create("phase_function", base_parameters);
            phase_function->get_inputs().find("average_cosine").bind(new ScalarSource(G[i]));

            const std::vector<Vector2f> points =
                setup_environment_and_evaluate<std::vector<Vector2f>>(
                    boost::bind(
                        &Fixture::generate_samples_for_plot,
                        boost::ref(*phase_function.get()), _1, _2));

            plotfile
                .new_plot()
                .set_points(points)
                .set_title("G = " + to_string(G[i]))
                .set_color(Colors[i])
                .set_style("points");
        }

        plotfile.write("unit tests/outputs/test_phasefunction_henyey_samples.gnuplot");
    }
}
