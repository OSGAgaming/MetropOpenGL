#ifndef SHADER_CLASS_H
#define SHADER_CLASS_H

#include<glad/glad.h>
#include<string>
#include<fstream>
#include<sstream>
#include<iostream>
#include<cerrno>

std::string get_file_contents(const char* filename);

class Shader {
public:
	GLuint ID;
	Shader(const char* vertexFile, const char* fragmentFile);
	Shader(const char* computeFile);

	void Activate(bool compute = false, GLuint gX = 0, GLuint gY = 0, GLuint gZ = 0);
	void Delete();
};

#endif // !SHADER_CLASS_H