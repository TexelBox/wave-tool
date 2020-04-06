#include "render-engine.h"

#include <array>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

namespace wave_tool {
    RenderEngine::RenderEngine(GLFWwindow *window) {
        int width, height;
        glfwGetWindowSize(window, &width, &height);

        //NOTE: near distance must be small enough to not conflict with skybox size
        m_camera = std::make_shared<Camera>(72.0f, (float)width / height, 0.1f, 5000.0f, glm::vec3(0.0f, 1000.0f, 1000.0f));

        skyboxProgram = ShaderTools::compileShaders("../../assets/shaders/skybox.vert", "../../assets/shaders/skybox.frag");
        skyboxStarsProgram = ShaderTools::compileShaders("../../assets/shaders/skybox-stars.vert", "../../assets/shaders/skybox-stars.frag");
        skysphereProgram = ShaderTools::compileShaders("../../assets/shaders/skysphere.vert", "../../assets/shaders/skysphere.frag");
        trivialProgram = ShaderTools::compileShaders("../../assets/shaders/trivial.vert", "../../assets/shaders/trivial.frag");
        mainProgram = ShaderTools::compileShaders("../../assets/shaders/main.vert", "../../assets/shaders/main.frag");
        //lightProgram = ShaderTools::compileShaders("../assets/shaders/light.vert", "../assets/shaders/light.frag");
        waterGridProgram = ShaderTools::compileShaders("../../assets/shaders/water-grid.vert", "../../assets/shaders/water-grid.frag");

        //NOTE: currently placing the light at the top of the y-axis
        //lightPos = glm::vec3(0.0f, 500.0f, 0.0f);

        // Set OpenGL state
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_LINE_SMOOTH);
        glPointSize(30.0f);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    }

    std::shared_ptr<Camera> RenderEngine::getCamera() const {
        return m_camera;
    }

    // Called to render provided objects under view matrix
    void RenderEngine::render(std::shared_ptr<const MeshObject> skyboxStars, std::shared_ptr<const MeshObject> skysphere, std::shared_ptr<const MeshObject> skyboxClouds, std::shared_ptr<const MeshObject> waterGrid, std::vector<std::shared_ptr<MeshObject>> const& objects) {
        glm::mat4 const view = m_camera->getViewMat();
        glm::mat4 const projection = m_camera->getProjectionMat();
        glm::mat4 const viewProjection = projection * view;
        glm::mat4 const inverseViewProjection = glm::inverse(viewProjection);

        // compute sun position...
        float const timeOfDayInDays{timeOfDayInHours / 24.0f};
        float const timeThetaInRadians{timeOfDayInDays * glm::two_pi<float>() - glm::half_pi<float>()};
        glm::vec3 const sunPosition{glm::cos(timeThetaInRadians), glm::sin(timeThetaInRadians), 0.0f};

        float const oneMinusCloudProportion = 1.0f - cloudProportion;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // render skybox (star layer) on top of clear colour...
        // reference: https://learnopengl.com/Advanced-OpenGL/Cubemaps
        // reference: http://antongerdelan.net/opengl/cubemaps.html
        if (nullptr != skyboxStars && skyboxStars->m_isVisible) {
            // disable depth writing to draw skybox behind everything drawn after
            glDepthMask(GL_FALSE);
            // enable skybox shader
            glUseProgram(skyboxStarsProgram);
            glm::mat4 const viewNoTranslation = glm::mat4(glm::mat3(view));
            glm::mat4 const VPNoTranslation = projection * viewNoTranslation;
            // set VP matrix uniform in shader program
            glUniformMatrix4fv(glGetUniformLocation(skyboxStarsProgram, "VPNoTranslation"), 1, GL_FALSE, glm::value_ptr(VPNoTranslation));
            // bind geometry data...
            glBindVertexArray(skyboxStars->vao);

            // bind texture...
            glActiveTexture(GL_TEXTURE0 + skyboxStars->textureID);
            glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxStars->textureID);
            // set skyboxStars samplerCube uniform in shader program
            glUniform1i(glGetUniformLocation(skyboxStarsProgram, "skyboxStars"), skyboxStars->textureID);

            // POINT, LINE or FILL...
            glPolygonMode(GL_FRONT_AND_BACK, skyboxStars->m_polygonMode);
            glDrawElements(skyboxStars->m_primitiveMode, skyboxStars->drawFaces.size(), GL_UNSIGNED_INT, (void*)0);
            glBindVertexArray(0); // unbind vao
            // unbind texture...
            glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
            // re-enable depth writing for next geometry
            glDepthMask(GL_TRUE);
        }

        // render skysphere on top of stars...
        if (nullptr != skysphere && skysphere->m_isVisible) {
            // disable depth writing to draw skysphere behind everything drawn after
            glDepthMask(GL_FALSE);
            // enable skysphere shader
            glUseProgram(skysphereProgram);
            glm::mat4 const viewNoTranslation = glm::mat4(glm::mat3(view));
            glm::mat4 const VPNoTranslation = projection * viewNoTranslation;
            // set VP matrix uniform in shader program
            glUniformMatrix4fv(glGetUniformLocation(skysphereProgram, "VPNoTranslation"), 1, GL_FALSE, glm::value_ptr(VPNoTranslation));
            // bind geometry data...
            glBindVertexArray(skysphere->vao);

            // bind texture...
            Texture::bind1DTexture(skysphereProgram, skysphere->textureID, "skysphere");
            glUniform1f(glGetUniformLocation(skysphereProgram, "sunHorizonDarkness"), sunHorizonDarkness);
            glUniform3fv(glGetUniformLocation(skysphereProgram, "sunPosition"), 1, glm::value_ptr(sunPosition));
            glUniform1f(glGetUniformLocation(skysphereProgram, "sunShininess"), sunShininess);
            glUniform1f(glGetUniformLocation(skysphereProgram, "sunStrength"), sunStrength);

            // POINT, LINE or FILL...
            glPolygonMode(GL_FRONT_AND_BACK, skysphere->m_polygonMode);
            glDrawElements(skysphere->m_primitiveMode, skysphere->drawFaces.size(), GL_UNSIGNED_INT, (void*)0);
            glBindVertexArray(0); // unbind vao
            // unbind texture...
            Texture::unbind1DTexture();
            // re-enable depth writing for next geometry
            glDepthMask(GL_TRUE);
        }

        // render skybox (cloud layer) on top of skysphere...
        // reference: https://learnopengl.com/Advanced-OpenGL/Cubemaps
        // reference: http://antongerdelan.net/opengl/cubemaps.html
        if (nullptr != skyboxClouds && skyboxClouds->m_isVisible) {
            // disable depth writing to draw skybox behind everything drawn after
            glDepthMask(GL_FALSE);
            // enable skybox shader
            glUseProgram(skyboxProgram);
            glm::mat4 const viewNoTranslation = glm::mat4(glm::mat3(view));
            glm::mat4 const VPNoTranslation = projection * viewNoTranslation;
            // set VP matrix uniform in shader program
            glUniformMatrix4fv(glGetUniformLocation(skyboxProgram, "VPNoTranslation"), 1, GL_FALSE, glm::value_ptr(VPNoTranslation));
            // bind geometry data...
            glBindVertexArray(skyboxClouds->vao);

            glUniform1f(glGetUniformLocation(skyboxProgram, "oneMinusCloudProportion"), oneMinusCloudProportion);
            glUniform1f(glGetUniformLocation(skyboxProgram, "overcastStrength"), overcastStrength);

            // bind texture...
            glActiveTexture(GL_TEXTURE0 + skyboxClouds->textureID);
            glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxClouds->textureID);
            // set skyboxClouds samplerCube uniform in shader program
            glUniform1i(glGetUniformLocation(skyboxProgram, "skyboxClouds"), skyboxClouds->textureID);

            glUniform3fv(glGetUniformLocation(skyboxProgram, "sunPosition"), 1, glm::value_ptr(sunPosition));

            // POINT, LINE or FILL...
            glPolygonMode(GL_FRONT_AND_BACK, skyboxClouds->m_polygonMode);
            glDrawElements(skyboxClouds->m_primitiveMode, skyboxClouds->drawFaces.size(), GL_UNSIGNED_INT, (void*)0);
            glBindVertexArray(0); // unbind vao
            // unbind texture...
            glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
            // re-enable depth writing for next geometry
            glDepthMask(GL_TRUE);
        }

        // render other objects...
        glUseProgram(mainProgram);
        for (std::shared_ptr<MeshObject const> o : objects) {
            // don't render invisible objects...
            if (!o->m_isVisible) continue;

            glBindVertexArray(o->vao);

            Texture::bind2DTexture(mainProgram, o->textureID, std::string("image"));

            glm::mat4 const model = o->getModel();
            glm::mat4 const modelView = view * model;

            glUniformMatrix4fv(glGetUniformLocation(mainProgram, "modelView"), 1, GL_FALSE, glm::value_ptr(modelView));
            glUniformMatrix4fv(glGetUniformLocation(mainProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
            //TODO: make sure the shader works with a directional light
            glUniform3fv(glGetUniformLocation(mainProgram, "lightPos"), 1, glm::value_ptr(sunPosition));
            //glUniform3fv(glGetUniformLocation(mainProgram, "lightPos"), 1, glm::value_ptr(lightPos));
            //glUniform3fv(glGetUniformLocation(mainProgram, "lightPos"), 1, glm::value_ptr(camera->getPosition())); // set light pos as camera pos

            glUniform1i(glGetUniformLocation(mainProgram, "hasTexture"), o->hasTexture);
            glUniform1i(glGetUniformLocation(mainProgram, "hasNormals"), !o->normals.empty());

            // POINT, LINE or FILL...
            glPolygonMode(GL_FRONT_AND_BACK, o->m_polygonMode);

            glDrawElements(o->m_primitiveMode, o->drawFaces.size(), GL_UNSIGNED_INT, (void*)0);

            glBindVertexArray(0);
            Texture::unbind2DTexture();
        }

        //TODO: setup clipping and rendering for local reflections/refractions

        //NOTE: the order of drawing matters for alpha-blending
        // render water...
        if (nullptr != waterGrid && waterGrid->m_isVisible && nullptr != skysphere && nullptr != skyboxClouds) {

            // reference: https://fileadmin.cs.lth.se/graphics/theses/projects/projgrid/
            //NOTE: this code closely follows the algorithm laid out by the demo at the above reference

            //TODO: have a UI slider for this
            float const WAVE_AMPLITUDE = 5.0f;
            //float const WAVE_AMPLITUDE = 0.0f;
            geometry::Plane const upperPlane{0.0f, 1.0f, 0.0f, WAVE_AMPLITUDE};
            geometry::Plane const basePlane{0.0f, 1.0f, 0.0f, 0.0f};
            geometry::Plane const lowerPlane{0.0f, 1.0f, 0.0f, -WAVE_AMPLITUDE};

            // reference: https://gamedev.stackexchange.com/questions/29999/how-do-i-create-a-bounding-frustum-from-a-view-projection-matrix
            // reference: https://stackoverflow.com/questions/7692988/opengl-math-projecting-screen-space-to-world-space-coords
            // reference: https://www.gamedev.net/forums/topic/644571-calculating-frustum-corners-from-a-projection-matrix/
            // initialize in NDC-space
            std::array<glm::vec4, 8> frustumCornerPoints{glm::vec4{-1.0f, -1.0f, -1.0f, 1.0f},  // [0] - (lbn) - left / bottom / near
                                                         glm::vec4{-1.0f, -1.0f, 1.0f, 1.0f},   // [1] - (lbf) - left / bottom / far
                                                         glm::vec4{-1.0f, 1.0f, -1.0f, 1.0f},   // [2] - (ltn) - left / top / near
                                                         glm::vec4{-1.0f, 1.0f, 1.0f, 1.0f},    // [3] - (ltf) - left / top / far
                                                         glm::vec4{1.0f, -1.0f, -1.0f, 1.0f},   // [4] - (rbn) - right / bottom / near
                                                         glm::vec4{1.0f, -1.0f, 1.0f, 1.0f},    // [5] - (rbf) - right / bottom / far
                                                         glm::vec4{1.0f, 1.0f, -1.0f, 1.0f},    // [6] - (rtn) - right / top / near
                                                         glm::vec4{1.0f, 1.0f, 1.0f, 1.0f}};    // [7] - (rtf) - right / top / far

            // transform into world-space...
            for (unsigned int i = 0; i < frustumCornerPoints.size(); ++i) {
                glm::vec4 const temp{inverseViewProjection * frustumCornerPoints.at(i)};
                frustumCornerPoints.at(i) = temp / temp.w;
            }

            // stores indices into frustumCornerPoints
            // 12 edges between pairs of points 
            std::array<unsigned int, 24> const FRUSTUM_EDGES{0,1,   // [0]  - lbn ---> lbf (across-edge)
                                                             0,2,   // [1]  - lbn ---> ltn (near-edge)
                                                             0,4,   // [2]  - lbn ---> rbn (near-edge) 
                                                             1,3,   // [3]  - lbf ---> ltf (far-edge)
                                                             1,5,   // [4]  - lbf ---> rbf (far-edge)
                                                             2,3,   // [5]  - ltn ---> ltf (across-edge) 
                                                             2,6,   // [6]  - ltn ---> rtn (near-edge)
                                                             3,7,   // [7]  - ltf ---> rtf (far-edge)
                                                             4,5,   // [8]  - rbn ---> rbf (across-edge)
                                                             4,6,   // [9]  - rbn ---> rtn (near-edge)
                                                             5,7,   // [10] - rbf ---> rtf (far-edge)
                                                             6,7};  // [11] - rtn ---> rtf (across-edge)

            // stores intersection points of camera frustum with the displaceable volume (between upper and lower bounding planes)
            std::vector<glm::vec4> intersectionPoints;

            // intersection testing with upper/lower bound planes...
            // for each frustum edge...
            for (unsigned int i = 0; i < 12; ++i) {
                unsigned int const src{FRUSTUM_EDGES.at(i * 2)};
                unsigned int const dest{FRUSTUM_EDGES.at(i * 2 + 1)};

                geometry::Line const line{frustumCornerPoints.at(src), frustumCornerPoints.at(dest)};

                // upper-bound plane
                // first, we do a quick intersection check (plane in this case can be described by all points with y = d, since the normal is <0,1,0>)
                if (glm::min(line.p0.y, line.p1.y) <= upperPlane.d && upperPlane.d <= glm::max(line.p0.y, line.p1.y)) {
                    glm::vec3 intersectionPoint;
                    bool const isIntersection{utils::linePlaneIntersection(intersectionPoint, line, upperPlane)};
                    //NOTE: we can't currently assert this is true, since there is the rare chance that the line lies in the plane (which is currently treated as no intersection for simplicity)
                    if (isIntersection) intersectionPoints.push_back(glm::vec4{intersectionPoint, 1.0f});
                }

                // lower-bound plane
                // first, we do a quick intersection check (plane in this case can be described by all points with y = d, since the normal is <0,1,0>)
                if (glm::min(line.p0.y, line.p1.y) <= lowerPlane.d && lowerPlane.d <= glm::max(line.p0.y, line.p1.y)) {
                    glm::vec3 intersectionPoint;
                    bool const isIntersection{utils::linePlaneIntersection(intersectionPoint, line, lowerPlane)};
                    //NOTE: we can't currently assert this is true, since there is the rare chance that the line lies in the plane (which is currently treated as no intersection for simplicity)
                    if (isIntersection) intersectionPoints.push_back(glm::vec4{intersectionPoint, 1.0f});
                }
            }

            // include any frustum vertices that lie within (intersect) the displaceable volume (between upper and lower bounding planes)
            // for each frustum vertex...
            for (unsigned int i = 0; i < frustumCornerPoints.size(); ++i) {
                glm::vec4 const& frustumCornerPoint{frustumCornerPoints.at(i)};
                // we do a quick intersection check (planes in this case can be described by all points with y = d, since both have normals as <0, 1, 0>)
                if (lowerPlane.d <= frustumCornerPoint.y && frustumCornerPoint.y <= upperPlane.d) intersectionPoints.push_back(frustumCornerPoint);
            }

            // only continue to render the water grid, if there were intersection points
            if (!intersectionPoints.empty()) {
                ///////////////////////////////////////////////////////////////////////////////////
                // create projector...
                // rules...
                //  1. should never aim away from base plane
                //  2. eye position must be outside visible volume (thus eye.y <= lowerPlane.d OR eye.y >= upperPlane.d)
                //  3. provide the most "pleasant" possible projector transformation
                //NOTE: due to how the triangle mesh is tessellated, the winding will always be counter-clockwise, regardless of whether the projector is above or below the base plane
                //NOTE: the projector will only differ from the camera in its position.y and pitch, thus we can just clone the camera and then apply a translation + set pitch
                //NOTE: there are two aimpoints that get interpolated between based on the camera's forward vector - two extreme cases (1. abs(cameraForward • <0,1,0>) == 1 (bird's eye) and 2. cameraForward • <0,1,0> == 0 (horizon)

                Camera projector{*m_camera};

                float const cameraDistanceFromBasePlane{m_camera->getPosition().y};
                bool const isUnderwater{cameraDistanceFromBasePlane < 0.0f};
                //TODO: make this a UI property
                float const PROJECTOR_ELEVATION_FROM_CAMERA = 7.0f;
                float const MINIMUM_PROJECTOR_DISTANCE_FROM_BASE_PLANE{WAVE_AMPLITUDE + PROJECTOR_ELEVATION_FROM_CAMERA};

                // translate the y-position of the projector, so that it lies outside the displaceable volume (with some extra elevation padding)
                if (cameraDistanceFromBasePlane < MINIMUM_PROJECTOR_DISTANCE_FROM_BASE_PLANE) {
                    if (isUnderwater) projector.translate(glm::vec3{0.0f, MINIMUM_PROJECTOR_DISTANCE_FROM_BASE_PLANE - 2.0f * cameraDistanceFromBasePlane, 0.0f});
                    else projector.translate(glm::vec3{0.0f, MINIMUM_PROJECTOR_DISTANCE_FROM_BASE_PLANE - cameraDistanceFromBasePlane, 0.0f});
                }

                // safely handle when the camera is looking too close to the horizon (shift the forward vector a bit to ensure the intersection test succeeds for aimpoint_1)
                glm::vec3 cameraForwardIntersectionSafe{m_camera->getForward()};
                float const SAFE_EPSILON{0.001f};
                if (glm::abs(cameraForwardIntersectionSafe.y) < SAFE_EPSILON) {
                    float const sign{cameraForwardIntersectionSafe.y >= 0.0f ? 1.0f : -1.0f};
                    cameraForwardIntersectionSafe.y = sign * SAFE_EPSILON;
                    //NOTE: there is no need to normalize this (and I don't want to cause the ypos will decrease)
                }

                // compute aimpoint for method 1 (bird's eye)...
                glm::vec3 aimpoint_1;
                bool const isLookingDown{cameraForwardIntersectionSafe.y < 0.0f};
                bool const isLookingDown_XOR_isUnderwater{isLookingDown != isUnderwater};
                if (isLookingDown_XOR_isUnderwater) {
                    bool const isIntersection{utils::linePlaneIntersection(aimpoint_1, geometry::Line{m_camera->getPosition(), m_camera->getPosition() + cameraForwardIntersectionSafe}, basePlane)};
                    assert(isIntersection);
                } else {
                    glm::vec3 const cameraForwardIntersectionSafeMirrored{glm::reflect(cameraForwardIntersectionSafe, basePlane.getNormalVec())};
                    bool const isIntersection{utils::linePlaneIntersection(aimpoint_1, geometry::Line{m_camera->getPosition(), m_camera->getPosition() + cameraForwardIntersectionSafeMirrored}, basePlane)};
                    assert(isIntersection);
                }

                // compute aimpoint for method 2 (horizon)...
                //TODO: make this a UI property? auto-generate it?
                float const FORWARD_FIXED_LENGTH{10.0f};
                glm::vec3 aimpoint_2{m_camera->getPosition() + FORWARD_FIXED_LENGTH * m_camera->getForward()};
                // project this point onto the base plane
                aimpoint_2.y = 0.0f;

                //NOTE: the grid changes abruptly when aimpoint_final == aimpoint2 (a == 0), but this will never occur since...
                //      I made the camera's forward vector (for the math only) intersection safe (aimpoint_1 will be defined and a != 0.0)
                // compute the interpolation coefficient in range [SAFE_EPSILON, 1.0]...
                float const a{glm::abs(cameraForwardIntersectionSafe.y)};

                // compute the final aimpoint as an interpolation between the two aimpoints...
                glm::vec3 const aimpoint_final{glm::mix(aimpoint_2, aimpoint_1, a)};

                // compute the projector's pitch in order to aim at this aimpoint...
                glm::vec3 const projectorNewForwardVec{glm::normalize(aimpoint_final - projector.getPosition())};
                glm::vec3 const projectorNewForwardVecXZProjection{glm::normalize(glm::vec3{projectorNewForwardVec.x, 0.0f, projectorNewForwardVec.z})};
                //NOTE: the projector's position will always be above water, thus the pitch will always be negative
                float projectorNewPitchDegrees{-glm::degrees(glm::acos(glm::dot(projectorNewForwardVec, projectorNewForwardVecXZProjection)))};

                // now, aim the projector...
                projector.setRotation(projector.getYaw(), projectorNewPitchDegrees);
                ///////////////////////////////////////////////////////////////////////////////////

                // project all intersection points onto base plane...
                for (unsigned int i = 0; i < intersectionPoints.size(); ++i) {
                    intersectionPoints.at(i).y = 0.0f;
                }

                // transform all intersection points into NDC-space (for projector)
                // reference: https://community.khronos.org/t/homogenous-normalized-device-coords-and-clipping/61965
                // reference: https://stackoverflow.com/questions/21841598/when-does-the-transition-from-clip-space-to-screen-coordinates-happen
                //NOTE: I was having a lot of issues before I divided by w, so hopefully everything works now
                glm::mat4 const projector_viewProjectionMat{projector.getProjectionMat() * projector.getViewMat()};
                for (unsigned int i = 0; i < intersectionPoints.size(); ++i) {
                    glm::vec4 const temp{projector_viewProjectionMat * intersectionPoints.at(i)}; // now in clip-space
                    intersectionPoints.at(i) = temp / temp.w; // now in NDC-space
                }

                // determine the xy-NDC bounds of the intersection points
                float x_min = intersectionPoints.at(0).x;
                float x_max = intersectionPoints.at(0).x;
                float y_min = intersectionPoints.at(0).y;
                float y_max = intersectionPoints.at(0).y;
                for (unsigned int i = 1; i < intersectionPoints.size(); ++i) {
                    if (intersectionPoints.at(i).x < x_min) x_min = intersectionPoints.at(i).x;
                    else if (intersectionPoints.at(i).x > x_max) x_max = intersectionPoints.at(i).x;

                    if (intersectionPoints.at(i).y < y_min) y_min = intersectionPoints.at(i).y;
                    else if (intersectionPoints.at(i).y > y_max) y_max = intersectionPoints.at(i).y;
                }

                // setup the range conversion matrix (in column-major order)
                // will be used to convert from a special uv-space ("range-space")
                //
                // |x_max - x_min,       0      , 0, x_min|
                // |      0      , y_max - y_min, 0, y_min|
                // |      0      ,       0      , 1,   0  |
                // |      0      ,       0      , 0,   1  |
                //
                glm::mat4 const rangeMat{x_max - x_min, 0.0f, 0.0f, 0.0f,
                                         0.0f, y_max - y_min, 0.0f, 0.0f,
                                         0.0f, 0.0f, 1.0f, 0.0f,
                                         x_min, y_min, 0.0f, 1.0f};

                // compute M_projector...
                glm::mat4 const projectorMat{glm::inverse(projection * projector.getViewMat()) * rangeMat};

                // compute the world-space coordinates of the four grid corners...
                // init the corner positions in a special uv-space ("range-space") - (with z = -1 (near) for convenience for intersection test below)
                std::array<glm::vec4, 4> waterGridCornerPoints{glm::vec4{0.0f, 0.0f, -1.0f, 1.0f},   // [0] - bottom-left
                                                               glm::vec4{0.0f, 1.0f, -1.0f, 1.0f},   // [1] - top-left
                                                               glm::vec4{1.0f, 0.0f, -1.0f, 1.0f},   // [2] - bottom-right
                                                               glm::vec4{1.0f, 1.0f, -1.0f, 1.0f}};  // [3] - top-right

                // transform the coordinates to world-space...
                // intersect projected rays with XZ-plane (base plane) to get world-space bounds of grid...
                for (unsigned int i = 0; i < waterGridCornerPoints.size(); ++i) {
                    glm::vec4 p0{waterGridCornerPoints.at(i)};
                    glm::vec4 p1{p0};
                    p1.z = 1.0f; // far

                    // transform both points to world-space...
                    p0 = projectorMat * p0;
                    p0 /= p0.w;
                    p1 = projectorMat * p1;
                    p1 /= p1.w;

                    // intersection test...
                    geometry::Line const line{p0, p1};
                    glm::vec3 intersectionPoint;
                    bool const isIntersection{utils::linePlaneIntersection(intersectionPoint, line, basePlane)};
                    assert(isIntersection);
                    waterGridCornerPoints.at(i) = glm::vec4{intersectionPoint, 1.0f};
                }

                // set useful aliases for the grid corners
                glm::vec4 const& bottomLeftGridPointInWorld{waterGridCornerPoints.at(0)};
                glm::vec4 const& topLeftGridPointInWorld{waterGridCornerPoints.at(1)};
                glm::vec4 const& bottomRightGridPointInWorld{waterGridCornerPoints.at(2)};
                glm::vec4 const& topRightGridPointInWorld{waterGridCornerPoints.at(3)};

                // now render...
                glUseProgram(waterGridProgram);
                glBindVertexArray(waterGrid->vao);

                // set uniforms...
                //TODO: should get uniform locations ONCE and store them (and error handle)

                //TODO: right now this just matches a constant in program.cpp, but will be attached to MeshObject in the future
                //TODO: make this a UI setting (and probably should store this with the mesh itself since drawFaces will be closely related)
                //TODO: might even split this into a width/height (or hres/vres) in the future for non-square grids
                //NOTE: this should be >= 2
                //GLuint const GRID_LENGTH = 65;
                GLuint const GRID_LENGTH = 257;

                glUniform4fv(glGetUniformLocation(waterGridProgram, "bottomLeftGridPointInWorld"), 1, glm::value_ptr(bottomLeftGridPointInWorld));
                glUniform4fv(glGetUniformLocation(waterGridProgram, "bottomRightGridPointInWorld"), 1, glm::value_ptr(bottomRightGridPointInWorld));
                glUniform3fv(glGetUniformLocation(waterGridProgram, "cameraPosition"), 1, glm::value_ptr(m_camera->getPosition()));
                glUniform1ui(glGetUniformLocation(waterGridProgram, "gridLength"), GRID_LENGTH);
                Texture::bind2DTexture(waterGridProgram, waterGrid->textureID, "heightmap");

                // bind texture...
                glActiveTexture(GL_TEXTURE0 + skyboxClouds->textureID);
                glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxClouds->textureID);
                // set skyboxClouds samplerCube uniform in shader program
                glUniform1i(glGetUniformLocation(waterGridProgram, "skyboxClouds"), skyboxClouds->textureID);

                Texture::bind1DTexture(waterGridProgram, skysphere->textureID, "skysphere");
                glUniform3fv(glGetUniformLocation(waterGridProgram, "sunPosition"), 1, glm::value_ptr(sunPosition));

                glUniform4fv(glGetUniformLocation(waterGridProgram, "topLeftGridPointInWorld"), 1, glm::value_ptr(topLeftGridPointInWorld));
                glUniform4fv(glGetUniformLocation(waterGridProgram, "topRightGridPointInWorld"), 1, glm::value_ptr(topRightGridPointInWorld));
                glUniformMatrix4fv(glGetUniformLocation(waterGridProgram, "viewProjection"), 1, GL_FALSE, glm::value_ptr(viewProjection));
                glUniform1f(glGetUniformLocation(waterGridProgram, "waveAmplitude"), WAVE_AMPLITUDE);

                // draw...
                // POINT, LINE or FILL...
                glPolygonMode(GL_FRONT_AND_BACK, waterGrid->m_polygonMode);
                glDrawElements(waterGrid->m_primitiveMode, waterGrid->drawFaces.size(), GL_UNSIGNED_INT, (void*)0);

                Texture::unbind1DTexture();

                // unbind texture...
                glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

                Texture::unbind2DTexture();
                glBindVertexArray(0); // unbind VAO
                glUseProgram(0); // unbind shader program
            }
        }

        //renderLight();
    }

