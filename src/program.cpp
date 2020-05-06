/*
 * Sample boilerplate taken from CPSC453 at the University of Calgary
 * Main class for user-defined program
 * Created on: Sep 10, 2018
 * Author: John Hall
 * Modifications by: Aaron Hornby (10176084)
 */

#include "program.h"

#include <fstream>
#include <iostream>
#include <string>

#include <imgui/imgui.h>
#include <imgui/examples/imgui_impl_glfw.h>
#include <imgui/examples/imgui_impl_opengl3.h>

// remember: include glad before GLFW
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "image-buffer.h"
#include "input-handler.h"
#include "mesh-object.h"
#include "object-loader.h"
#include "render-engine.h"

namespace wave_tool {
    Program::Program() {}

    Program::~Program() {}

    std::shared_ptr<RenderEngine> Program::getRenderEngine() const {
        return m_renderEngine;
    }

    bool Program::start() {
        if (!setupWindow()) return false;

        m_renderEngine = std::make_shared<RenderEngine>(m_window);

        initScene();

        //image.Initialize();
        //do a bunch of raytracing into texture
        //image.SaveToFile("image.png"); // no need to put in loop since we dont update image

        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        // render loop
        while (!glfwWindowShouldClose(m_window)) {
            // handle inputs
            glfwPollEvents();

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            buildUI();

            // rendering...
            ImGui::Render();
            //image.Render();
            m_renderEngine->render(m_skyboxStars, m_skysphere, m_skyboxClouds, m_waterGrid, m_meshObjects);

            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            glfwSwapBuffers(m_window);
        }

        return cleanup();
    }

