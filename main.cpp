// main.cpp
//

#include <iostream>
#include <filesystem>
#include <map>
#include <stack>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "game.h"
#include "check_error.h"
#include "texture_2D.h"
#include "renderer.h"
#include "entity.h"
#include "ecs.h"

Game Game::main;
ECS ECS::main;

// This is the hub which handles updates and setup.
// In an attempt to keep this from getting cluttered, we're keeping some information
// regarding game and world states in the game and engine class respectively.
// We don't want this to be so cluttered that you have to dig to find what you want
// so don't put stuff here unless it really belongs.

int main(void)
{
    int windowWidth = Game::main.windowWidth;
    int windowHeight = Game::main.windowHeight;

    #pragma region GL Rendering Setup
    // Here we're initiating all the stuff related to rendering.
    GLFWwindow* window;

    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwWindowHintString(GLFW_X11_CLASS_NAME, "OpenGL");
    glfwWindowHintString(GLFW_X11_INSTANCE_NAME, "OpenGL");

    window = glfwCreateWindow(windowWidth, windowHeight, "The Elder Tongue", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << '\n';
        return -1;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Game::main.window = window;
    #pragma endregion

    #pragma region World Setup

    ECS::main.Init();

    #pragma endregion

    #pragma region Camera & Texture Setup
    // Now that we've finished that, we'll set up the camera
    // and textures that we'll need later.
    Game::main.updateOrtho();

    Texture2D* whiteTexture = Texture2D::whiteTexture();

    Renderer renderer{ whiteTexture->ID };

    Texture2D test{ "assets/sprites/test.png", true, GL_NEAREST };
    renderer.textureIDs.push_back(test.ID);
    Game::main.textureMap.emplace("test", &test);

    Texture2D blank{ "assets/sprites/blank.png", true, GL_NEAREST };
    renderer.textureIDs.push_back(blank.ID);
    Game::main.textureMap.emplace("blank", &blank);

    Game::main.renderer = &renderer;
    #pragma endregion

    #pragma region Game Loop
    // This is the loop where the game runs, duh.
    // Everything that should happen each frame should occur here,
    // or nested somewhere within methods called in here.
    double lastTime = glfwGetTime();
    int frameCount = 0;

    float checkedTime = glfwGetTime();
    float elapsedTime = 0.0f;

    while (!glfwWindowShouldClose(window))
    {
        #pragma region Elapsed Time

        float deltaTime = glfwGetTime() - checkedTime;
        // std::cout << "Delta Time: " + std::to_string(deltaTime) + "\n";
        checkedTime = glfwGetTime();

        #pragma endregion

        #pragma region FPS
        double currentTime = glfwGetTime();
        frameCount++;
        // If a second has passed.
        if (currentTime - lastTime >= 1.0)
        {
            // Display the frame count here any way you want.
            std::cout << "Frame Count: " + std::to_string(frameCount) + "\n";

            frameCount = 0;
            lastTime = currentTime;
        }

        #pragma endregion

        #pragma region Update Worldview
        // Here, we make sure the camera is oriented correctly.
        glm::vec3 cam = glm::vec3(Game::main.camX, Game::main.camY, Game::main.camZ);
        glm::vec3 center = cam + glm::vec3(0.0f, 0.0f, -1.0f);
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
        Game::main.view = glm::lookAt(cam, center, up);

        // Here we manage the game window and view.
        const float halfWindowHeight = Game::main.windowHeight * Game::main.zoom * 0.5f;
        const float halfWindowWidth = Game::main.windowWidth * Game::main.zoom * 0.5f;
        Game::main.topY = Game::main.camY + halfWindowHeight;
        Game::main.bottomY = Game::main.camY - halfWindowHeight;
        Game::main.rightX = Game::main.camX + halfWindowWidth;
        Game::main.leftX = Game::main.camX - halfWindowWidth;
        #pragma endregion

        #pragma region Test Input

        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS ||
            glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, true);
        }

        if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS)
        {
            Game::main.zoom -= 0.01f;
            Game::main.updateOrtho();
        }
        else if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS)
        {
            Game::main.zoom += 0.01f;

            Game::main.updateOrtho();
        }

        #pragma endregion

        #pragma region GL Color & Clear
        // Here we update the background color.
        // We want it black for now, so that's what it is.
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        #pragma endregion

        #pragma region Update World State

        ECS::main.Update(deltaTime);

        #pragma endregion;

        #pragma region Render
        // This is where we finally render and reset buffers.
        Game::main.renderer->sendToGL();
        Game::main.renderer->resetBuffers();

        glfwSwapBuffers(window);

        glfwPollEvents();

        glCheckError();
        #pragma endregion
    }
    #pragma endregion

    #pragma region Shutdown
    delete whiteTexture;

    glfwTerminate();
    return 0;
    #pragma endregion
}
