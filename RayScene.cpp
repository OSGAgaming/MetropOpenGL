#include"RayScene.h";
#include "ComputeStructures.h"

    RayScene::RayScene() :
        shader("default.vert", "default.frag"),
        computeShader("compute.glsl"),
        tex(GL_TEXTURE_2D, SCREEN_WIDTH, SCREEN_HEIGHT, GL_TEXTURE0,0, GL_READ_WRITE),
        oldTex(GL_TEXTURE_2D, SCREEN_WIDTH, SCREEN_HEIGHT, GL_TEXTURE1,6, GL_READ_WRITE),
        averageTex(GL_TEXTURE_2D, SCREEN_WIDTH, SCREEN_HEIGHT, GL_TEXTURE2, 7, GL_READ_WRITE),
        camera(SCREEN_WIDTH, SCREEN_HEIGHT, glm::vec3(0.0f, 0.0f, -5.0f)) {}

    void RayScene::AddSurfaces() {
        std::vector<TraceCircle> circles = {};
        const float noOfCircles = 5;
        for (int i = 0; i < noOfCircles; i++) {
            TraceCircle circle;
            Material material;

            material.emmisionColor = vec3(1.0f, 1.0f, 1.0f);
            material.emmisionStrength = (i == floor(noOfCircles / 2)) ? vec3(1, 1, 1) : vec3(0, 0, 0);
            material.diffuseColor = (i == 0) ? vec3(1.0f, 1.0f, 1.0f) : vec3(1.0f, 1 - i / noOfCircles, 1 - i / noOfCircles);
            material.smoothness = vec3(i / noOfCircles,0,0);
            circle.material = material;
            circle.position = (i == floor(noOfCircles / 2)) ? 
                vec3(i * 2.4f - noOfCircles * 0.5f * 2.4f, 1.0f, 5.0f) :
                vec3(i * 2.4f - noOfCircles * 0.5f * 2.4f, 0, 5.0f);
            circle.radius = 1.0f;
            circles.push_back(circle);
        }

        TraceCircle circle;
        Material material;

        material.emmisionColor = vec3(1.0f, 1.0f, 1.0f);
        material.emmisionStrength = vec3(0, 0, 0);
        material.diffuseColor = vec3(0.0f, 0.0f, 1.0f);
        circle.material = material;
        circle.position = vec3(0, -9.0f, 11.0f);
        circle.radius = 10.0f;
        circles.push_back(circle);

        TraceCircle circle2;
        Material material2;

        material2.emmisionColor = vec3(1.0f, 1.0f, 1.0f);
        material2.emmisionStrength = vec3(4.0f, 4.0f, 4.0f);
        material2.diffuseColor = vec3(1.0f, 1.0f, 1.0f);
        circle2.material = material2;
        circle2.position = vec3(0, 0, 25.0f);
        circle2.radius = 10.0f;
        circles.push_back(circle2);

        computeShader.StoreSSBO<TraceCircle>(circles, 2);
        computeShader.StoreSSBO<GLuint>(static_cast<GLuint>(circles.size()), 3);

    }

    void RayScene::OnBufferSwap(Window& win) {

        bool hasMoved = camera.Inputs(win.instance);
        camera.Matrix(45.0f, 0.1f, 100.0f, computeShader, "viewProj");

        if (hasMoved) Frame = 0;

        glMemoryBarrier(GL_ALL_BARRIER_BITS);
        glCopyImageSubData(tex.ID, GL_TEXTURE_2D, 0, 0, 0, 0,
            oldTex.ID, GL_TEXTURE_2D, 0, 0, 0, 0,
            SCREEN_WIDTH, SCREEN_HEIGHT, 1);

        GLuint frame = computeShader.StoreSSBO<GLuint>(Frame, 4);
        Frame++;


        // CAMERA PARAMS
        CameraSettings cameraSettings;
        cameraSettings.position = vec3(camera.Position.x, camera.Position.y, camera.Position.z);
        cameraSettings.direction = vec3(camera.Orientation.x, camera.Orientation.y, camera.Orientation.z);
        cameraSettings.fov = 90.0f;
        GLuint camSettings = computeShader.StoreSSBO<CameraSettings>(cameraSettings, 1);

        AddSurfaces(); // Only update surfaces if necessary
        
        computeShader.Activate(true, SCREEN_WIDTH / 8, SCREEN_HEIGHT / 8, 1);

        shader.Activate();

        tex.texUnit(shader, "tex0", 0);
        oldTex.texUnit(shader, "tex1", 1);
        averageTex.texUnit(shader, "tex2", 2);

        shader.SetParameterInt(Frame, "Frame");
        SceneVAO.Bind();
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        computeShader.DeleteSSBOs();
    }

    void RayScene::OnWindowLoad(Window& win) {
        //-------------------------------------
        // VERTEX OBJECTS                     
        //-------------------------------------
        SceneVAO.Bind();

        GLfloat vertices[] =
        {
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
             1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,
            -1.0f,  1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f
        };

        GLuint indices[] =
        {
            0, 1, 2,
            0, 2, 3
        };

        // Initialize SceneVBO and SceneEBO using pointers
        SceneVBO = std::make_unique<VBO>(vertices, sizeof(vertices));
        SceneEBO = std::make_unique<EBO>(indices, sizeof(indices));

        SceneVAO.LinkAttrib(*SceneVBO, 0, 3, GL_FLOAT, 8 * sizeof(float), (void*)0);
        SceneVAO.LinkAttrib(*SceneVBO, 1, 3, GL_FLOAT, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        SceneVAO.LinkAttrib(*SceneVBO, 2, 2, GL_FLOAT, 8 * sizeof(float), (void*)(6 * sizeof(float)));

        // Unbind
        SceneVBO->Unbind();
        SceneVAO.Unbind();
        SceneEBO->Unbind();
    }

    void RayScene::OnWindowClose(Window& win) {

        SceneVAO.Delete();
        SceneVBO->Delete();
        SceneEBO->Delete();
        shader.Delete();
        tex.Delete();
    }
