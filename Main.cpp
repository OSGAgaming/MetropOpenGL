#include<iostream>
#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include<stb/stb_image.h>

#include"VAO.h"
#include"VBO.h"
#include"EBO.h"
#include"Shader.h"
#include"Texture.h"
#include"Window.h"

GLfloat vertices[] =
{
	-1.0f,-1.0f,0.0f,       0.0f, 0.0f, 0.0f,		0.0f,0.0f,
	1.0f, -1.0f,0.0f,       0.0f, 0.0f, 0.0f,		1.0f,0.0f,
	1.0f, 1.0f,0.0f,        0.0f, 0.0f, 0.0f,		1.0f,1.0f,
	-1.0f, 1.0f,0.0f,		0.0f, 0.0f, 0.0f,		0.0f,1.0f
};

GLuint indices[] =
{
	0, 1, 2,
	0, 2, 3
};

const unsigned int SCREEN_WIDTH = 1024;
const unsigned int SCREEN_HEIGHT = 1024;

void print_prefferred_workgroup_size() {
	int work_grp_cnt[3];
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &work_grp_cnt[0]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &work_grp_cnt[1]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &work_grp_cnt[2]);
	std::cout << "Max work groups per compute shader" <<
		" x:" << work_grp_cnt[0] <<
		" y:" << work_grp_cnt[1] <<
		" z:" << work_grp_cnt[2] << "\n";

	int work_grp_size[3];
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &work_grp_size[0]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &work_grp_size[1]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &work_grp_size[2]);
	std::cout << "Max work group sizes" <<
		" x:" << work_grp_size[0] <<
		" y:" << work_grp_size[1] <<
		" z:" << work_grp_size[2] << "\n";

	int work_grp_inv;
	glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &work_grp_inv);
	std::cout << "Max invocations count per work group: " << work_grp_inv << "\n";
}

int main() 
{
	Window win("Metropolis", 800, 800);
	Shader shader("default.vert", "default.frag");
	Shader computeShader("compute.glsl");

	//-------------------------------------
	//VERTEX OBJECTS					  
	//-------------------------------------

	VAO VAO1;
	VAO1.Bind();

	VBO VBO1(vertices, sizeof(vertices));
	EBO EBO1(indices, sizeof(indices));

	VAO1.LinkAttrib(VBO1, 0, 3, GL_FLOAT, 8 * sizeof(float), (void*)0);
	VAO1.LinkAttrib(VBO1, 1, 3, GL_FLOAT, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	VAO1.LinkAttrib(VBO1, 2, 2, GL_FLOAT, 8 * sizeof(float), (void*)(6 * sizeof(float)));

	//Unbind
	VBO1.Unbind();
	VAO1.Unbind();
	EBO1.Unbind();

	//-------------------------------------
	//LOAD TEXTURES					  
	//-------------------------------------
	Texture tex(GL_TEXTURE_2D, SCREEN_WIDTH, SCREEN_HEIGHT, 0);

	print_prefferred_workgroup_size();

	while(!win.ShouldClose())
	{
		computeShader.Activate(true, SCREEN_WIDTH / 8, SCREEN_HEIGHT / 8, 1);
		shader.Activate();
		tex.texUnit(shader, "screen", 0);

		VAO1.Bind();
		glDrawElements(GL_TRIANGLES,6,GL_UNSIGNED_INT,0);
		win.SwapBuffers();

		glfwPollEvents();
	}

	VAO1.Delete();
	VBO1.Delete();
	EBO1.Delete();
	shader.Delete();
	tex.Delete();

	win.Delete();
	glfwTerminate();

	return 0;
}