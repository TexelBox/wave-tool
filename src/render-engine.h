#ifndef WAVE_TOOL_RENDER_ENGINE_H_
#define WAVE_TOOL_RENDER_ENGINE_H_

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <array>
#include <memory>
#include <vector>

#include "camera.h"
#include "mesh-object.h"
#include "shader-tools.h"
#include "texture.h"

namespace wave_tool {
    //TODO: refactor these out to their own files...

    namespace geometry {
        // can be treated either as a line segment between the two end-points or an infite line (extrapolated from segment)
        struct Line {
            glm::vec3 p0;
            glm::vec3 p1;

            Line(glm::vec3 const& p0, glm::vec3 const& p1)
                : p0{p0}, p1{p1}
            {
                assert(p0 != p1); // assert that the bi-direction vector is non-zero
            }

            float getSegmentLength() const { return glm::distance(p0, p1); }
        };
    }
    
    namespace geometry {
        // reference: https://sites.math.washington.edu/~king/coursedir/m445w04/notes/vector/equations.html
        // infinite plane equation - contains all points <x, y, z> satisfying: ax + by + cz = d
        // plane normal vector = <a, b, c>
        // plane displacement scalar from world origin (along plane normal) = d
        struct Plane {
            float a;
            float b;
            float c;
            float d;

            Plane(float const a, float const b, float const c, float const d)
                : a{a}, b{b}, c{c}, d{d}
            {
                assert(0.0f != a || 0.0f != b || 0.0f != c); // assert that plane normal is non-zero
                //TODO: change this cause the comparison is unstable!
                assert(1.0f == a * a + b * b + c * c); // assert that normal vector is a unit vector
            }

            // returns a symbolic known-point that can be thought of as the "center" of our infinite plane
            glm::vec3 getCenterPoint() const { return d * getNormalVec(); }

            // returns the unit normal vector of the plane
            glm::vec3 getNormalVec() const { return glm::vec3{a, b, c}; }
        };
    }

    namespace utils {
        // reference: https://doxygen.reactos.org/de/d57/dll_2directx_2wine_2d3dx9__36_2math_8c.html#a63d0fdac0a1bf065069709fcdc97ad16
        // reference: https://stackoverflow.com/questions/23975555/how-to-do-ray-plane-intersection
        //NOTE: my plane definition has the d value negated vs these references, thus the math is slightly different
        // explanation...
        // first, treat the line like a ray = <x, y, z> = rayOrigin + t * rayDirection
        // second, remember that my plane is defined as A * x + B * y + C * z = d, with the planeNormal being <A, B, C> of course
        // third, the intersection point on the plane will be at <x, y, z> such that that point is the tip of the ray
        // plugging the ray components into the plane equation, we get...
        // ---> A * (origin.x + t * direction.x) + B * (origin.y + t * direction.y) + C * (origin.z + t * direction.z) = d
        // ---> (A * origin.x + B * origin.y + C * origin.z) + t * (A * direction.x + B * direction.y + C * direction.z) = d
        // ---> (planeNormal • rayOrigin) + t * (planeNormal • rayDirection) = d
        // ---> t = (d - (planeNormal • rayOrigin)) / (planeNormal • rayDirection)
        //NOTE: now since we are dealing with a bi-directional line instead of a uni-directional ray, we don't care about the sign of t. 
        // ---> intersectionPoint = rayOrigin + t * rayDirection
        inline bool linePlaneIntersection(glm::vec3 &out_intersectionPoint, geometry::Line const& line, geometry::Plane const& plane) {
            glm::vec3 const planeNormal{plane.getNormalVec()}; // already normalized
            glm::vec3 const& rayOrigin{line.p0};
            glm::vec3 const rayDirection{glm::normalize(line.p1 - line.p0)};

            float const denom{glm::dot(planeNormal, rayDirection)}; // in range [-1.0f, 1.0f]
            // if our line and plane are parallel, we would either have 0 or infinite intersection points, so we just treat both cases as one (no intersection)
            if (0.0f == denom) return false;

            float const t{(plane.d - glm::dot(planeNormal, rayOrigin)) / denom};

            out_intersectionPoint = rayOrigin + t * rayDirection;
            return true;
        }
    }

    namespace geometry {
        // reference: https://developer.nvidia.com/gpugems/gpugems/part-i-natural-effects/chapter-1-effective-water-simulation-physical-models
        //TODO: the statics are very unsafe at the moment
        struct GerstnerWave {
            public:
                static unsigned int const MAX_COUNT = 4;
                static_assert(MAX_COUNT > 0);

