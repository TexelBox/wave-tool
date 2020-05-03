#include "render-engine.h"

#include <array>
#include <string>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

namespace wave_tool {
    RenderEngine::RenderEngine(GLFWwindow *window) {
        glfwGetWindowSize(window, &m_windowWidth, &m_windowHeight);

        // hard-coded defaults
        gerstnerWaves.at(0) = std::make_shared<geometry::GerstnerWave>(0.06f, 1.0f, 2.0f, 1.0f, glm::vec2{1.0f, 0.0f});
        gerstnerWaves.at(1) = std::make_shared<geometry::GerstnerWave>(0.1f, 1.0f, 0.2f, 0.0f, glm::normalize(glm::vec2{1.0f, 1.0f}));

        //NOTE: near distance must be small enough to not conflict with skybox size
        m_camera = std::make_shared<Camera>(72.0f, (float)m_windowWidth / m_windowHeight, Z_NEAR, Z_FAR, glm::vec3(0.0f, 4.0f, 70.0f));

        //TODO: assert these are not 0, or wrap them and assert non-null
        screenSpaceQuadProgram = ShaderTools::compileShaders("../../assets/shaders/screen-space-quad.vert", "../../assets/shaders/screen-space-quad.frag");
        skyboxCloudsProgram = ShaderTools::compileShaders("../../assets/shaders/skybox-clouds.vert", "../../assets/shaders/skybox-clouds.frag");
        skyboxStarsProgram = ShaderTools::compileShaders("../../assets/shaders/skybox-stars.vert", "../../assets/shaders/skybox-stars.frag");
        skyboxTrivialProgram = ShaderTools::compileShaders("../../assets/shaders/skybox-trivial.vert", "../../assets/shaders/skybox-trivial.frag");
        skysphereProgram = ShaderTools::compileShaders("../../assets/shaders/skysphere.vert", "../../assets/shaders/skysphere.frag");
        trivialProgram = ShaderTools::compileShaders("../../assets/shaders/trivial.vert", "../../assets/shaders/trivial.frag");
        mainProgram = ShaderTools::compileShaders("../../assets/shaders/main.vert", "../../assets/shaders/main.frag");
        waterGridProgram = ShaderTools::compileShaders("../../assets/shaders/water-grid.vert", "../../assets/shaders/water-grid.frag");

        // Set OpenGL state
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_LINE_SMOOTH);
        glPointSize(30.0f);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

        // this dummy VAO can be used for attribute-less rendering
        glGenVertexArrays(1, &m_emptyVAO);

