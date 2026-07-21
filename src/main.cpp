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

	VBO VBO1(0, nullptr);
	EBO EBO1(0, nullptr);
    
	VAO1.LinkAttrib(VBO1, 0, 3, GL_FLOAT, 7 * sizeof(float), (void*)0);
	VAO1.LinkAttrib(VBO1, 1, 4, GL_FLOAT, 7 * sizeof(float), (void*)(3 * sizeof(float)));

	VAO1.Unbind();
	VBO1.Unbind();
	EBO1.Unbind();

	GLuint resUniID = glGetUniformLocation(shaderProgram.ID, "u_resolution");

    UIManager uIManager;

	//refaotring to use AddElement template function

    // uIManager.AddElement(&root);
	// uIManager.AddElement(&container);
    // uIManager.AddElement(&rect1);
    // uIManager.AddElement(&rect2);
    // uIManager.AddElement(&rect3);
    // uIManager.AddElement(&rect4);

    // uIManager.AddElement(&container2);
    // uIManager.AddElement(&rect5);
    // uIManager.AddElement(&rect6);
    // uIManager.AddElement(&rect7);
    // uIManager.AddElement(&rect8);

    // root.AddChild(&container);
    // container.AddChild(&rect1);
    // container.AddChild(&rect2);
    // container.AddChild(&rect3);
    // container.AddChild(&rect4);

    // root.AddChild(&container2);
    // container2.AddChild(&rect5);
    // container2.AddChild(&rect6);
    // container2.AddChild(&rect7);
    // container2.AddChild(&rect8);

    // container.padding = 20.0f;
    // container.centerHorizontally = false;
    // container.resizeChildren = false;
	// container.fitContentHeight = true;
	// container.fitContentWidth = true;

	// container2.padding = 12.0f;
    // container2.centerHorizontally = true;
    // container2.resizeChildren = true;
	// container2.fitContentHeight = true;
	// container2.fitContentWidth = false;

    // uIManager.EditElement(root.id,      { 0.0f, 0.0f, resolution[0], resolution[1], 1.0f, 1.0f, 1.0f, 1.0f }, true);
    // uIManager.EditElement(container.id, { 200.0f, 20.0f, 30.0f,        6.0f,        0.5f, 0.1f, 0.5f, 1.0f }, true);
    
    // uIManager.EditElement(rect1.id,     { 0.0f, 0.0f, 60.0f,         40.0f,         1.0f, 0.2f, 0.2f, 1.0f }, true); // Soft Red
    // uIManager.EditElement(rect2.id,     { 0.0f, 0.0f, 120.0f,        80.0f,         0.2f, 0.8f, 0.2f, 1.0f }, true); // Soft Green
    // uIManager.EditElement(rect3.id,     { 0.0f, 0.0f, 40.0f,         60.0f,        0.2f, 0.4f, 1.0f, 1.0f }, true); // Soft Blue
    // uIManager.EditElement(rect4.id,     { 0.0f, 0.0f, 90.0f,         90.0f,         1.0f, 0.8f, 0.2f, 1.0f }, true); // Yellow

    // uIManager.EditElement(container2.id, { 320.0f, 230.0f, 200.0f,        300.0f,        0.2f, 0.8f, 0.2f, 1.0f }, true);
    
    // uIManager.EditElement(rect5.id,     { 0.0f, 0.0f, 60.0f,         40.0f,         0.3f, 1.0f, 0.4f, 1.0f }, true);
    // uIManager.EditElement(rect6.id,     { 0.0f, 0.0f, 120.0f,        80.0f,         1.0f, 0.2f, 0.8f, 1.0f }, true);
    // uIManager.EditElement(rect7.id,     { 0.0f, 0.0f, 40.0f,         60.0f,        0.4f, 0.1f, 0.7f, 1.0f }, true);
    // uIManager.EditElement(rect8.id,     { 0.0f, 0.0f, 90.0f,         90.0f,         0.8f, 1.0f, 0.4f, 1.0f }, true);
	
	auto root = uIManager.AddElement<UIRect>();
	auto container = uIManager.AddElement<VerticalContainer>();
	auto rect1 = uIManager.AddElement<UIRect>();
	auto rect2 = uIManager.AddElement<UIRect>();
	auto rect3 = uIManager.AddElement<UIRect>();
	auto rect4 = uIManager.AddElement<UIRect>();

	uIManager.AddChild(root, container);
	uIManager.AddChild(container, rect1);
	uIManager.AddChild(container, rect2);
	uIManager.AddChild(container, rect3);
	uIManager.AddChild(container, rect4);

	uIManager.EditElement(root,      { 0.0f, 0.0f, resolution[0], resolution[1], 1.0f, 1.0f, 1.0f, 1.0f }, true);
	uIManager.EditElement(container, { 200.0f, 20.0f, 30.0f,        6.0f,        0.5f, 0.1f, 0.5f, 1.0f }, true);
	uIManager.EditElement(rect1,     { 0.0f, 0.0f, 60.0f,         40.0f,         0.3f, 1.0f, 0.4f, 1.0f }, true);
	uIManager.EditElement(rect2,     { 0.0f, 0.0f, 120.0f,        80.0f,         0.2f, 0.8f, 0.2f, 1.0f }, true);
	uIManager.EditElement(rect3,     { 0.0f, 0.0f, 40.0f,         60.0f,        0.2f, 0.4f, 1.0f, 1.0f }, true);
	uIManager.EditElement(rect4,     { 0.0f, 0.0f, 90.0f,         90.0f,         1.0f, 0.8f, 0.2f, 1.0f }, true);

	

	while(!glfwWindowShouldClose(window)){
        glfwWaitEventsTimeout(1.0);
		glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

        uIManager.StepFrame(resolution);

		shaderProgram.Activate();
		glUniform2f(resUniID, static_cast<float>(resolution[0]), static_cast<float>(resolution[1]));

		VAO1.Bind();

        const auto& dynamicVerts = uIManager.globalVertices;
        const auto& dynamicIndices = uIManager.globalIndices;

        VBO1.Bind();
        glBufferData(GL_ARRAY_BUFFER, dynamicVerts.size() * sizeof(Vertex), dynamicVerts.data(), GL_DYNAMIC_DRAW);

        EBO1.Bind();
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, dynamicIndices.size() * sizeof(GLuint), dynamicIndices.data(), GL_DYNAMIC_DRAW);

		for (const auto& cmd : uIManager.drawCommands) {
				if (cmd.indexCount == 0) continue;

				if (cmd.useScissor) {
					glEnable(GL_SCISSOR_TEST);
					glScissor(cmd.scissorBox.x, cmd.scissorBox.y, cmd.scissorBox.w, cmd.scissorBox.h);
				} else {
					glDisable(GL_SCISSOR_TEST);
				}

				glDrawElements(
					GL_TRIANGLES, 
					static_cast<GLsizei>(cmd.indexCount), 
					GL_UNSIGNED_INT, 
					(void*)(cmd.indexOffset * sizeof(GLuint)) // Offset in bytes!
				);
			}
		glfwSwapBuffers(window);
        glDisable(GL_SCISSOR_TEST);
	}

	VAO1.Delete();
	VBO1.Delete();
	EBO1.Delete();
	shaderProgram.Delete();

    glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}
