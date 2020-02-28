#include "render-engine.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

namespace wave_tool {
    RenderEngine::RenderEngine(GLFWwindow *window) {
        int width, height;
        glfwGetWindowSize(window, &width, &height);

        //NOTE: near distance must be small enough to not conflict with skybox size
        m_camera = std::make_shared<Camera>(72.0f, (float)width / height, 0.1f, 5000.0f, glm::vec3(0.0f, 1000.0f, 1000.0f));

        skyboxProgram = ShaderTools::compileShaders("../assets/shaders/skybox.vert", "../assets/shaders/skybox.frag");
        trivialProgram = ShaderTools::compileShaders("../assets/shaders/trivial.vert", "../assets/shaders/trivial.frag");
        mainProgram = ShaderTools::compileShaders("../assets/shaders/main.vert", "../assets/shaders/main.frag");
        //lightProgram = ShaderTools::compileShaders("../assets/shaders/light.vert", "../assets/shaders/light.frag");

        //NOTE: currently placing the light at the top of the y-axis
        lightPos = glm::vec3(0.0f, 500.0f, 0.0f);

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
    void RenderEngine::render(std::shared_ptr<const MeshObject> skybox, std::vector<std::shared_ptr<MeshObject>> const& objects) {
        glm::mat4 const view = m_camera->getViewMat();
        glm::mat4 const projection = m_camera->getProjectionMat();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // render skybox first...
        // reference: https://learnopengl.com/Advanced-OpenGL/Cubemaps
        // reference: http://antongerdelan.net/opengl/cubemaps.html
        if (nullptr != skybox && skybox->m_isVisible) {
            // disable depth writing to draw skybox behind everything else
            glDepthMask(GL_FALSE);
            // enable skybox shader
            glUseProgram(skyboxProgram);
            glm::mat4 const viewNoTranslation = glm::mat4(glm::mat3(view));
            glm::mat4 const VPNoTranslation = projection * viewNoTranslation;
            // set VP matrix uniform in shader program
            glUniformMatrix4fv(glGetUniformLocation(skyboxProgram, "VPNoTranslation"), 1, GL_FALSE, glm::value_ptr(VPNoTranslation));
            // bind geometry data...
            glBindVertexArray(skybox->vao);
            // bind texture...
            glActiveTexture(GL_TEXTURE0 + skybox->textureID);
            glBindTexture(GL_TEXTURE_CUBE_MAP, skybox->textureID);
            // set skybox samplerCube uniform in shader program
            glUniform1i(glGetUniformLocation(skyboxProgram, "skybox"), skybox->textureID);
            // POINT, LINE or FILL...
            glPolygonMode(GL_FRONT_AND_BACK, skybox->m_polygonMode);
            glDrawElements(skybox->m_primitiveMode, skybox->drawFaces.size(), GL_UNSIGNED_INT, (void*)0);
            glBindVertexArray(0); // unbind vao
            // unbind texture...
            glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
            // re-enable depth writing for next geometry
            glDepthMask(GL_TRUE);
        }

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
            glUniform3fv(glGetUniformLocation(mainProgram, "lightPos"), 1, glm::value_ptr(lightPos));
            //glUniform3fv(glGetUniformLocation(mainProgram, "lightPos"), 1, glm::value_ptr(camera->getPosition())); // set light pos as camera pos

            glUniform1i(glGetUniformLocation(mainProgram, "hasTexture"), o->hasTexture);
            glUniform1i(glGetUniformLocation(mainProgram, "hasNormals"), !o->normals.empty());

            // POINT, LINE or FILL...
            glPolygonMode(GL_FRONT_AND_BACK, o->m_polygonMode);

            glDrawElements(o->m_primitiveMode, o->drawFaces.size(), GL_UNSIGNED_INT, (void*)0);

            glBindVertexArray(0);
            Texture::unbind2DTexture();
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

    // Assigns and binds buffers for a Mesh Object - vertices, normals, UV coordinates, faces
    void RenderEngine::assignBuffers(MeshObject &object)
    {
        std::vector<glm::vec3> &vertices = object.drawVerts;
        std::vector<glm::vec3> &normals = object.normals;
        std::vector<glm::vec2> &uvs = object.uvs;
        std::vector<glm::vec3> &colours = object.colours;
        std::vector<GLuint> &faces = object.drawFaces;

        // Bind attribute array for triangles
        glGenVertexArrays(1, &object.vao);
        glBindVertexArray(object.vao);

        // Vertex buffer
        //NOTE: every object should have verts
        // location 0 in vao
        glGenBuffers(1, &object.vertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, object.vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glEnableVertexAttribArray(0);

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
        //NOTE: assuming every object is using an index buffer (thus glDrawElements is always used)
        // this is fully compatible since if an object has only verts and wanted to use glDrawArrays, then it could just initialize a trivial index buffer (0,1,2,...,verts.size()-1)
        glGenBuffers(1, &object.indexBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, object.indexBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)*faces.size(), faces.data(), GL_STATIC_DRAW);

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
    void RenderEngine::updateLightPos(glm::vec3 add) {
        lightPos += add;
    }

    // Sets projection and viewport for new width and height
    void RenderEngine::setWindowSize(int width, int height) {
        m_camera->setAspect((float)width / height);
        glViewport(0, 0, width, height);
    }
}
