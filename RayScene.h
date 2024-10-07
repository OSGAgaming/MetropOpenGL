#pragma once
#include"Scene.h";
#include "EBO.h"
#include "VAO.h"
#include "Camera.h"
#include "Texture.h"
#include "Shader.h"

class RayScene : public Scene {
public:
    const unsigned int SCREEN_WIDTH = 1024;
    const unsigned int SCREEN_HEIGHT = 1024;

    Shader computeShader;
    Shader shader;

    Texture tex;
    Texture oldTex;
    Texture averageTex;

    Camera camera;

    VAO SceneVAO;
    std::unique_ptr<VBO> SceneVBO;
    std::unique_ptr<EBO> SceneEBO;

    int Frame = 0;

    void AddSurfaces();

    void OnBufferSwap(Window& win) override;
    void OnWindowLoad(Window& win) override;
    void OnWindowClose(Window& win) override;

    RayScene();
};