                inline static unsigned int Count() { return count; }
                inline static float TotalAmplitude() { return totalAmplitude; }

                float amplitude_A; // height of crest above equilibrium plane
                float frequency_w; // w = 2/L (roughly), where L =:= wavelength (crest-to-crest distance)
                float phaseConstant_phi; // phi = S x w, where S =:= speed (distance crest moves forward per second)
                float steepness_Q; // controls "sharpness" of crest
                glm::vec2 xzDirection_D; // horizontal unit vector perpendicular to the wave front along which the crest travels 

                GerstnerWave(float const amplitude_A, float const frequency_w, float const phaseConstant_phi, float const steepness_Q, glm::vec2 const& xzDirection_D)
                    : amplitude_A(amplitude_A), frequency_w(frequency_w), phaseConstant_phi(phaseConstant_phi), steepness_Q(steepness_Q), xzDirection_D(xzDirection_D)
                {
                    assert(count < MAX_COUNT);
                    assert(amplitude_A >= 0.0f);
                    assert(frequency_w >= 0.0f);
                    assert(phaseConstant_phi >= 0.0f);
                    assert(0.0f <= steepness_Q && steepness_Q <= 1.0f);
                    float const EPSILON{0.001f};
                    float const xzDirection_D_length{glm::length(xzDirection_D)};
                    assert(1.0f - EPSILON <= xzDirection_D_length && xzDirection_D_length <= 1.0f + EPSILON);

                    ++count;
                    totalAmplitude += amplitude_A;
                }

                ~GerstnerWave() {
                    --count;
                    totalAmplitude -= amplitude_A;
                }
            private:
                inline static unsigned int count = 0;
                inline static float totalAmplitude = 0.0f;
        };
    }

    class RenderEngine {
        public:
            //TODO: refactor these UI params out into a struct
            float animationSpeedTimeOfDayInSecondsPerHour = 1.0f; // in range [0.0, inf)
            float animationSpeedVerticalBounceWavePhasePeriodInSeconds = 3.0f; // in range [0.0, inf)
            float cloudProportion = 0.6f; // in range [0.0, 1.0]
            bool isAnimatingTimeOfDay = false;
            bool isAnimatingWaves = true;
            float overcastStrength = 0.0f; // in range [0.0, 1.0]
            float sunHorizonDarkness = 0.25f; // in range [0.0, 1.0]
            float sunShininess = 50.0f; // in range [0.0, inf)
            float sunStrength = 1.0f; // in range [0.0, 1.0]
            float timeOfDayInHours = 9.0f; // in range [0.0, 24.0]
            float waveAnimationTimeInSeconds = 0.0f; // in range [0.0, inf)
            float verticalBounceWaveAmplitude = 5.0f; // in range [0.0, inf)
            float verticalBounceWavePhase = 0.0f; // in range [0.0, 1.0]

            std::array<std::shared_ptr<geometry::GerstnerWave>, geometry::GerstnerWave::MAX_COUNT> gerstnerWaves;

            RenderEngine(GLFWwindow *window);

            std::shared_ptr<Camera> getCamera() const;

            void render(std::shared_ptr<const MeshObject> skyboxStars, std::shared_ptr<const MeshObject> skysphere, std::shared_ptr<const MeshObject> skyboxClouds, std::shared_ptr<const MeshObject> waterGrid, std::vector<std::shared_ptr<MeshObject>> const& objects);
            //void renderLight();
            void assignBuffers(MeshObject &object);
            void updateBuffers(MeshObject &object, bool const updateVerts, bool const updateUVs, bool const updateNormals, bool const updateColours);

            void setWindowSize(int width, int height);

            //void updateLightPos(glm::vec3 add);

            GLuint load1DTexture(std::string const& filePath);
            GLuint load2DTexture(std::string const& filePath);
            GLuint loadCubemap(std::vector<std::string> const& faces);
        private:
            std::shared_ptr<Camera> m_camera = nullptr;

            GLuint skyboxCloudsProgram;
            GLuint skyboxStarsProgram;
            GLuint skysphereProgram;
            GLuint trivialProgram;
            GLuint mainProgram;
            //GLuint lightProgram;
            GLuint waterGridProgram;

            //glm::vec3 lightPos;
    };
}

#endif // WAVE_TOOL_RENDER_ENGINE_H_
