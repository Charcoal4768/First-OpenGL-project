#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <stdio.h>
#include <cmath>
#include <openglBasics/shaderclass.h>
#include <openglBasics/VAO.h>
#include <openglBasics/VBO.h>
#include <openglBasics/EBO.h>
#include <UI/UIElements.h>

std::array<float, 2> resolution{
	800.0f,
	800.0f
};

const char* TITLE = "Index buffer testing";

// This function triggers automatically whenever the window size changes
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
	resolution = {static_cast<float>(width), static_cast<float>(height)};
}

int main(){

	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(resolution[0], resolution[1], TITLE, NULL, NULL);
	if (window == NULL){
		std::cout << "Failed to make GLFW window." << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	glViewport(0,0,resolution[0], resolution[1]);

	Shader shaderProgram("default.vert", "default.frag");

	VAO VAO1;
	VAO1.Bind();

	// Initialize with empty streaming configurations instead of hardcoded triangles
	VBO VBO1(0, nullptr);
	EBO EBO1(0, nullptr);
    
	// 7 * sizeof(float) matches your Vertex struct exactly: pos[3] (3) + color[4] (4)
	VAO1.LinkAttrib(VBO1, 0, 3, GL_FLOAT, 7 * sizeof(float), (void*)0);
	VAO1.LinkAttrib(VBO1, 1, 4, GL_FLOAT, 7 * sizeof(float), (void*)(3 * sizeof(float)));

	VAO1.Unbind();
	VBO1.Unbind();
	EBO1.Unbind();

	GLuint resUniID = glGetUniformLocation(shaderProgram.ID, "u_resolution");

    UIManager uIManager;

    // 1. Declare pure behavior instances
    AnchorElement root;
    VerticalContainer container;
    UIRect rect1;
    UIRect rect2;
    UIRect rect3;
    UIRect rect4;

    uIManager.AddElement(&root);
    uIManager.AddElement(&container);
    uIManager.AddElement(&rect1);
    uIManager.AddElement(&rect2);
    uIManager.AddElement(&rect3);
    uIManager.AddElement(&rect4);

    root.AddChild(&container);
    container.AddChild(&rect1);
    container.AddChild(&rect2);
    container.AddChild(&rect3);
    container.AddChild(&rect4);

    container.padding = 20.0f;
    container.centerHorizontally = true;
    container.resizeChildren = true;

    uIManager.EditElement(root.id,      { 0.0f, 0.0f, resolution[0], resolution[1], 1.0f, 1.0f, 1.0f, 1.0f }, true);
    uIManager.EditElement(container.id, { 200.0f, 20.0f, 300.0f,        600.0f,        0.5f, 0.1f, 0.5f, 1.0f }, true);
    
    uIManager.EditElement(rect1.id,     { 0.0f, 0.0f, 60.0f,         40.0f,         1.0f, 0.2f, 0.2f, 1.0f }, true); // Soft Red
    uIManager.EditElement(rect2.id,     { 0.0f, 0.0f, 120.0f,        80.0f,         0.2f, 0.8f, 0.2f, 1.0f }, true); // Soft Green
    uIManager.EditElement(rect3.id,     { 0.0f, 0.0f, 40.0f,         160.0f,        0.2f, 0.4f, 1.0f, 1.0f }, true); // Soft Blue
    uIManager.EditElement(rect4.id,     { 0.0f, 0.0f, 90.0f,         90.0f,         1.0f, 0.8f, 0.2f, 1.0f }, true); // Yellow

	while(!glfwWindowShouldClose(window)){
        glfwWaitEventsTimeout(1.0);
		glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

        // This runs updates across dataTables and constructs geometry
        uIManager.StepFrame(resolution);

		shaderProgram.Activate();
		glUniform2f(resUniID, static_cast<float>(resolution[0]), static_cast<float>(resolution[1]));

		VAO1.Bind();

        const auto& dynamicVerts = uIManager.globalVertices;
        const auto& dynamicIndices = uIManager.globalIndices;

        // Stream generated engine vertex results straight into GPU buffers
        VBO1.Bind();
        glBufferData(GL_ARRAY_BUFFER, dynamicVerts.size() * sizeof(Vertex), dynamicVerts.data(), GL_DYNAMIC_DRAW);

        EBO1.Bind();
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, dynamicIndices.size() * sizeof(GLuint), dynamicIndices.data(), GL_DYNAMIC_DRAW);

        if(!dynamicIndices.empty()){
            glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(dynamicIndices.size()), GL_UNSIGNED_INT, 0);
        }

		glfwSwapBuffers(window);
	}

	VAO1.Delete();
	VBO1.Delete();
	EBO1.Delete();
	shaderProgram.Delete();

    glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}