    //TODO: look at Dear ImGui demo code and expand this to be better organized
    void Program::buildUI() {
        // start Dear ImGui frame...
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
        ImGui::SetNextWindowSizeConstraints(ImVec2(1024.0f, 64.0f), ImVec2(1024.0f, 512.0f));
        //ImGui::SetWindowSize(ImVec2(1024.0f, 256.0f));
        // begin main window...
        //NOTE: don't do this with my current setup, cause the early return will prevent all animations from updating (so effectively this is a cheap but unwanted pause button!)
        //TODO: move animation updates into a better location, add a proper pause button and then add this back in
        //if (!ImGui::Begin("SETTINGS")) {
        //    // early out (optimization) if the window is collapsed
        //    ImGui::End();
        //    return;
        //}
        ImGui::Begin("SETTINGS");

        ImGui::Separator();

        // reference: https://github.com/ocornut/imgui/blob/cc0d4e346a3e4a5408c85c7e6bf0df5e1307bb2d/examples/example_marmalade/main.cpp#L93
        ImGui::Text("AVG. FRAMETIME (VSYNC ON) - %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

        ImGui::Separator();

        if (ImGui::Button("EXPORT IMAGE - SAVE AS")) {
            exportFrontBufferToImageFile(std::string{m_imageSaveAsName} +".png");
        }
        ImGui::SameLine();
        ImGui::InputText(".png", m_imageSaveAsName, IM_ARRAYSIZE(m_imageSaveAsName));

        ImGui::Separator();

        if (nullptr != m_yzPlane) {
            if (ImGui::Button("TOGGLE YZ-PLANE / +X-AXIS (RED)")) m_yzPlane->m_isVisible = !m_yzPlane->m_isVisible;
            ImGui::SameLine();
        }
        if (nullptr != m_xzPlane) {
            if (ImGui::Button("TOGGLE XZ-PLANE / +Y-AXIS (GREEN)")) m_xzPlane->m_isVisible = !m_xzPlane->m_isVisible;
            ImGui::SameLine();
        }
        if (nullptr != m_xyPlane) {
            if (ImGui::Button("TOGGLE XY-PLANE / +Z-AXIS (BLUE)")) m_xyPlane->m_isVisible = !m_xyPlane->m_isVisible;
            ImGui::SameLine();
        }
        if (nullptr != m_yzPlane || nullptr != m_xzPlane || nullptr != m_xyPlane) {
            ImGui::Text("   note: grid spacing is 1 unit");
            ImGui::Separator();
        }

        ImGui::Separator();

        ImGui::Text("RENDER MODE:");
        ImGui::SameLine();
        if (ImGui::Button("DEFAULT##0")) m_renderEngine->renderMode = RenderMode::DEFAULT;
        ImGui::SameLine();
        if (ImGui::Button("LOCAL REFLECTIONS##0")) m_renderEngine->renderMode = RenderMode::LOCAL_REFLECTIONS;
        ImGui::SameLine();
        if (ImGui::Button("LOCAL REFRACTIONS##0")) m_renderEngine->renderMode = RenderMode::LOCAL_REFRACTIONS;

        ImGui::Separator();

        if (ImGui::SliderFloat("TIME OF DAY (HOURS)", &m_renderEngine->timeOfDayInHours, 0.0f, 24.0f)) {
            // force-clamp (handle CTRL + LEFT_CLICK)
            m_renderEngine->timeOfDayInHours = glm::clamp(m_renderEngine->timeOfDayInHours, 0.0f, 24.0f);
        }

        if (ImGui::Button("TOGGLE ANIMATION##0")) {
            m_renderEngine->isAnimatingTimeOfDay = !m_renderEngine->isAnimatingTimeOfDay;
        }
        ImGui::SameLine();

        ImGui::PushItemWidth(300.0f);
        if (ImGui::SliderFloat("ANIMATION SPEED (REAL-TIME SECONDS PER SIMULATED HOUR)", &m_renderEngine->animationSpeedTimeOfDayInSecondsPerHour, 0.0f, 60.0f)) {
            // force-clamp (handle CTRL + LEFT_CLICK)
            if (m_renderEngine->animationSpeedTimeOfDayInSecondsPerHour < 0.0f) m_renderEngine->animationSpeedTimeOfDayInSecondsPerHour = 0.0f;
        }
        ImGui::PopItemWidth();

        //TODO: refactor this into its own function somewhere else...
        //TODO: check if this ImGui framerate is applicable here (or is it an average of several frames???)
        if (m_renderEngine->isAnimatingTimeOfDay && m_renderEngine->animationSpeedTimeOfDayInSecondsPerHour > 0.0f) {
            float const deltaTimeInSeconds = 1.0f / ImGui::GetIO().Framerate;
            float const deltaTimeOfDayInHours = (1.0f / m_renderEngine->animationSpeedTimeOfDayInSecondsPerHour) * deltaTimeInSeconds;
            m_renderEngine->timeOfDayInHours += deltaTimeOfDayInHours;
            m_renderEngine->timeOfDayInHours = glm::mod(m_renderEngine->timeOfDayInHours, 24.0f);
        }

        if (ImGui::SliderFloat("CLOUD PROPORTION", &m_renderEngine->cloudProportion, 0.0f, 1.0f)) {
            // force-clamp (handle CTRL + LEFT_CLICK)
            m_renderEngine->cloudProportion = glm::clamp(m_renderEngine->cloudProportion, 0.0f, 1.0f);
        }

        if (ImGui::SliderFloat("OVERCAST STRENGTH", &m_renderEngine->overcastStrength, 0.0f, 1.0f)) {
            // force-clamp (handle CTRL + LEFT_CLICK)
            m_renderEngine->overcastStrength = glm::clamp(m_renderEngine->overcastStrength, 0.0f, 1.0f);
        }

        if (ImGui::SliderFloat("SUN-HORIZON DARKNESS", &m_renderEngine->sunHorizonDarkness, 0.0f, 1.0f)) {
            // force-clamp (handle CTRL + LEFT_CLICK)
            m_renderEngine->sunHorizonDarkness = glm::clamp(m_renderEngine->sunHorizonDarkness, 0.0f, 1.0f);
        }

        if (ImGui::SliderFloat("SUN SHININESS", &m_renderEngine->sunShininess, 0.0f, 200.0f)) {
            // force-clamp (handle CTRL + LEFT_CLICK)
            if (m_renderEngine->sunShininess < 0.0f) m_renderEngine->sunShininess = 0.0f;
        }

        if (ImGui::SliderFloat("SUN STRENGTH", &m_renderEngine->sunStrength, 0.0f, 1.0f)) {
            // force-clamp (handle CTRL + LEFT_CLICK)
            m_renderEngine->sunStrength = glm::clamp(m_renderEngine->sunStrength, 0.0f, 1.0f);
        }

        ImGui::Separator();

        if (ImGui::ColorEdit4("FOG COLOUR (RGB - tint. A - density)", (float*)&m_renderEngine->fogColourFarAtNoon, ImGuiColorEditFlags_Float)) {
            // force-clamp (handle alternate inputs)
            m_renderEngine->fogColourFarAtNoon = glm::clamp(m_renderEngine->fogColourFarAtNoon, glm::vec4{0.0f, 0.0f, 0.0f, 0.0f}, glm::vec4{1.0f, 1.0f, 1.0f, 1.0f});
        }

        if (ImGui::SliderFloat("FOG DEPTH RADIUS (far)", &m_renderEngine->fogDepthRadiusFar, 0.0f, 1.0f)) {
            // force-clamp (handle CTRL + LEFT_CLICK)
            m_renderEngine->fogDepthRadiusFar = glm::clamp(m_renderEngine->fogDepthRadiusFar, 0.0f, 1.0f);
            // shift the other bound if needed
            m_renderEngine->fogDepthRadiusNear = glm::clamp(m_renderEngine->fogDepthRadiusNear, 0.0f, m_renderEngine->fogDepthRadiusFar);
        }

        if (ImGui::SliderFloat("FOG DEPTH RADIUS (near)", &m_renderEngine->fogDepthRadiusNear, 0.0f, 1.0f)) {
            // force-clamp (handle CTRL + LEFT_CLICK)
            m_renderEngine->fogDepthRadiusNear = glm::clamp(m_renderEngine->fogDepthRadiusNear, 0.0f, 1.0f);
            // shift the other bound if needed
            m_renderEngine->fogDepthRadiusFar = glm::clamp(m_renderEngine->fogDepthRadiusFar, m_renderEngine->fogDepthRadiusNear, 1.0f);
        }

        ImGui::Separator();

        if (nullptr != m_skyboxStars || nullptr != m_skysphere || nullptr != m_skyboxClouds) {
            if (ImGui::TreeNode("SKYBOX")) {
                ImGui::Separator();
                if (nullptr != m_skyboxStars) {
                    if (ImGui::TreeNode("SPACE/STAR LAYER")) {
                        ImGui::Separator();
                        ImGui::Text("VISIBILITY:");
                        ImGui::SameLine();
                        if (ImGui::Button("TOGGLE##0")) m_skyboxStars->m_isVisible = !m_skyboxStars->m_isVisible;
                        ImGui::SameLine();
                        ImGui::Text("POLYGON MODE:");
                        ImGui::SameLine();
                        if (ImGui::Button("FULL##0")) m_skyboxStars->m_polygonMode = PolygonMode::FILL;
                        ImGui::SameLine();
                        if (ImGui::Button("WIREFRAME##0")) m_skyboxStars->m_polygonMode = PolygonMode::LINE;
                        ImGui::Separator();
                        ImGui::TreePop();
                    }
                }
                if (nullptr != m_skysphere) {
                    if (ImGui::TreeNode("ATMOSPHERE/SUN LAYER")) {
                        ImGui::Separator();
                        ImGui::Text("VISIBILITY:");
                        ImGui::SameLine();
                        if (ImGui::Button("TOGGLE##1")) m_skysphere->m_isVisible = !m_skysphere->m_isVisible;
                        ImGui::SameLine();
                        ImGui::Text("POLYGON MODE:");
                        ImGui::SameLine();
                        if (ImGui::Button("FULL##1")) m_skysphere->m_polygonMode = PolygonMode::FILL;
                        ImGui::SameLine();
                        if (ImGui::Button("WIREFRAME##1")) m_skysphere->m_polygonMode = PolygonMode::LINE;
                        ImGui::Separator();
                        ImGui::TreePop();
                    }
                }
                if (nullptr != m_skyboxClouds) {
                    if (ImGui::TreeNode("CLOUD LAYER")) {
                        ImGui::Separator();
                        ImGui::Text("VISIBILITY:");
                        ImGui::SameLine();
                        if (ImGui::Button("TOGGLE##2")) m_skyboxClouds->m_isVisible = !m_skyboxClouds->m_isVisible;
                        ImGui::SameLine();
                        ImGui::Text("POLYGON MODE:");
                        ImGui::SameLine();
                        if (ImGui::Button("FULL##2")) m_skyboxClouds->m_polygonMode = PolygonMode::FILL;
                        ImGui::SameLine();
                        if (ImGui::Button("WIREFRAME##2")) m_skyboxClouds->m_polygonMode = PolygonMode::LINE;
                        ImGui::Separator();
                        ImGui::TreePop();
                    }
                }
                ImGui::Separator();
                ImGui::TreePop();
            }
            ImGui::Separator();
        }

        if (nullptr != m_terrain) {
            if (ImGui::TreeNode("TERRAIN")) {
                ImGui::Separator();
                ImGui::Text("VISIBILITY:");
                ImGui::SameLine();
                if (ImGui::Button("TOGGLE##3")) m_terrain->m_isVisible = !m_terrain->m_isVisible;
                ImGui::SameLine();
                ImGui::Text("POLYGON MODE:");
                ImGui::SameLine();
                if (ImGui::Button("FULL##3")) m_terrain->m_polygonMode = PolygonMode::FILL;
                ImGui::SameLine();
                if (ImGui::Button("WIREFRAME##3")) m_terrain->m_polygonMode = PolygonMode::LINE;
                ImGui::Separator();
                ImGui::TreePop();
            }
            ImGui::Separator();
        }

        if (nullptr != m_waterGrid) {
            if (ImGui::TreeNode("WATER-GRID")) {
                ImGui::Separator();
                ImGui::Text("VISIBILITY:");
                ImGui::SameLine();
                if (ImGui::Button("TOGGLE##4")) m_waterGrid->m_isVisible = !m_waterGrid->m_isVisible;
                ImGui::SameLine();
                ImGui::Text("POLYGON MODE:");
                ImGui::SameLine();
                if (ImGui::Button("FULL##4")) m_waterGrid->m_polygonMode = PolygonMode::FILL;
                ImGui::SameLine();
                if (ImGui::Button("WIREFRAME##4")) m_waterGrid->m_polygonMode = PolygonMode::LINE;
                ImGui::Separator();
                ImGui::TreePop();
            }
            ImGui::Separator();
        }

        if (!m_renderEngine->gerstnerWaves.empty()) {
            if (ImGui::TreeNode("GERSTNER WAVES")) {
                ImGui::Separator();
                for (unsigned int i = 0; i < m_renderEngine->gerstnerWaves.size(); ++i) {
                    if (nullptr == m_renderEngine->gerstnerWaves.at(i)) continue;

                    if (ImGui::TreeNode(std::string{"wave" + std::to_string(i)}.c_str())) {
                        ImGui::Separator();
                        if (ImGui::SliderFloat(std::string{"Amplitude##" + std::to_string(i)}.c_str(), &m_renderEngine->gerstnerWaves.at(i)->amplitude_A, 0.0f, 1.0f)) {
                            // force-clamp (handle CTRL + LEFT_CLICK)
                            if (m_renderEngine->gerstnerWaves.at(i)->amplitude_A < 0.0f) m_renderEngine->gerstnerWaves.at(i)->amplitude_A = 0.0f;
                        }
                        if (ImGui::SliderFloat(std::string{"Frequency##" + std::to_string(i)}.c_str(), &m_renderEngine->gerstnerWaves.at(i)->frequency_w, 0.0f, 1.0f)) {
                            // force-clamp (handle CTRL + LEFT_CLICK)
                            if (m_renderEngine->gerstnerWaves.at(i)->frequency_w < 0.0f) m_renderEngine->gerstnerWaves.at(i)->frequency_w = 0.0f;
                        }
                        if (ImGui::SliderFloat(std::string{"Phase Constant (~Speed)##" + std::to_string(i)}.c_str(), &m_renderEngine->gerstnerWaves.at(i)->phaseConstant_phi, 0.0f, 10.0f)) {
                            // force-clamp (handle CTRL + LEFT_CLICK)
                            if (m_renderEngine->gerstnerWaves.at(i)->phaseConstant_phi < 0.0f) m_renderEngine->gerstnerWaves.at(i)->phaseConstant_phi = 0.0f;
                        }
                        if (ImGui::SliderFloat(std::string{"Steepness##" + std::to_string(i)}.c_str(), &m_renderEngine->gerstnerWaves.at(i)->steepness_Q, 0.0f, 1.0f)) {
                            // force-clamp (handle CTRL + LEFT_CLICK)
                            m_renderEngine->gerstnerWaves.at(i)->steepness_Q = glm::clamp(m_renderEngine->gerstnerWaves.at(i)->steepness_Q, 0.0f, 1.0f);
                        }
                        if (ImGui::SliderFloat2(std::string{"XZ-Direction##" + std::to_string(i)}.c_str(), (float*)&m_renderEngine->gerstnerWaves.at(i)->xzDirection_D, 0.0f, 1.0f)) {
                            // force-clamp (handle CTRL + LEFT_CLICK)
                            m_renderEngine->gerstnerWaves.at(i)->xzDirection_D = glm::clamp(m_renderEngine->gerstnerWaves.at(i)->xzDirection_D, glm::vec2{0.0f, 0.0f}, glm::vec2{1.0f, 1.0f});
                            //TODO: use an epsilon???
                            // we can't normalize the 0 vector, so reset to a dummy
                            if (0.0f == glm::length(m_renderEngine->gerstnerWaves.at(i)->xzDirection_D)) m_renderEngine->gerstnerWaves.at(i)->xzDirection_D = glm::vec2{0.0f, 1.0f};
                            else m_renderEngine->gerstnerWaves.at(i)->xzDirection_D = glm::normalize(m_renderEngine->gerstnerWaves.at(i)->xzDirection_D);
                        }
                        ImGui::Separator();
                        ImGui::TreePop();
                    }
                }
                ImGui::Separator();
                ImGui::TreePop();
            }
            ImGui::Separator();
        }

        if (ImGui::SliderFloat("WATER BUMP ROUGHNESS", &m_renderEngine->heightmapSampleScale, 0.0f, 1.0f)) {
            // force-clamp (handle CTRL + LEFT_CLICK)
            if (m_renderEngine->heightmapSampleScale < 0.0f) m_renderEngine->heightmapSampleScale = 0.0f;
        }

        if (ImGui::SliderFloat("WATER BUMP STEEPNESS", &m_renderEngine->heightmapDisplacementScale, 0.0f, 1.0f)) {
            // force-clamp (handle CTRL + LEFT_CLICK)
            if (m_renderEngine->heightmapDisplacementScale < 0.0f) m_renderEngine->heightmapDisplacementScale = 0.0f;
        }

        if (ImGui::SliderFloat("VERTICAL-BOUNCE-WAVE AMPLITUDE", &m_renderEngine->verticalBounceWaveAmplitude, 0.0f, 1.0f)) {
            // force-clamp (handle CTRL + LEFT_CLICK)
            if (m_renderEngine->verticalBounceWaveAmplitude < 0.0f) m_renderEngine->verticalBounceWaveAmplitude = 0.0f;
        }

        if (ImGui::SliderFloat("VERTICAL-BOUNCE-WAVE PHASE", &m_renderEngine->verticalBounceWavePhase, 0.0f, 1.0f)) {
            // force-clamp (handle CTRL + LEFT_CLICK)
            m_renderEngine->verticalBounceWavePhase = glm::clamp(m_renderEngine->verticalBounceWavePhase, 0.0f, 1.0f);
        }

        //TODO: once i figure out why the Gerstner waves decay over time, change the max here to an appropriate value (and maybe clamp or mod???)
        //      maybe there is a looping here that I can take advantage of
        //if (ImGui::SliderFloat("WAVE-ANIMATION TIME (s)", &m_renderEngine->waveAnimationTimeInSeconds, 0.0f, std::numeric_limits<float>::max())) {
        if (ImGui::SliderFloat("WAVE-ANIMATION TIME (s)", &m_renderEngine->waveAnimationTimeInSeconds, 0.0f, 3600.0f)) {
            // force-clamp (handle CTRL + LEFT_CLICK)
            if (m_renderEngine->waveAnimationTimeInSeconds < 0.0f) m_renderEngine->waveAnimationTimeInSeconds = 0.0f;
        }

        if (ImGui::Button("TOGGLE ANIMATION##1")) {
            m_renderEngine->isAnimatingWaves = !m_renderEngine->isAnimatingWaves;
        }
        ImGui::SameLine();

        ImGui::PushItemWidth(300.0f);
        if (ImGui::SliderFloat("ANIMATION SPEED (VERTICAL-BOUNCE-WAVE PERIOD (s))", &m_renderEngine->animationSpeedVerticalBounceWavePhasePeriodInSeconds, 0.0f, 60.0f)) {
            // force-clamp (handle CTRL + LEFT_CLICK)
            if (m_renderEngine->animationSpeedVerticalBounceWavePhasePeriodInSeconds < 0.0f) m_renderEngine->animationSpeedVerticalBounceWavePhasePeriodInSeconds = 0.0f;
        }
        ImGui::PopItemWidth();

        //TODO: refactor this into its own function somewhere else...
        //TODO: check if this ImGui framerate is applicable here (or is it an average of several frames???)
        if (m_renderEngine->isAnimatingWaves) {
            float const deltaTimeInSeconds = 1.0f / ImGui::GetIO().Framerate;

            m_renderEngine->waveAnimationTimeInSeconds += deltaTimeInSeconds;
            // handle overflow...
            if (m_renderEngine->waveAnimationTimeInSeconds < 0.0f) m_renderEngine->waveAnimationTimeInSeconds = 0.0f;

            if (m_renderEngine->animationSpeedVerticalBounceWavePhasePeriodInSeconds > 0.0f) {
                float const deltaVerticalBounceWavePhase = (1.0f / m_renderEngine->animationSpeedVerticalBounceWavePhasePeriodInSeconds) * deltaTimeInSeconds;
                m_renderEngine->verticalBounceWavePhase += deltaVerticalBounceWavePhase;
                m_renderEngine->verticalBounceWavePhase = glm::mod(m_renderEngine->verticalBounceWavePhase, 1.0f);
            }
        }

        if (ImGui::SliderFloat("TINT DEPTH THRESHOLD", &m_renderEngine->tintDeltaDepthThreshold, 0.0f, 1.0f)) {
            // force-clamp (handle CTRL + LEFT_CLICK)
            m_renderEngine->tintDeltaDepthThreshold = glm::clamp(m_renderEngine->tintDeltaDepthThreshold, 0.0f, 1.0f);
        }

        if (ImGui::SliderFloat("WATER CLARITY", &m_renderEngine->waterClarity, 0.0f, 1.0f)) {
            // force-clamp (handle CTRL + LEFT_CLICK)
            m_renderEngine->waterClarity = glm::clamp(m_renderEngine->waterClarity, 0.0f, 1.0f);
        }

        if (ImGui::SliderFloat("SOFT-EDGES DEPTH THRESHOLD", &m_renderEngine->softEdgesDeltaDepthThreshold, 0.0f, 1.0f)) {
            // force-clamp (handle CTRL + LEFT_CLICK)
            m_renderEngine->softEdgesDeltaDepthThreshold = glm::clamp(m_renderEngine->softEdgesDeltaDepthThreshold, 0.0f, 1.0f);
        }

        ImGui::Separator();

        ImGui::End();

        ImGui::EndFrame();
    }

    bool Program::cleanup() {
        // Dear ImGui cleanup...
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        // glfw cleanup...
        if (nullptr != m_window) {
            glfwDestroyWindow(m_window);
            m_window = nullptr;
        }
        glfwTerminate();
        return true;
    }

    // precondition: OpenGL context was properly initialized
    // precondition: currently set viewport resolution matches window resolution
    void Program::exportFrontBufferToImageFile(std::string const& filePath) {
        ImageBuffer image;
        if (!image.Initialize()) return;

        image.readFromFrontBuffer();
        image.SaveToFile(filePath);
    }

    //NOTE: this method should only be called ONCE at start
    void Program::initScene() {
        // CREATE THE 3 PLANES...

        // draw a symmetrical grid for each cartesian plane...

        //TODO: should probably change this to read from variable in render engine that could store the far clip-plane distance
        //NOTE: compare this to far clipping plane distance of 100
        //NOTE: all these should be the same
        int const maxX{100};
        int const maxY{100};
        int const maxZ{100};
        //NOTE: any change here should be reflected in the ImGui notice
        int const deltaX{1};
        int const deltaY{1};
        int const deltaZ{1};

        // YZ-PLANE / +X-AXIS (RED)...
        m_yzPlane = std::make_shared<MeshObject>();
        m_yzPlane->setTag(Tag::DEBUG);

        for (int y = -maxY; y <= maxY; y += deltaY) {
            m_yzPlane->drawVerts.push_back(glm::vec3(0.0f, y, -maxZ));
            m_yzPlane->drawVerts.push_back(glm::vec3(0.0f, y, maxZ));
        }
        for (int z = -maxZ; z <= maxZ; z += deltaZ) {
            m_yzPlane->drawVerts.push_back(glm::vec3(0.0f, -maxY, z));
            m_yzPlane->drawVerts.push_back(glm::vec3(0.0f, maxY, z));
        }
        m_yzPlane->drawVerts.push_back(glm::vec3{0.0f, 0.0f, 0.0f});
        m_yzPlane->drawVerts.push_back(glm::vec3{2.0f * maxX, 0.0f, 0.0f});

        for (unsigned int i = 0; i < m_yzPlane->drawVerts.size(); ++i) {
            m_yzPlane->drawFaces.push_back(i);
            m_yzPlane->colours.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
        }

        m_yzPlane->m_primitiveMode = PrimitiveMode::LINES;
        m_yzPlane->m_isVisible = false;
        m_yzPlane->shaderProgramID = m_renderEngine->getTrivialProgram();
        m_meshObjects.push_back(m_yzPlane);
        m_renderEngine->assignBuffers(*m_yzPlane);

        // XZ-PLANE / +Y-AXIS (GREEN)...

        m_xzPlane = std::make_shared<MeshObject>();
        m_xzPlane->setTag(Tag::DEBUG);

        for (int x = -maxX; x <= maxX; x += deltaX) {
            m_xzPlane->drawVerts.push_back(glm::vec3(x, 0.0f, -maxZ));
            m_xzPlane->drawVerts.push_back(glm::vec3(x, 0.0f, maxZ));
        }
        for (int z = -maxZ; z <= maxZ; z += deltaZ) {
            m_xzPlane->drawVerts.push_back(glm::vec3(-maxX, 0.0f, z));
            m_xzPlane->drawVerts.push_back(glm::vec3(maxX, 0.0f, z));
        }
        m_xzPlane->drawVerts.push_back(glm::vec3{0.0f, 0.0f, 0.0f});
        m_xzPlane->drawVerts.push_back(glm::vec3{0.0f, 2.0f * maxY, 0.0f});

        for (unsigned int i = 0; i < m_xzPlane->drawVerts.size(); ++i) {
            m_xzPlane->drawFaces.push_back(i);
            m_xzPlane->colours.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
        }

        m_xzPlane->m_primitiveMode = PrimitiveMode::LINES;
        m_xzPlane->m_isVisible = false;
        m_xzPlane->shaderProgramID = m_renderEngine->getTrivialProgram();
        m_meshObjects.push_back(m_xzPlane);
        m_renderEngine->assignBuffers(*m_xzPlane);

        // XY-PLANE / +Z-AXIS (BLUE)

        m_xyPlane = std::make_shared<MeshObject>();
        m_xyPlane->setTag(Tag::DEBUG);

        for (int x = -maxX; x <= maxX; x += deltaX) {
            m_xyPlane->drawVerts.push_back(glm::vec3(x, -maxY, 0.0f));
            m_xyPlane->drawVerts.push_back(glm::vec3(x, maxY, 0.0f));
        }
        for (int y = -maxY; y <= maxY; y += deltaY) {
            m_xyPlane->drawVerts.push_back(glm::vec3(-maxX, y, 0.0f));
            m_xyPlane->drawVerts.push_back(glm::vec3(maxX, y, 0.0f));
        }
        m_xyPlane->drawVerts.push_back(glm::vec3{0.0f, 0.0f, 0.0f});
        m_xyPlane->drawVerts.push_back(glm::vec3{0.0f, 0.0f, 2.0f * maxZ});

        for (unsigned int i = 0; i < m_xyPlane->drawVerts.size(); ++i) {
            m_xyPlane->drawFaces.push_back(i);
            m_xyPlane->colours.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
        }

        m_xyPlane->m_primitiveMode = PrimitiveMode::LINES;
        m_xyPlane->m_isVisible = false;
        m_xyPlane->shaderProgramID = m_renderEngine->getTrivialProgram();
        m_meshObjects.push_back(m_xyPlane);
        m_renderEngine->assignBuffers(*m_xyPlane);

        //TODO: is it possible to mirror the skybox textures on loading them in (since we are inside the cube), but keeping the proper orientation???
        // hard-coded skyboxes...

        // this will hold the skybox geometry (cube) and star skybox cubemap
        m_skyboxStars = ObjectLoader::createTriMeshObject("../../assets/models/imports/cube.obj", true, true);
        if (nullptr != m_skyboxStars) {
            m_skyboxStars->textureID = m_renderEngine->loadCubemap({"../../assets/textures/skyboxes/wwwtyro-space-3d/2drp4i9sx0lc-stars-2048/right.png",
                                                                    "../../assets/textures/skyboxes/wwwtyro-space-3d/2drp4i9sx0lc-stars-2048/left.png",
                                                                    "../../assets/textures/skyboxes/wwwtyro-space-3d/2drp4i9sx0lc-stars-2048/top.png",
                                                                    "../../assets/textures/skyboxes/wwwtyro-space-3d/2drp4i9sx0lc-stars-2048/bottom.png",
                                                                    "../../assets/textures/skyboxes/wwwtyro-space-3d/2drp4i9sx0lc-stars-2048/front.png",
                                                                    "../../assets/textures/skyboxes/wwwtyro-space-3d/2drp4i9sx0lc-stars-2048/back.png"});

            // fallback #1 (use debug skybox)
            if (0 == m_skyboxStars->textureID) {
                m_skyboxStars->textureID = m_renderEngine->loadCubemap({"../../assets/textures/skyboxes/debug/_px.jpg",
                                                                        "../../assets/textures/skyboxes/debug/_nx.jpg",
                                                                        "../../assets/textures/skyboxes/debug/_py.jpg",
                                                                        "../../assets/textures/skyboxes/debug/_ny.jpg",
                                                                        "../../assets/textures/skyboxes/debug/_pz.jpg",
                                                                        "../../assets/textures/skyboxes/debug/_nz.jpg"});

                // fallback #2 (no star skybox)
                if (0 == m_skyboxStars->textureID) m_skyboxStars = nullptr;
            }
            if (nullptr != m_skyboxStars) {
                m_skyboxStars->shaderProgramID = m_renderEngine->getSkyboxStarsProgram();
                m_renderEngine->assignBuffers(*m_skyboxStars);
            }
        }

        // skysphere...
        m_skysphere = ObjectLoader::createTriMeshObject("../../assets/models/imports/icosphere.obj", true, true);
        if (nullptr != m_skysphere) {
            m_skysphere->textureID = m_renderEngine->load1DTexture("../../assets/textures/sky-gradient.png");
            // fallback #1 (no skysphere)
            if (0 == m_skysphere->textureID) m_skysphere = nullptr;
            if (nullptr != m_skysphere) {
                m_skysphere->shaderProgramID = m_renderEngine->getSkysphereProgram();
                m_renderEngine->assignBuffers(*m_skysphere);
            }
        }

        // this will hold the skybox geometry (cube) and cloud skybox cubemap
        m_skyboxClouds = ObjectLoader::createTriMeshObject("../../assets/models/imports/cube.obj", true, true);
        if (nullptr != m_skyboxClouds) {
            m_skyboxClouds->textureID = m_renderEngine->loadCubemap({"../../assets/textures/skyboxes/wwwtyro-space-3d/2drp4i9sx0lc-nebulae-2048/right.png",
                                                                     "../../assets/textures/skyboxes/wwwtyro-space-3d/2drp4i9sx0lc-nebulae-2048/left.png",
                                                                     "../../assets/textures/skyboxes/wwwtyro-space-3d/2drp4i9sx0lc-nebulae-2048/top.png",
                                                                     "../../assets/textures/skyboxes/wwwtyro-space-3d/2drp4i9sx0lc-nebulae-2048/bottom.png",
                                                                     "../../assets/textures/skyboxes/wwwtyro-space-3d/2drp4i9sx0lc-nebulae-2048/front.png",
                                                                     "../../assets/textures/skyboxes/wwwtyro-space-3d/2drp4i9sx0lc-nebulae-2048/back.png"});

            // fallback #1 (use debug skybox)
            if (0 == m_skyboxClouds->textureID) {
                m_skyboxClouds->textureID = m_renderEngine->loadCubemap({"../../assets/textures/skyboxes/debug/_px.jpg",
                                                                         "../../assets/textures/skyboxes/debug/_nx.jpg",
                                                                         "../../assets/textures/skyboxes/debug/_py.jpg",
                                                                         "../../assets/textures/skyboxes/debug/_ny.jpg",
                                                                         "../../assets/textures/skyboxes/debug/_pz.jpg",
                                                                         "../../assets/textures/skyboxes/debug/_nz.jpg"});

                // fallback #2 (no cloud skybox)
                if (0 == m_skyboxClouds->textureID) m_skyboxClouds = nullptr;
            }
            if (nullptr != m_skyboxClouds) {
                m_skyboxClouds->shaderProgramID = m_renderEngine->getSkyboxCloudsProgram();
                m_renderEngine->assignBuffers(*m_skyboxClouds);
            }
        }

        m_waterGrid = std::make_shared<MeshObject>();
        //m_waterGrid->m_polygonMode = PolygonMode::POINT; //NOTE: doing this atm makes a cool pixel art world
        //TEMP: hacking some indices together to draw grid as tri-mesh (should move this to MeshObject in the future)...
        //TODO: explain this better in the future (with diagrams)

        // first, store indices into a length * length square grid
        //NOTE: atm, this must match the constant of the same name in render-engine.cpp, but will be changed in the future
        GLuint const GRID_LENGTH = 513;
        //NOTE: must use vector instead of array to handle larger grid lengths
        // zero-fill
        std::vector<std::vector<GLuint>> gridIndices(GRID_LENGTH, std::vector<GLuint>(GRID_LENGTH, 0));
        // now fill with the proper indices in the same layout that shader expects
        // not that it really matters, but visualize it as [row][col] = [0][0] as the bottom-left element of 2D array
        GLuint counterIndex = 0;
        for (GLuint row = 0; row < GRID_LENGTH; ++row) {
            for (GLuint col = 0; col < GRID_LENGTH; ++col) {
                gridIndices.at(row).at(col) = counterIndex;
                ++counterIndex;
            }
        }

        // now using the vertex indices in this format, we can easily tesselate this grid into triangles as so...
        //TODO: draw a diagram comment here to better explain this
        for (GLuint row = 0; row < GRID_LENGTH - 1; ++row) {
            for (GLuint col = 0; col < GRID_LENGTH - 1; ++col) {
                // make 2 triangles (thus a square) from each of these indices acting as the bottom-left corner
                // ensures that the winding of all triangles is counter-clockwise

                m_waterGrid->drawFaces.push_back(gridIndices.at(row).at(col));
                m_waterGrid->drawFaces.push_back(gridIndices.at(row + 1).at(col + 1));
                m_waterGrid->drawFaces.push_back(gridIndices.at(row + 1).at(col));

                m_waterGrid->drawFaces.push_back(gridIndices.at(row).at(col));
                m_waterGrid->drawFaces.push_back(gridIndices.at(row).at(col + 1));
                m_waterGrid->drawFaces.push_back(gridIndices.at(row + 1).at(col + 1));
            }
        }

        m_waterGrid->textureID = m_renderEngine->load2DTexture("../../assets/textures/noise/waves/waves3/00.png"); //WARNING: THIS MAY HAVE TO BE CHANGED TO LOAD IN SPECIFICALLY WITH 8-bits (or may work, but should be optimized)
        // fallback #1 (no water grid)
        if (0 == m_waterGrid->textureID) m_waterGrid = nullptr;
        if (nullptr != m_waterGrid) {
            m_waterGrid->shaderProgramID = m_renderEngine->getWaterGridProgram();
            m_renderEngine->assignBuffers(*m_waterGrid);
        }

        //TODO: in the future, allow users to load in different terrains? (it would be nice to get program to work dynamically with whatever terrain it comes across) - probably not since finding terrain that works with my loader is hell
        // terrain...
        m_terrain = ObjectLoader::createTriMeshObject("../../assets/models/imports/everest.obj");
        if (nullptr != m_terrain) {
            if (m_terrain->hasTexture) {
                m_terrain->textureID = m_renderEngine->load2DTexture("../../assets/textures/everest.png");
                // fallback #1 (use default texture)
                if (0 == m_terrain->textureID) {
                    m_terrain->textureID = m_renderEngine->load2DTexture("../../assets/textures/default.png");
                    // fallback #2 (no terrain)
                    if (0 == m_terrain->textureID) m_terrain = nullptr;
                }
            }
            if (nullptr != m_terrain) {
                //m_terrain->generateNormals();
                m_terrain->setScale(glm::vec3{100.0f, 100.0f, 100.0f});
                m_terrain->shaderProgramID = m_renderEngine->getMainProgram();
                m_meshObjects.push_back(m_terrain);
                m_renderEngine->assignBuffers(*m_terrain);
            }
        }
    }

    void Program::queryGLVersion() {
        // query OpenGL version and renderer information
        std::string const GLV = reinterpret_cast<char const*>(glGetString(GL_VERSION));
        std::string const GLSLV = reinterpret_cast<char const*>(glGetString(GL_SHADING_LANGUAGE_VERSION));
        std::string const GLR = reinterpret_cast<char const*>(glGetString(GL_RENDERER));

        std::cout << "OpenGL [ " << GLV << " ] " << "with GLSL [ " << GLSLV << " ] " << "on renderer [ " << GLR << " ]" << std::endl;
    }

    bool Program::setupWindow() {
        // initialize the GLFW windowing system
        if (!glfwInit()) {
            std::cout << "ERROR: GLFW failed to initialize, TERMINATING..." << std::endl;
            return false;
        }

        // set the custom error callback function
        // errors will be printed to the console
        glfwSetErrorCallback(errorCallback);

        // attempt to create a window with an OpenGL 4.1 core profile context
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        // reference: https://stackoverflow.com/questions/42848322/what-does-my-choice-of-glfw-samples-actually-do
        //glfwWindowHint(GLFW_SAMPLES, 4);
        //glEnable(GL_MULTISAMPLE);
        int const WIDTH = 1024;
        int const HEIGHT = 1024;
        m_window = glfwCreateWindow(WIDTH, HEIGHT, "WaveTool", nullptr, nullptr);
        if (!m_window) {
            std::cout << "ERROR: Program failed to create GLFW window, TERMINATING..." << std::endl;
            glfwTerminate();
            return false;
        }

        // so that we can access this program object in callbacks...
        glfwSetWindowUserPointer(m_window, this);

        // set callbacks...
        glfwSetCursorPosCallback(m_window, InputHandler::motion);
        //glfwSetKeyCallback(m_window, keyCallback);
        glfwSetKeyCallback(m_window, InputHandler::key);
        glfwSetMouseButtonCallback(m_window, InputHandler::mouse);
        glfwSetScrollCallback(m_window, InputHandler::scroll);
        //glfwSetWindowSizeCallback(m_window, windowSizeCallback);
        glfwSetWindowSizeCallback(m_window, InputHandler::reshape);

        // bring the new window to the foreground (not strictly necessary but convenient)
        glfwMakeContextCurrent(m_window);
        // enable VSync
        glfwSwapInterval(1);

        // reference: https://www.khronos.org/opengl/wiki/OpenGL_Loading_Library#glad_.28Multi-Language_GL.2FGLES.2FEGL.2FGLX.2FWGL_Loader-Generator.29
        // glad uses GLFW loader to find appropriate OpenGL config (load OpenGL functions) for your system
        if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
            std::cout << "ERROR: Failed to initialize OpenGL context, TERMINATING..." << std::endl;
            glfwTerminate();
            return false;
        }

        // reference: https://blog.conan.io/2019/06/26/An-introduction-to-the-Dear-ImGui-library.html
        // setup Dear ImGui context...
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        // setup platform/renderer bindings...
        ImGui_ImplGlfw_InitForOpenGL(m_window, true);
        char const* glsl_version = "#version 410 core";
        //NOTE: must init glad before this call to not get an exception
        // reference: https://stackoverflow.com/questions/48582444/imgui-with-the-glad-opengl-loader-throws-segmentation-fault-core-dumped
        ImGui_ImplOpenGL3_Init(glsl_version);
        // set UI style...
        ImGui::StyleColorsDark();

        // query and print out information about our OpenGL environment
        queryGLVersion();

        return true;
    }

    void errorCallback(int error, char const* description) {
        std::cout << "GLFW ERROR: " << error << ":" << std::endl;
        std::cout << description << std::endl;
    }

/*
    void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
        Program *program = (Program*)glfwGetWindowUserPointer(window);

        // key codes are often prefixed with GLFW_KEY_ and can be found on the GLFW website
        if (GLFW_KEY_ESCAPE == key && GLFW_PRESS == action) {
            glfwSetWindowShouldClose(window, GL_TRUE);
        }
    }
*/

/*
    void windowSizeCallback(GLFWwindow *window, int width, int height) {
        glViewport(0, 0, width, height);
    }
*/
}
