#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath> 
#include "shader.h"
#include "stb_image.h"

#include <glm/glm.hpp> 
#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtc/type_ptr.hpp>
#include "cmath"

void processInput(GLFWwindow* window);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

float vertices[] = {
// λ��              // ��ɫ           // ��������
 0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // ����
 0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // ����
-0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // ����
-0.5f,  0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f  // ���� 
};

unsigned int indices[] = { // ע�⣬���Ǵ��㿪ʼ��! 
0, 1, 3, // ��һ�������� 
1, 2, 3 // �ڶ��������� 
};

float ratio = 0.5;

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
	unsigned int EBO;

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	//��VBO��VAO����
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	//Ϊ��ǰ�󶨵�target�Ļ��������󴴽�һ���µ����ݴ洢��
	//���data����NULL����ʹ�����Դ�ָ������ݳ�ʼ�����ݴ洢
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices),indices, GL_STATIC_DRAW);

	//��֪Shader��ν����������λ������ֵ
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);  //0��Ӧlayout(location = 0)
	//����VAO����ĵ�һ������ֵ
	glEnableVertexAttribArray(0);

	//��֪Shader��ν������������ɫ����ֵ
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	//����VAO����ĵ�һ������ֵ
	glEnableVertexAttribArray(1);

	//��֪Shader��ν������������������ֵ
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	//����VAO����ĵ�һ������ֵ
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	//���������
	unsigned int texture0, texture1;
	glGenTextures(1, &texture0);
	glBindTexture(GL_TEXTURE_2D, texture0);

	stbi_set_flip_vertically_on_load(true);
	// ���ز���������0
	int width, height, nrChannels;
	unsigned char* data = stbi_load("../../pics/container.jpg", &width, &height, &nrChannels, 0);
	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		// �����������/����ѡ��ڵ�ǰ�󶨵���������ϣ�
		float borderColor[] = { 1.0f, 1.0f, 0.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);//GL_LINEAR�������������Կ�����GL_NEAREST��һ���
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);//

	}
	else {
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);

	glGenTextures(1, &texture1);
	glBindTexture(GL_TEXTURE_2D, texture1);

	// ���ز���������1
	data = stbi_load("../../pics/awesomeface.png", &width, &height, &nrChannels, 0);
	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		// �����������/����ѡ��ڵ�ǰ�󶨵���������ϣ�
		float borderColor[] = { 1.0f, 1.0f, 0.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);//GL_LINEAR�������������Կ�����GL_NEAREST��һ���
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);//

	}
	else {
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);

	//OpenGL��֤������16������Ԫ��Ҳ����˵����Լ����GL_TEXTURE0��GL_TEXTRUE15�����Ƕ��ǰ�˳����ģ�GL_TEXTURE0 + 8���Ի��GL_TEXTURE8��
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texture1);
	ourShader.use();
	ourShader.setInt("texture0", 0);
	ourShader.setInt("texture1", 1);

	// ��Ⱦѭ��
	while (!glfwWindowShouldClose(window)) {

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::rotate(model, glm::radians(55.0f), glm::vec3(-1.0f, 0.0f, 0.0f));
		unsigned int modelLoc = glGetUniformLocation(ourShader.ID, "model");
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

		glm::mat4 view = glm::mat4(1.0f);
		// ��ע�⣬��������������Ҫ�ƶ��ķ����෴�ķ���ƽ�Ƴ���
		view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
		int viewLoc = glGetUniformLocation(ourShader.ID, "view");
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

		glm::mat4 projection;
		projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
		int projectionLoc = glGetUniformLocation(ourShader.ID, "projection");
		glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));


		processInput(window);
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f); //״̬����
		glClear(GL_COLOR_BUFFER_BIT); //״̬ʹ��

		ourShader.use();
		
		
		// ����uniformֵ
		float timeValue = glfwGetTime();
		float greenValue = sin(timeValue) / 2.0f + 0.5f;
		ourShader.setVec4("ourColor", 0.0f, greenValue, 0.0f, 1.0f);
		ourShader.setFloat("offsetX",0);
		ourShader.setFloat("ratio",ratio);

		glBindVertexArray(VAO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		// glfw: ��������������ѯIO�¼�����������/�ͷš�����ƶ��ȣ�
		//glDrawArrays(GL_TRIANGLES, 0, 3);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	// glfw: ����ǰ������GLFW�ȹ���Դ. 
	glfwTerminate();
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
	glDeleteProgram(ourShader.ID);

	return 0;
}

void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
		ratio = ratio >= 1 ? 1 : ratio + 0.001;
		std::cout << "key up ratio:" << ratio << std::endl;
	}
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
		ratio = ratio <= 0 ? 0 : ratio - 0.001;
		std::cout << "key down ratio:" << ratio << std::endl;
	}	
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}