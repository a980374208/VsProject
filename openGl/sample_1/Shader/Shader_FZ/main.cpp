#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath> 
#include "shader.h"
float vertices[] = { // λ�� // ��ɫ 
	0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, // ���� 
	-0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, // ���� 
	0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f // �� 
};

void processInput(GLFWwindow* window);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
int main() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__ 
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
	GLFWwindow* window = glfwCreateWindow(800, 600, "LearnOpenGL", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	//GLFW�����ڵ�����������Ϊ��ǰ�̵߳�������
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	//GLAD
	// glad: ��������OpenGL����ָ��
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	Shader ourShader("shaders/shader.vs","shaders/shader.fs");

	//����VBO��VAO���󣬲�����ID
	unsigned int VBO, VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	//��VBO��VAO����
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	//Ϊ��ǰ�󶨵�target�Ļ��������󴴽�һ���µ����ݴ洢��
	//���data����NULL����ʹ�����Դ�ָ������ݳ�ʼ�����ݴ洢
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	//��֪Shader��ν��������������ֵ
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	//����VAO����ĵ�һ������ֵ
	glEnableVertexAttribArray(0);

	//��֪Shader��ν��������������ֵ
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	//����VAO����ĵ�һ������ֵ
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// ��Ⱦѭ��
	while (!glfwWindowShouldClose(window)) {
		processInput(window);
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f); //״̬����
		glClear(GL_COLOR_BUFFER_BIT); //״̬ʹ��

		ourShader.use();

		// ����uniformֵ
		float timeValue = glfwGetTime();
		float greenValue = sin(timeValue) / 2.0f + 0.5f;
		ourShader.setVec4("ourColor", 0.0f, greenValue, 0.0f, 1.0f);

		glBindVertexArray(VAO);
		// glfw: ��������������ѯIO�¼�����������/�ͷš�����ƶ��ȣ�
		glDrawArrays(GL_TRIANGLES, 0, 3);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	// glfw: ����ǰ������GLFW�ȹ���Դ. 
	glfwTerminate();
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteProgram(ourShader.ID);

	return 0;
}

void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}