/*
    // Renders the current position of the light as a point
    void RenderEngine::renderLight() {
        glUseProgram(lightProgram);

        glm::mat4 const view = camera->getLookAt();
        // Uniforms
        glUniformMatrix4fv(glGetUniformLocation(lightProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(lightProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniform3fv(glGetUniformLocation(lightProgram, "lightPos"), 1, glm::value_ptr(lightPos));

        glDrawArrays(GL_POINTS, 0, 1);
    }
*/

    void RenderEngine::assignBuffers(MeshObject &object)
    {
        std::vector<glm::vec3> const& vertices = object.drawVerts;
        std::vector<glm::vec3> const& normals = object.normals;
        std::vector<glm::vec2> const& uvs = object.uvs;
        std::vector<glm::vec3> const& colours = object.colours;
        std::vector<GLuint> const& faces = object.drawFaces;

        glGenVertexArrays(1, &object.vao);
        glBindVertexArray(object.vao);

        // Vertex buffer
        // location 0 in vao
        if (vertices.size() > 0) {
            glGenBuffers(1, &object.vertexBuffer);
            glBindBuffer(GL_ARRAY_BUFFER, object.vertexBuffer);
            glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
            glEnableVertexAttribArray(0);
        }

        // Normal buffer
        // location 1 in vao
        if (normals.size() > 0) {
            glGenBuffers(1, &object.normalBuffer);
            glBindBuffer(GL_ARRAY_BUFFER, object.normalBuffer);
            glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*normals.size(), normals.data(), GL_STATIC_DRAW);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
            glEnableVertexAttribArray(1);
        }

        if (uvs.size() > 0) {
            // UV buffer
            // location 2 in vao
            glGenBuffers(1, &object.uvBuffer);
            glBindBuffer(GL_ARRAY_BUFFER, object.uvBuffer);
            glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2)*uvs.size(), uvs.data(), GL_STATIC_DRAW);
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
            glEnableVertexAttribArray(2);
        }

        // Colour buffer
        // location 3 in vao
        if (colours.size() > 0) {
            glGenBuffers(1, &object.colourBuffer);
            glBindBuffer(GL_ARRAY_BUFFER, object.colourBuffer);
            glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*colours.size(), colours.data(), GL_STATIC_DRAW);
            glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
            glEnableVertexAttribArray(3);
        }

        // Face buffer
        if (faces.size() > 0) {
            //NOTE: assuming every object with faces is using an index buffer (thus glDrawElements is always used)
            // this is fully compatible since if an object has only verts and wanted to use glDrawArrays, then it could just initialize a trivial index buffer (0,1,2,...,verts.size()-1)
            glGenBuffers(1, &object.indexBuffer);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, object.indexBuffer);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)*faces.size(), faces.data(), GL_STATIC_DRAW);
        }

        // unbind vao
        glBindVertexArray(0);
    }

    //NOTE: this method assumes that the vector sizes have remained the same, the data in them has just changed
    //NOTE: it also assumes that the buffers have already been created and bound to the vao (by assignBuffers)
    void RenderEngine::updateBuffers(MeshObject &object, bool const updateVerts, bool const updateUVs, bool const updateNormals, bool const updateColours) {
        // nothing bound
        if (0 == object.vao) return;

        if (updateVerts && 0 != object.vertexBuffer) {
            std::vector<glm::vec3> const& newVerts = object.drawVerts;
            unsigned int const newSize = sizeof(glm::vec3)*newVerts.size();

            GLint oldSize = 0;
            glBindBuffer(GL_ARRAY_BUFFER, object.vertexBuffer);
            glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &oldSize); // get size of data in buffer

            // only update buffer data if new data is same size, otherwise buffer will be unchanged
            if (newSize == oldSize) {
                glBufferSubData(GL_ARRAY_BUFFER, 0, newSize, newVerts.data());
            }
        }

        if (updateUVs && 0 != object.uvBuffer) {
            std::vector<glm::vec2> const& newUVs = object.uvs;
            unsigned int const newSize = sizeof(glm::vec2)*newUVs.size();

            GLint oldSize = 0;
            glBindBuffer(GL_ARRAY_BUFFER, object.uvBuffer);
            glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &oldSize); // get size of data in buffer

            // only update buffer data if new data is same size, otherwise buffer will be unchanged
            if (newSize == oldSize) {
                glBufferSubData(GL_ARRAY_BUFFER, 0, newSize, newUVs.data());
            }
        }

        if (updateNormals && 0 != object.normalBuffer) {
            std::vector<glm::vec3> const& newNormals = object.normals;
            unsigned int const newSize = sizeof(glm::vec3)*newNormals.size();

            GLint oldSize = 0;
            glBindBuffer(GL_ARRAY_BUFFER, object.normalBuffer);
            glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &oldSize); // get size of data in buffer

            // only update buffer data if new data is same size, otherwise buffer will be unchanged
            if (newSize == oldSize) {
                glBufferSubData(GL_ARRAY_BUFFER, 0, newSize, newNormals.data());
            }
        }

        if (updateColours && 0 != object.colourBuffer) {
            std::vector<glm::vec3> const& newColours = object.colours;
            unsigned int const newSize = sizeof(glm::vec3)*newColours.size();

            GLint oldSize = 0;
            glBindBuffer(GL_ARRAY_BUFFER, object.colourBuffer);
            glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &oldSize); // get size of data in buffer

            // only update buffer data if new data is same size, otherwise buffer will be unchanged
            if (newSize == oldSize) {
                glBufferSubData(GL_ARRAY_BUFFER, 0, newSize, newColours.data());
            }
        }
    }

    // Creates a 1D texture
    GLuint RenderEngine::load1DTexture(std::string const& filePath) {
        int width, height, nrChannels;
        stbi_set_flip_vertically_on_load(true);
        unsigned char *data = stbi_load(filePath.c_str(), &width, &height, &nrChannels, STBI_rgb_alpha); // force RGBA conversion, but original number of 8-bit channels will remain in nrChannels
        if (nullptr == data) {
            std::cout << "ERROR: failed to read texture at path: " << filePath << std::endl;
            return 0; // error code (no OpenGL object can have id 0)
        }

        GLuint const textureID = Texture::create1DTexture(data, width * height);
        stbi_image_free(data);
        if (0 == textureID) std::cout << "ERROR: failed to create texture at path: " << filePath << std::endl;

        return textureID;
    }

    // Creates a 2D texture
    // reference: https://learnopengl.com/Getting-started/Textures
    GLuint RenderEngine::load2DTexture(std::string const& filePath) {
        int width, height, nrChannels;
        stbi_set_flip_vertically_on_load(true);
        unsigned char *data = stbi_load(filePath.c_str(), &width, &height, &nrChannels, STBI_rgb_alpha); // force RGBA conversion, but original number of 8-bit channels will remain in nrChannels
        if (nullptr == data) {
            std::cout << "ERROR: failed to read texture at path: " << filePath << std::endl;
            return 0; // error code (no OpenGL object can have id 0)
        }

        GLuint const textureID = Texture::create2DTexture(data, width, height);
        stbi_image_free(data);
        if (0 == textureID) std::cout << "ERROR: failed to create texture at path: " << filePath << std::endl;

        return textureID;
    }

    // reference: https://learnopengl.com/Advanced-OpenGL/Cubemaps
    // reference: https://www.html5gamedevs.com/topic/40806-where-can-you-find-skybox-textures/
    // modified a bit to not leak texture memory if an error happens
    // assumes 6 faces are given in order (px,nx,py,ny,pz,nz)
    GLuint RenderEngine::loadCubemap(std::vector<std::string> const& faces) {
        if (6 != faces.size()) return 0; // error code (no OpenGL object can have id 0)

        int width, height, nrChannels;
        stbi_set_flip_vertically_on_load(false); // cubemap textures shouldn't be flipped
        unsigned char *dataArr[6] = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};
        for (unsigned int i = 0; i < 6; ++i) {
            unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, STBI_rgb_alpha); // force RGBA conversion, but original number of 8-bit channels will remain in nrChannels
            if (nullptr == data) {
                std::cout << "ERROR: failed to read cubemap texture at path: " << faces[i] << std::endl;
                // cleanup previous read data...
                for (unsigned int j = 0; j < i; ++j) {
                    stbi_image_free(dataArr[j]);
                    dataArr[j] = nullptr;
                }
                return 0; // error code (no OpenGL object can have id 0)
            }
            // get here if this image file was read correctly
            dataArr[i] = data;
        }
        // get here if all 6 image files were read correctly
        GLuint textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
        // set options on currently bound texture object...
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        /*
        enum order (incremented by 1)
        GL_TEXTURE_CUBE_MAP_POSITIVE_X
        GL_TEXTURE_CUBE_MAP_NEGATIVE_X
        GL_TEXTURE_CUBE_MAP_POSITIVE_Y
        GL_TEXTURE_CUBE_MAP_NEGATIVE_Y
        GL_TEXTURE_CUBE_MAP_POSITIVE_Z
        GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
        */
        for (unsigned int i = 0; i < 6; i++) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, dataArr[i]); // save data in VRAM
            // cleanup...
            stbi_image_free(dataArr[i]);
            dataArr[i] = nullptr;
        }

        return textureID;
    }

    // Updates lightPos by specified value
    //void RenderEngine::updateLightPos(glm::vec3 add) {
    //    lightPos += add;
    //}

    // Sets projection and viewport for new width and height
    void RenderEngine::setWindowSize(int width, int height) {
        m_camera->setAspect((float)width / height);
        glViewport(0, 0, width, height);
    }
}