        // init stuff for dynamic skybox texture updating...
        // reference: https://www.youtube.com/watch?v=21UsMuFTN0k
        // reference: https://www.youtube.com/watch?v=lW_iqrtJORc
        glGenFramebuffers(1, &m_skyboxFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, m_skyboxFBO);
        // attach colour buffer to FBO
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        // unbind / reset to default screen framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        //TODO: refactor this to own function, along with part in loadCubemap()
        glGenTextures(1, &m_skyboxCubemap);
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_skyboxCubemap);
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
        for (unsigned int i = 0; i < 6; ++i) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, CUBEMAP_LENGTH, CUBEMAP_LENGTH, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr); // allocate empty chunk in VRAM
        }
        // unbind
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

        // reference: https://learnopengl.com/Advanced-OpenGL/Framebuffers
        // REUSABLE DEPTH/STENCIL RBO...
        glGenRenderbuffers(1, &m_depth24Stencil8RBO);
        glBindRenderbuffer(GL_RENDERBUFFER, m_depth24Stencil8RBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_windowWidth, m_windowHeight);
        // unbind
        glBindRenderbuffer(GL_RENDERBUFFER, 0);

        // LOCAL REFLECTIONS TEXTURE (2D)...
        glGenTextures(1, &m_localReflectionsTexture2D);
        glBindTexture(GL_TEXTURE_2D, m_localReflectionsTexture2D);
        // set options on currently bound texture object...
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // generate empty texture (2D)...
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_windowWidth, m_windowHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        // unbind
        glBindTexture(GL_TEXTURE_2D, 0);

        // LOCAL REFLECTIONS FBO...
        glGenFramebuffers(1, &m_localReflectionsFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, m_localReflectionsFBO);
        // attach colour buffer to FBO
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_localReflectionsTexture2D, 0);
        // attach depth/stencil buffer to FBO
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depth24Stencil8RBO);
        // set fragment shader (location = 0) output
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        // check FBO setup status...
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) std::cout << "ERROR: render-engine.cpp - local reflections FBO setup failed!" << std::endl;
        // unbind / reset to default screen framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // LOCAL REFRACTIONS TEXTURE (2D)...
        glGenTextures(1, &m_localRefractionsTexture2D);
        glBindTexture(GL_TEXTURE_2D, m_localRefractionsTexture2D);
        // set options on currently bound texture object...
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // generate empty texture (2D)...
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_windowWidth, m_windowHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        // unbind
        glBindTexture(GL_TEXTURE_2D, 0);

        // LOCAL REFRACTIONS FBO...
        glGenFramebuffers(1, &m_localRefractionsFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, m_localRefractionsFBO);
        // attach colour buffer to FBO
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_localRefractionsTexture2D, 0);
        // attach depth/stencil buffer to FBO
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depth24Stencil8RBO);
        // set fragment shader (location = 0) output
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        // check FBO setup status...
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) std::cout << "ERROR: render-engine.cpp - local refractions FBO setup failed!" << std::endl;
        // unbind / reset to default screen framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    RenderEngine::~RenderEngine() {
        glDeleteRenderbuffers(1, &m_depth24Stencil8RBO);

        glDeleteTextures(1, &m_localReflectionsTexture2D);
        glDeleteFramebuffers(1, &m_localReflectionsFBO);
        glDeleteTextures(1, &m_localRefractionsTexture2D);
        glDeleteFramebuffers(1, &m_localRefractionsFBO);

        glDeleteTextures(1, &m_skyboxCubemap);
        glDeleteFramebuffers(1, &m_skyboxFBO);

        glDeleteVertexArrays(1, &m_emptyVAO);

        glDeleteProgram(mainProgram);
        glDeleteProgram(screenSpaceQuadProgram);
        glDeleteProgram(skyboxCloudsProgram);
        glDeleteProgram(skyboxStarsProgram);
        glDeleteProgram(skyboxTrivialProgram);
        glDeleteProgram(skysphereProgram);
        glDeleteProgram(trivialProgram);
        glDeleteProgram(waterGridProgram);
    }

    std::shared_ptr<Camera> RenderEngine::getCamera() const {
        return m_camera;
    }

    // Called to render provided objects under view matrix
    void RenderEngine::render(std::shared_ptr<const MeshObject> skyboxStars, std::shared_ptr<const MeshObject> skysphere, std::shared_ptr<const MeshObject> skyboxClouds, std::shared_ptr<const MeshObject> waterGrid, std::vector<std::shared_ptr<MeshObject>> const& objects) {
        glm::mat4 const view = m_camera->getViewMat();
        Camera cameraOnlyYaw{*m_camera};
        cameraOnlyYaw.setRotation(cameraOnlyYaw.getYaw(), 0.0f);
        glm::mat4 const viewMatOnlyYaw{cameraOnlyYaw.getViewMat()};
        glm::mat4 const viewNoTranslation{glm::mat3{view}};
        glm::mat4 const projection = m_camera->getProjectionMat();
        glm::mat4 const viewProjection = projection * view;
        glm::mat4 const VPNoTranslation{projection * viewNoTranslation};
        glm::mat4 const inverseViewProjection = glm::inverse(viewProjection);

        // compute sun position...
        float const timeOfDayInDays{timeOfDayInHours / 24.0f};
        float const timeThetaInRadians{timeOfDayInDays * glm::two_pi<float>() - glm::half_pi<float>()};
        glm::vec3 const sunPosition{glm::cos(timeThetaInRadians), glm::sin(timeThetaInRadians), 0.0f};

        // tint fades to black when sun is lower in sky
        glm::vec4 const fogColourFarAtCurrentTime{glm::clamp(sunPosition.y, 0.0f, 1.0f) * glm::vec3{fogColourFarAtNoon}, fogColourFarAtNoon.a};

        float const oneMinusCloudProportion = 1.0f - cloudProportion;

        float const verticalBounceWavePhaseShift{verticalBounceWavePhase * glm::two_pi<float>()};
        float const verticalBounceWaveDisplacement{verticalBounceWaveAmplitude * glm::sin(verticalBounceWavePhaseShift)};

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        ///////////////////////////////////////////////////
        // dynamic skybox rendering (render 6 faces of cubemap to textures)...
        // bind FBO (switch to render to textures)
        glBindFramebuffer(GL_FRAMEBUFFER, m_skyboxFBO);

        //TODO: see if this is even needed
        // disable depth writing to draw everything in layers (NOTE: the FBO doesn't have a depth buffer)
        glDepthMask(GL_FALSE);
        // set a square viewport
        glViewport(0, 0, CUBEMAP_LENGTH, CUBEMAP_LENGTH);

        //TODO: if I ever get around to allowing exporting of the skybox, I might have to flip the image data since we are on the inside

        // render each side of skybox to texture
        //TODO: should probably only set the uniforms once
        for (unsigned int i = 0; i < 6; ++i) {
            // attach the next cube map face texture as the color attachment to render colours to
            //TODO: I think I can move this call into the FBO setup
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_skyboxCubemap, 0);

            glClear(GL_COLOR_BUFFER_BIT);

            // now render star skybox then skysphere then cloud skybox then fog...
            //NOTE: in the future, I could also render more objects (like far mountains/land) on top of everything
            //TODO: refactor this into own function

            // render skybox (star layer) on top of clear colour...
            // reference: https://learnopengl.com/Advanced-OpenGL/Cubemaps
            // reference: http://antongerdelan.net/opengl/cubemaps.html
            if (nullptr != skyboxStars && skyboxStars->m_isVisible) {
                // enable star shader program
                glUseProgram(skyboxStarsProgram);
                // bind geometry data...
                glBindVertexArray(skyboxStars->vao);

                // set uniforms...
                //TODO: refactor into own function
                // bind texture...
                glActiveTexture(GL_TEXTURE0 + skyboxStars->textureID);
                glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxStars->textureID);
                glUniform1i(glGetUniformLocation(skyboxStarsProgram, "skyboxStars"), skyboxStars->textureID);
                glUniformMatrix4fv(glGetUniformLocation(skyboxStarsProgram, "VPNoTranslation"), 1, GL_FALSE, glm::value_ptr(CUBEMAP_VP_NO_TRANSLATION_MATS.at(i)));

                // POINT, LINE or FILL...
                glPolygonMode(GL_FRONT_AND_BACK, skyboxStars->m_polygonMode);
                glDrawElements(skyboxStars->m_primitiveMode, skyboxStars->drawFaces.size(), GL_UNSIGNED_INT, (void*)0);

                //TODO: refactor into own function
                // unbind texture...
                glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
                // unbind
                glBindVertexArray(0);
            }

            // render skysphere on top of stars...
            if (nullptr != skysphere && skysphere->m_isVisible) {
                // enable skysphere shader program
                glUseProgram(skysphereProgram);
                // bind geometry data...
                glBindVertexArray(skysphere->vao);

                // set uniforms...
                Texture::bind1DTexture(skysphereProgram, skysphere->textureID, "skysphere");
                glUniform1f(glGetUniformLocation(skysphereProgram, "sunHorizonDarkness"), sunHorizonDarkness);
                glUniform3fv(glGetUniformLocation(skysphereProgram, "sunPosition"), 1, glm::value_ptr(sunPosition));
                glUniform1f(glGetUniformLocation(skysphereProgram, "sunShininess"), sunShininess);
                glUniform1f(glGetUniformLocation(skysphereProgram, "sunStrength"), sunStrength);
                glUniformMatrix4fv(glGetUniformLocation(skysphereProgram, "VPNoTranslation"), 1, GL_FALSE, glm::value_ptr(CUBEMAP_VP_NO_TRANSLATION_MATS.at(i)));

                // POINT, LINE or FILL...
                glPolygonMode(GL_FRONT_AND_BACK, skysphere->m_polygonMode);
                glDrawElements(skysphere->m_primitiveMode, skysphere->drawFaces.size(), GL_UNSIGNED_INT, (void*)0);

                Texture::unbind1DTexture();
                // unbind
                glBindVertexArray(0);
            }

            // render skybox (cloud layer) on top of skysphere...
            // reference: https://learnopengl.com/Advanced-OpenGL/Cubemaps
            // reference: http://antongerdelan.net/opengl/cubemaps.html
            if (nullptr != skyboxClouds && skyboxClouds->m_isVisible) {
                // enable cloud shader program
                glUseProgram(skyboxCloudsProgram);
                // bind geometry data...
                glBindVertexArray(skyboxClouds->vao);

                // set uniforms...
                glUniform1f(glGetUniformLocation(skyboxCloudsProgram, "oneMinusCloudProportion"), oneMinusCloudProportion);
                glUniform1f(glGetUniformLocation(skyboxCloudsProgram, "overcastStrength"), overcastStrength);
                //TODO: refactor into own function
                // bind texture...
                glActiveTexture(GL_TEXTURE0 + skyboxClouds->textureID);
                glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxClouds->textureID);
                glUniform1i(glGetUniformLocation(skyboxCloudsProgram, "skyboxClouds"), skyboxClouds->textureID);
                glUniform3fv(glGetUniformLocation(skyboxCloudsProgram, "sunPosition"), 1, glm::value_ptr(sunPosition));
                glUniformMatrix4fv(glGetUniformLocation(skyboxCloudsProgram, "VPNoTranslation"), 1, GL_FALSE, glm::value_ptr(CUBEMAP_VP_NO_TRANSLATION_MATS.at(i)));

                // POINT, LINE or FILL...
                glPolygonMode(GL_FRONT_AND_BACK, skyboxClouds->m_polygonMode);
                glDrawElements(skyboxClouds->m_primitiveMode, skyboxClouds->drawFaces.size(), GL_UNSIGNED_INT, (void*)0);

                //TODO: refactor into own function
                // unbind texture...
                glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
                // unbind
                glBindVertexArray(0);
            }

            // render fog layer on top of clouds...
            if (0 != m_emptyVAO) {
                // enable screen-space-quad shader program
                glUseProgram(screenSpaceQuadProgram);
                // bind geometry data...
                glBindVertexArray(m_emptyVAO);

                // set uniforms...
                glUniform1i(glGetUniformLocation(screenSpaceQuadProgram, "isTextured"), GL_FALSE);
                Texture::bind2DTexture(screenSpaceQuadProgram, 0, "textureData"); // no texture
                glUniform4fv(glGetUniformLocation(screenSpaceQuadProgram, "solidColour"), 1, glm::value_ptr(fogColourFarAtCurrentTime));

                // POINT, LINE or FILL...
                glPolygonMode(GL_FRONT_AND_BACK, PolygonMode::FILL);
                glDrawArrays(PrimitiveMode::TRIANGLE_STRIP, 0, 4);

                Texture::unbind2DTexture();
                // unbind
                glBindVertexArray(0);
            }
        }

        // reset viewport back to match GLFW window
        glViewport(0, 0, m_windowWidth, m_windowHeight);
        // re-enable depth writing for the rest of the scene
        glDepthMask(GL_TRUE);
        // unbind / reset to default screen framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        ///////////////////////////////////////////////////

        ///////////////////////////////////////////////////
        // RENDER LOCAL REFLECTIONS TO TEXTURE...
        glBindFramebuffer(GL_FRAMEBUFFER, m_localReflectionsFBO);

        glEnable(GL_CLIP_DISTANCE0);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        //NOTE: this must be clockwise since we are mirroring our scene across the XZ-plane which will flip the winding
        glFrontFace(GL_CW);

        // alpha of 0.0 is used to indicate no local reflection at fragment (i.e. the skybox is here and is already handled in global reflections)
        glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // render other objects...
        //TODO: currently not rendering any objects using trivial shader program (cause I don't want to change those shaders), but this is fine since only the debug planes currently are shaded this way
        //      I could create a modified trivial shader program, or switch everything to the main shader program and then just skip rendering objects flagged as DEBUG
        //TODO: optimize by batch-drawing objects that use the same shader program, as well as removing redundant uniform setting
        //TODO: design some sort of wrapper around shader programs that can dynamically set all uniforms properly

        // in column-major order
        // mirrors world-space position about the XZ-plane
        glm::mat4 const LOCAL_REFLECTIONS_MATRIX{1.0f, 0.0f, 0.0f, 0.0f,
                                                 0.0f, -1.0f, 0.0f, 0.0f,
                                                 0.0f, 0.0f, 1.0f, 0.0f,
                                                 0.0f, 0.0f, 0.0f, 1.0f};

        // <A, B, C, D> where Ax + By + Cz = D
        // clipping test will succeed if underneath XZ-plane
        //TODO: see if any padding is needed to hide artifacts when grazing the surface
        glm::vec4 const LOCAL_REFLECTIONS_CLIP_PLANE{0.0f, -1.0f, 0.0f, 0.0f};

        for (std::shared_ptr<MeshObject const> o : objects) {
            assert(0 != o->shaderProgramID);

            // don't render invisible objects...
            if (!o->m_isVisible) continue;

            if (o->shaderProgramID == mainProgram) {
                glm::mat4 const modelMat{LOCAL_REFLECTIONS_MATRIX * o->getModel()};
                glm::mat4 const modelView{view * modelMat};

                // enable shader program...
                glUseProgram(mainProgram);
                // bind geometry data...
                glBindVertexArray(o->vao);

                // set uniforms...
                glUniform4fv(glGetUniformLocation(mainProgram, "clipPlane0"), 1, glm::value_ptr(LOCAL_REFLECTIONS_CLIP_PLANE));
                glUniform4fv(glGetUniformLocation(mainProgram, "fogColourFarAtCurrentTime"), 1, glm::value_ptr(fogColourFarAtCurrentTime));
                glUniform1f(glGetUniformLocation(mainProgram, "fogDepthRadiusFar"), fogDepthRadiusFar);
                glUniform1f(glGetUniformLocation(mainProgram, "fogDepthRadiusNear"), fogDepthRadiusNear);
                glUniform1i(glGetUniformLocation(mainProgram, "hasNormals"), !o->normals.empty());
                glUniform1i(glGetUniformLocation(mainProgram, "hasTexture"), o->hasTexture);
                Texture::bind2DTexture(mainProgram, o->textureID, std::string("image"));
                //TODO: make sure the shader works with a directional light
                glUniform3fv(glGetUniformLocation(mainProgram, "lightPos"), 1, glm::value_ptr(sunPosition));
                glUniformMatrix4fv(glGetUniformLocation(mainProgram, "modelMat"), 1, GL_FALSE, glm::value_ptr(modelMat));
                glUniformMatrix4fv(glGetUniformLocation(mainProgram, "modelView"), 1, GL_FALSE, glm::value_ptr(modelView));
                glUniformMatrix4fv(glGetUniformLocation(mainProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
                glUniform1f(glGetUniformLocation(mainProgram, "zFar"), Z_FAR);

                // POINT, LINE or FILL...
                glPolygonMode(GL_FRONT_AND_BACK, o->m_polygonMode);
                glDrawElements(o->m_primitiveMode, o->drawFaces.size(), GL_UNSIGNED_INT, (void*)0);

                Texture::unbind2DTexture();
                // unbind
                glBindVertexArray(0);
            }
        }

        // reset
        glFrontFace(GL_CCW);
        glDisable(GL_CULL_FACE);

        glDisable(GL_CLIP_DISTANCE0);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        ///////////////////////////////////////////////////

        ///////////////////////////////////////////////////
        // RENDER LOCAL REFRACTIONS TO TEXTURE...
        glBindFramebuffer(GL_FRAMEBUFFER, m_localRefractionsFBO);

        glEnable(GL_CLIP_DISTANCE0);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        //NOTE: this must be our standard counter-clockwise
        glFrontFace(GL_CCW);

        // alpha of 0.0 is used to indicate no local refraction at fragment (i.e. the skybox is here and gets handled as deepest water)
        glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // render other objects...
        //TODO: currently not rendering any objects using trivial shader program (cause I don't want to change those shaders), but this is fine since only the debug planes currently are shaded this way
        //      I could create a modified trivial shader program, or switch everything to the main shader program and then just skip rendering objects flagged as DEBUG
        //TODO: optimize by batch-drawing objects that use the same shader program, as well as removing redundant uniform setting
        //TODO: design some sort of wrapper around shader programs that can dynamically set all uniforms properly

        // in column-major order
        // shrinks/shallows world-space position in the Y-axis by the refractive index ratio of air (n_1 = 1.0003) / water (n_2 = 1.33) ~= 0.75
        glm::mat4 const LOCAL_REFRACTIONS_MATRIX{1.0f, 0.0f, 0.0f, 0.0f,
                                                 0.0f, 0.75f, 0.0f, 0.0f,
                                                 0.0f, 0.0f, 1.0f, 0.0f,
                                                 0.0f, 0.0f, 0.0f, 1.0f};

        // <A, B, C, D> where Ax + By + Cz = D
        //TODO: this might be improved by accounting for amplitude
        // clipping test will succeed if underneath XZ-plane
        //TODO: see if any padding is needed to hide artifacts when grazing the surface
        glm::vec4 const LOCAL_REFRACTIONS_CLIP_PLANE{0.0f, -1.0f, 0.0f, 0.0f};

        for (std::shared_ptr<MeshObject const> o : objects) {
            assert(0 != o->shaderProgramID);

            // don't render invisible objects...
            if (!o->m_isVisible) continue;

            if (o->shaderProgramID == mainProgram) {
                glm::mat4 const modelMat{LOCAL_REFRACTIONS_MATRIX * o->getModel()};
                glm::mat4 const modelView{view * modelMat};

                // enable shader program...
                glUseProgram(mainProgram);
                // bind geometry data...
                glBindVertexArray(o->vao);

                // set uniforms...
                glUniform4fv(glGetUniformLocation(mainProgram, "clipPlane0"), 1, glm::value_ptr(LOCAL_REFRACTIONS_CLIP_PLANE));
                glUniform4fv(glGetUniformLocation(mainProgram, "fogColourFarAtCurrentTime"), 1, glm::value_ptr(fogColourFarAtCurrentTime));
                glUniform1f(glGetUniformLocation(mainProgram, "fogDepthRadiusFar"), fogDepthRadiusFar);
                glUniform1f(glGetUniformLocation(mainProgram, "fogDepthRadiusNear"), fogDepthRadiusNear);
                glUniform1i(glGetUniformLocation(mainProgram, "hasNormals"), !o->normals.empty());
                glUniform1i(glGetUniformLocation(mainProgram, "hasTexture"), o->hasTexture);
                Texture::bind2DTexture(mainProgram, o->textureID, std::string("image"));
                //TODO: make sure the shader works with a directional light
                glUniform3fv(glGetUniformLocation(mainProgram, "lightPos"), 1, glm::value_ptr(sunPosition));
                glUniformMatrix4fv(glGetUniformLocation(mainProgram, "modelMat"), 1, GL_FALSE, glm::value_ptr(modelMat));
                glUniformMatrix4fv(glGetUniformLocation(mainProgram, "modelView"), 1, GL_FALSE, glm::value_ptr(modelView));
                glUniformMatrix4fv(glGetUniformLocation(mainProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
                glUniform1f(glGetUniformLocation(mainProgram, "zFar"), Z_FAR);

                // POINT, LINE or FILL...
                glPolygonMode(GL_FRONT_AND_BACK, o->m_polygonMode);
                glDrawElements(o->m_primitiveMode, o->drawFaces.size(), GL_UNSIGNED_INT, (void*)0);

                Texture::unbind2DTexture();
                // unbind
                glBindVertexArray(0);
            }
        }

        // reset
        glDisable(GL_CULL_FACE);

        glDisable(GL_CLIP_DISTANCE0);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        ///////////////////////////////////////////////////

        // now render everything else to main screen framebuffer...
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // render the combined skybox from our main camera...
        // render combined skybox (all layers) on top of clear colour...
        // reference: https://learnopengl.com/Advanced-OpenGL/Cubemaps
        // reference: http://antongerdelan.net/opengl/cubemaps.html
        // disable depth writing to draw the skybox in the background
        glDepthMask(GL_FALSE);
        // enable trivial skybox shader program
        glUseProgram(skyboxTrivialProgram);
        // bind geometry data...
        //NOTE: I might as well use the star skybox geometry since I just need a cube
        glBindVertexArray(skyboxStars->vao);

        // set uniforms...
        //TODO: refactor into own function
        // bind texture...
        glActiveTexture(GL_TEXTURE0 + m_skyboxCubemap);
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_skyboxCubemap);
        glUniform1i(glGetUniformLocation(skyboxTrivialProgram, "skybox"), m_skyboxCubemap);
        glUniformMatrix4fv(glGetUniformLocation(skyboxTrivialProgram, "VPNoTranslation"), 1, GL_FALSE, glm::value_ptr(VPNoTranslation));

        // POINT, LINE or FILL...
        glPolygonMode(GL_FRONT_AND_BACK, skyboxStars->m_polygonMode);
        glDrawElements(skyboxStars->m_primitiveMode, skyboxStars->drawFaces.size(), GL_UNSIGNED_INT, (void*)0);

        //TODO: refactor into own function
        // unbind texture...
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
        // unbind
        glBindVertexArray(0);
        // re-enable depth writing for the rest of the scene
        glDepthMask(GL_TRUE);

        // render other objects...
        //TODO: optimize by batch-drawing objects that use the same shader program, as well as removing redundant uniform setting
        //TODO: design some sort of wrapper around shader programs that can dynamically set all uniforms properly
        for (std::shared_ptr<MeshObject const> o : objects) {
            assert(0 != o->shaderProgramID);

            // don't render invisible objects...
            if (!o->m_isVisible) continue;

            if (o->shaderProgramID == mainProgram) {
                glm::mat4 const modelMat{o->getModel()};
                glm::mat4 const modelView{view * modelMat};

                // enable shader program...
                glUseProgram(mainProgram);
                // bind geometry data...
                glBindVertexArray(o->vao);

                // set uniforms...
                // pass a symbolic clip plane singularity to ensure this manual clipping test succeeds for all vertices - avoids driver bugs that ignore enable/disable state of clip distances
                glUniform4fv(glGetUniformLocation(mainProgram, "clipPlane0"), 1, glm::value_ptr(SYMBOLIC_CLIP_PLANE_SINGULARITY));
                glUniform4fv(glGetUniformLocation(mainProgram, "fogColourFarAtCurrentTime"), 1, glm::value_ptr(fogColourFarAtCurrentTime));
                glUniform1f(glGetUniformLocation(mainProgram, "fogDepthRadiusFar"), fogDepthRadiusFar);
                glUniform1f(glGetUniformLocation(mainProgram, "fogDepthRadiusNear"), fogDepthRadiusNear);
                glUniform1i(glGetUniformLocation(mainProgram, "hasNormals"), !o->normals.empty());
                glUniform1i(glGetUniformLocation(mainProgram, "hasTexture"), o->hasTexture);
                Texture::bind2DTexture(mainProgram, o->textureID, std::string("image"));
                //TODO: make sure the shader works with a directional light
                glUniform3fv(glGetUniformLocation(mainProgram, "lightPos"), 1, glm::value_ptr(sunPosition));
                glUniformMatrix4fv(glGetUniformLocation(mainProgram, "modelMat"), 1, GL_FALSE, glm::value_ptr(modelMat));
                glUniformMatrix4fv(glGetUniformLocation(mainProgram, "modelView"), 1, GL_FALSE, glm::value_ptr(modelView));
                glUniformMatrix4fv(glGetUniformLocation(mainProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
                glUniform1f(glGetUniformLocation(mainProgram, "zFar"), Z_FAR);

                // POINT, LINE or FILL...
                glPolygonMode(GL_FRONT_AND_BACK, o->m_polygonMode);
                glDrawElements(o->m_primitiveMode, o->drawFaces.size(), GL_UNSIGNED_INT, (void*)0);

                Texture::unbind2DTexture();
                // unbind
                glBindVertexArray(0);
            } else if (o->shaderProgramID == trivialProgram) {
                glm::mat4 const mvp{projection * view * o->getModel()};

                // enable shader program...
                glUseProgram(trivialProgram);
                // bind geometry data...
                glBindVertexArray(o->vao);

                // set uniforms...
                glUniformMatrix4fv(glGetUniformLocation(trivialProgram, "mvp"), 1, GL_FALSE, glm::value_ptr(mvp));

                // POINT, LINE or FILL...
                glPolygonMode(GL_FRONT_AND_BACK, o->m_polygonMode);
                glDrawElements(o->m_primitiveMode, o->drawFaces.size(), GL_UNSIGNED_INT, (void*)0);

                // unbind
                glBindVertexArray(0);
            } else assert(false);
        }

        //NOTE: the order of drawing matters for alpha-blending
        // render water...
        if (nullptr != waterGrid && waterGrid->m_isVisible && 0 != m_skyboxCubemap) {

            // reference: https://fileadmin.cs.lth.se/graphics/theses/projects/projgrid/
            //NOTE: this code closely follows the algorithm laid out by the demo at the above reference

            // the displaceable volume is defined by the maximum possible amplitude of all the wave summations
            float const DISPLACEABLE_AMPLITUDE = geometry::GerstnerWave::TotalAmplitude() + heightmapDisplacementScale + verticalBounceWaveAmplitude;
            //TODO: figure out if the below line causes any issues (cause it seems like it would be slightly more efficient)
            //float const DISPLACEABLE_AMPLITUDE = geometry::GerstnerWave::TotalAmplitude() + heightmapDisplacementScale + glm::abs(verticalBounceWaveDisplacement);

            geometry::Plane const upperPlane{0.0f, 1.0f, 0.0f, DISPLACEABLE_AMPLITUDE};
            geometry::Plane const basePlane{0.0f, 1.0f, 0.0f, 0.0f};
            geometry::Plane const lowerPlane{0.0f, 1.0f, 0.0f, -DISPLACEABLE_AMPLITUDE};

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

            // scale XY-NDC to account for Gerstner wave XZ-world displacement...
            //TODO: dynamically set this scale based on wave settings (so that the frustum is as small as possible - reduce overdraw)
            //TODO: also scale the grid resolution so it stays roughly the same, so that it doesnt change based on these settings
            //TODO: this still needs a lot of work (i.e. account for FOV / window size ???)
            float const SAFETY_PADDING_SCALAR = 1.2f;
            for (unsigned int i = 0; i < frustumCornerPoints.size(); ++i) {
                frustumCornerPoints.at(i).x *= SAFETY_PADDING_SCALAR;
                frustumCornerPoints.at(i).y *= SAFETY_PADDING_SCALAR;
            }

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
                float const PROJECTOR_ELEVATION_FROM_CAMERA{1.0f};
                float const MINIMUM_PROJECTOR_DISTANCE_FROM_BASE_PLANE{DISPLACEABLE_AMPLITUDE + PROJECTOR_ELEVATION_FROM_CAMERA};

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
                float const FORWARD_FIXED_LENGTH{1.0f};
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
                GLuint const GRID_LENGTH = 513;

                glUniform4fv(glGetUniformLocation(waterGridProgram, "bottomLeftGridPointInWorld"), 1, glm::value_ptr(bottomLeftGridPointInWorld));
                glUniform4fv(glGetUniformLocation(waterGridProgram, "bottomRightGridPointInWorld"), 1, glm::value_ptr(bottomRightGridPointInWorld));
                glUniform3fv(glGetUniformLocation(waterGridProgram, "cameraPosition"), 1, glm::value_ptr(m_camera->getPosition()));
                glUniform4fv(glGetUniformLocation(waterGridProgram, "fogColourFarAtCurrentTime"), 1, glm::value_ptr(fogColourFarAtCurrentTime));
                glUniform1f(glGetUniformLocation(waterGridProgram, "fogDepthRadiusFar"), fogDepthRadiusFar);
                glUniform1f(glGetUniformLocation(waterGridProgram, "fogDepthRadiusNear"), fogDepthRadiusNear);

                // reference: https://developer.nvidia.com/gpugems/gpugems/part-i-natural-effects/chapter-1-effective-water-simulation-physical-models
                // reference: https://github.com/CaffeineViking/osgw/blob/master/share/shaders/gerstner.glsl
                glUniform1ui(glGetUniformLocation(waterGridProgram, "gerstnerWaveCount"), geometry::GerstnerWave::Count());
                for (unsigned int i = 0; i < gerstnerWaves.size(); ++i) {
                    std::shared_ptr<geometry::GerstnerWave const> gerstnerWave{gerstnerWaves.at(i)};
                    if (nullptr == gerstnerWave) continue;

                    //TODO: handle div by zero
                    float const steepness_Q_i{gerstnerWave->steepness_Q / (gerstnerWave->frequency_w * gerstnerWave->amplitude_A * geometry::GerstnerWave::Count())};
                    std::string const prefixStr{"gerstnerWaves[" + std::to_string(i) + "]."};

                    glUniform1f(glGetUniformLocation(waterGridProgram, std::string{prefixStr + "amplitude_A"}.c_str()), gerstnerWave->amplitude_A);
                    glUniform1f(glGetUniformLocation(waterGridProgram, std::string{prefixStr + "frequency_w"}.c_str()), gerstnerWave->frequency_w);
                    glUniform1f(glGetUniformLocation(waterGridProgram, std::string{prefixStr + "phaseConstant_phi"}.c_str()), gerstnerWave->phaseConstant_phi);
                    glUniform1f(glGetUniformLocation(waterGridProgram, std::string{prefixStr + "steepness_Q_i"}.c_str()), steepness_Q_i);
                    glUniform2fv(glGetUniformLocation(waterGridProgram, std::string{prefixStr + "xzDirection_D"}.c_str()), 1, glm::value_ptr(gerstnerWave->xzDirection_D));
                }

                glUniform1ui(glGetUniformLocation(waterGridProgram, "gridLength"), GRID_LENGTH);
                Texture::bind2DTexture(waterGridProgram, waterGrid->textureID, "heightmap");
                glUniform1f(glGetUniformLocation(waterGridProgram, "heightmapDisplacementScale"), heightmapDisplacementScale);
                glUniform1f(glGetUniformLocation(waterGridProgram, "heightmapSampleScale"), heightmapSampleScale);
                Texture::bind2DTexture(waterGridProgram, m_localReflectionsTexture2D, "localReflectionsTexture2D");
                Texture::bind2DTexture(waterGridProgram, m_localRefractionsTexture2D, "localRefractionsTexture2D");

                //TODO: refactor into own function
                // bind texture...
                glActiveTexture(GL_TEXTURE0 + m_skyboxCubemap);
                glBindTexture(GL_TEXTURE_CUBE_MAP, m_skyboxCubemap);
                glUniform1i(glGetUniformLocation(waterGridProgram, "skybox"), m_skyboxCubemap);

                glUniform3fv(glGetUniformLocation(waterGridProgram, "sunPosition"), 1, glm::value_ptr(sunPosition));
                glUniform1f(glGetUniformLocation(waterGridProgram, "sunShininess"), sunShininess);
                glUniform1f(glGetUniformLocation(waterGridProgram, "sunStrength"), sunStrength);
                glUniform4fv(glGetUniformLocation(waterGridProgram, "topLeftGridPointInWorld"), 1, glm::value_ptr(topLeftGridPointInWorld));
                glUniform4fv(glGetUniformLocation(waterGridProgram, "topRightGridPointInWorld"), 1, glm::value_ptr(topRightGridPointInWorld));
                glUniform1f(glGetUniformLocation(waterGridProgram, "verticalBounceWaveDisplacement"), verticalBounceWaveDisplacement);
                glUniformMatrix4fv(glGetUniformLocation(waterGridProgram, "viewMatOnlyYaw"), 1, GL_FALSE, glm::value_ptr(viewMatOnlyYaw));
                glUniform2fv(glGetUniformLocation(waterGridProgram, "viewportWidthHeight"), 1, glm::value_ptr(glm::vec2{(float)m_windowWidth, (float)m_windowHeight}));
                glUniformMatrix4fv(glGetUniformLocation(waterGridProgram, "viewProjection"), 1, GL_FALSE, glm::value_ptr(viewProjection));
                glUniform1f(glGetUniformLocation(waterGridProgram, "waveAnimationTimeInSeconds"), waveAnimationTimeInSeconds);
                glUniform1f(glGetUniformLocation(waterGridProgram, "zFar"), Z_FAR);

                // draw...
                // POINT, LINE or FILL...
                glPolygonMode(GL_FRONT_AND_BACK, waterGrid->m_polygonMode);
                glDrawElements(waterGrid->m_primitiveMode, waterGrid->drawFaces.size(), GL_UNSIGNED_INT, (void*)0);

                // unbind texture...
                glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

                Texture::unbind2DTexture();
                glBindVertexArray(0); // unbind VAO
                glUseProgram(0); // unbind shader program
            }
        }
    }

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

    // Sets projection and viewport for new width and height
    void RenderEngine::setWindowSize(int width, int height) {
        m_windowWidth = width;
        m_windowHeight = height;
        m_camera->setAspect((float)m_windowWidth / m_windowHeight);
        glViewport(0, 0, m_windowWidth, m_windowHeight);

        //TODO: figure out if there are any driver bugs that require regenerating the FBO or rebinding the textures/buffers to it
        // reference: https://stackoverflow.com/questions/44763449/updating-width-and-height-of-render-target-on-the-fly
        // reallocate textures / buffers that must match new window dimensions...
        glBindRenderbuffer(GL_RENDERBUFFER, m_depth24Stencil8RBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_windowWidth, m_windowHeight);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);

        glBindTexture(GL_TEXTURE_2D, m_localReflectionsTexture2D);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_windowWidth, m_windowHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glBindTexture(GL_TEXTURE_2D, 0);

        glBindTexture(GL_TEXTURE_2D, m_localRefractionsTexture2D);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_windowWidth, m_windowHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}
