#include <iostream>
#include <chrono>
#include <thread>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <soloud/include/soloud.h>
#include <soloud/include/soloud_wav.h>

#include "graphics/shader.h"
#include "graphics/texture.h"
#include "camera/cameraFps.h"
#include "graphics/mesh.h"
#include "screen.h"
#include "input.h"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "tiny_gltf.h"
#include "font.h"
#include "graphics/model.h"

const unsigned int SCREEN_W = 1920;
const unsigned int SCREEN_H = 1200;
const float SCREEN_FOV = 90.f;

const unsigned int TARGET_FPS = 75;
const double TARGET_FRAMETIME = 1.0 / TARGET_FPS;

int main() {
    Screen screen(SCREEN_W, SCREEN_H, SCREEN_FOV, "Pain");
    WindowState windowState;
    windowState.camera = new CameraFps;
    glfwSetWindowUserPointer(screen.window, (void*) &windowState);
    Input::registerCallbacks(screen.window);

    // TODO: Move into screen.cpp
    Shader postShader("shaders/post.vert", "shaders/post.frag");
    postShader.use();
    postShader.setUniform("tex", 0);
    postShader.setUniform("texDither", 1);
    std::string ditherFilePath("res/textures/dith.png");
    Texture textureDither(ditherFilePath);

    // Test font renderer
    Font font("The quick brown fox jumps over the lazy dog.");

    // Main shader program
    Shader shader("shaders/vertex.vert", "shaders/fragment.frag");
    shader.use();
    shader.setUniform("tex", 0);

    // Scene
    glm::vec4 sunPosition = glm::vec4(0.f, -5.f, 0.f, 1.0);
    glm::vec3 sunColor = glm::vec3(1.0f, 1.0f, 1.0f);
    shader.use();
    shader.setUniform("sunColor", sunColor);

    Model suzanne = Model("res/suzanne.glb", "res/textures/0.png");
    suzanne.setMaterial(Materials::metallic);
    suzanne.translate(0.f, 0.0f, 5.f);

    Model cube = Model("res/cube.glb", "res/textures/1.png");
    cube.setMaterial(Materials::emissive);
    cube.translate(5.f, 0.f, 0.f);

    Model sphere = Model("res/sphere.glb", "res/textures/3.png");
    sphere.translate(0.f, 5.f, 0.f);
    sphere.scale(1.f, 1.f, 1.f);

    // Audio
    SoLoud::Soloud soloud; // Engine core
    SoLoud::Wav sample;    // One sample

    // Initialize SoLoud, miniaudio backend
    soloud.init(SoLoud::Soloud::CLIP_ROUNDOFF | SoLoud::Soloud::ENABLE_VISUALIZATION, SoLoud::Soloud::BACKENDS::MINIAUDIO);
    soloud.setGlobalVolume(.5f);

    sample.load("res/sound/dunkelheit.ogg"); // Load a wave file
    sample.setLooping(true);
    sample.set3dMinMaxDistance(1, 30);
    sample.set3dAttenuation(SoLoud::AudioSource::EXPONENTIAL_DISTANCE, 0.5);
    SoLoud::handle sampleHandle = soloud.play3d(sample, 0.f, 0.f, 0.f);        // Play it

    // Timing
    double timeNow, timeLast = 0.;
    unsigned long frames = 0;

    while (!glfwWindowShouldClose(screen.window)) {
        // Calculate frame time, cap framerate to target
        timeLast = timeNow;
        timeNow = glfwGetTime();
        while ((timeNow - timeLast) < TARGET_FRAMETIME) timeNow = glfwGetTime();
        windowState.timeDelta = timeNow - timeLast;

        if (frames % (TARGET_FPS*5) == 0)
            std::cout << "Previous frame time: "
                      << windowState.timeDelta*1e3 << "ms\t("
                      << 1/windowState.timeDelta << "fps)"
                      << std::endl;

        // Handle continuous response keys
        Input::processContinuousInput(screen.window);

        // Clear FB
        glClearColor(0.f, 0.f, 0.f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Render scene
        glm::vec3 viewSunPos = windowState.camera->getView() * (glm::mat4(1.0f) * sunPosition);
        shader.use();
        shader.setUniform("sunPosition", viewSunPos);

        cube.draw(shader, screen.perspective, windowState.camera->getView());

        sphere.rotate(
            (360.f * .1 /* rev per s */) * windowState.timeDelta, 0.f, 1.f, 0.f, false
        );
        sphere.draw(shader, screen.perspective, windowState.camera->getView());

        suzanne.rotate(
            (360.f * 1.) * windowState.timeDelta, 1.f, 0.f, 0.f, false
        );
        suzanne.draw(shader, screen.perspective, windowState.camera->getView());


        // Update audio
//        glm::vec4 suzannePosView =  modelView * glm::vec4(0.5,0.5,0.5,1.0);
//        float w = suzannePosView.w;
//        soloud.set3dSourceParameters(
//            sampleHandle,
//            suzannePosView.x/w, suzannePosView.y/w, suzannePosView.z/w,
//            0.f, 0.f, 0.f
//        );
//
//        soloud.set3dListenerPosition(0,0,0);
//
//        soloud.update3dAudio();

        // Render UI
        font.draw();

        screen.flip(postShader, textureDither.handle);
        frames++;
    }

    delete windowState.camera;

    return 0;
}
