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
// 位置              // 纹理坐标
-0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
 0.5f, -0.5f, -0.5f, 1.0f, 0.0f,
 0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
 0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
-0.5f, 0.5f, -0.5f, 0.0f, 1.0f,
-0.5f, -0.5f, -0.5f, 0.0f, 0.0f,

-0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
-0.5f, 0.5f, 0.5f, 0.0f, 1.0f,
-0.5f, -0.5f, 0.5f, 0.0f, 0.0f,

-0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
-0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
-0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
-0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
-0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
-0.5f, 0.5f, 0.5f, 1.0f, 0.0f,

 0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
 0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
 0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
 0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
 0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
 0.5f, 0.5f, 0.5f, 1.0f, 0.0f,

-0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
 0.5f, -0.5f, -0.5f, 1.0f, 1.0f,
 0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
 0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
-0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
-0.5f, -0.5f, -0.5f, 0.0f, 1.0f,

-0.5f, 0.5f, -0.5f, 0.0f, 1.0f,
 0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
 0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
 0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
-0.5f, 0.5f, 0.5f, 0.0f, 0.0f,
-0.5f, 0.5f, -0.5f, 0.0f, 1.0f

};

glm::vec3 cubePositions[] = {
glm::vec3(0.0f, 0.0f, 0.0f),
glm::vec3(2.0f, 5.0f, -15.0f),
glm::vec3(-1.5f, -2.2f, -2.5f),
glm::vec3(-3.8f, -2.0f, -12.3f),
glm::vec3(2.4f, -0.4f, -3.5f),
glm::vec3(-1.7f, 3.0f, -7.5f),
glm::vec3(1.3f, -2.0f, -2.5f),
glm::vec3(1.5f, 2.0f, -2.5f),
glm::vec3(1.5f, 0.2f, -1.5f),
glm::vec3(-1.3f, 1.0f, -1.5f)
};


//unsigned int indices[] = { // 注意，我们从零开始算! 
//0, 1, 3, // 第一个三角形 
//1, 2, 3 // 第二个三角形 
//};

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
	//GLFW将窗口的上下文设置为当前线程的上下文
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	//GLAD
	// glad: 加载所有OpenGL函数指针
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	Shader ourShader("shaders/shader.vs","shaders/shader.fs");


	//创建VBO和VAO对象，并赋予ID
	unsigned int VBO, VAO;
	unsigned int EBO;

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	//glGenBuffers(1, &EBO);
	//绑定VBO和VAO对象
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	//为当前绑定到target的缓冲区对象创建一个新的数据存储。
	//如果data不是NULL，则使用来自此指针的数据初始化数据存储
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices),indices, GL_STATIC_DRAW);

	//告知Shader如何解析缓冲里的位置属性值  //0对应location0
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);  //0对应layout(location = 0)
	//开启VAO管理的第一个属性值
	glEnableVertexAttribArray(0);

	//告知Shader如何解析缓冲里的纹理属性值
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	//开启VAO管理的第一个属性值
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	//绑定纹理对象
	unsigned int texture0, texture1;
	glGenTextures(1, &texture0);
	glBindTexture(GL_TEXTURE_2D, texture0);

	stbi_set_flip_vertically_on_load(true);
	// 加载并生成纹理0
	int width, height, nrChannels;
	unsigned char* data = stbi_load("../../pics/container.jpg", &width, &height, &nrChannels, 0);
	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		// 设置纹理包裹/过滤选项（在当前绑定的纹理对象上）
		float borderColor[] = { 1.0f, 1.0f, 0.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);//GL_LINEAR大多数更清晰相对开销比GL_NEAREST大一点点
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);//

	}
	else {
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);

	glGenTextures(1, &texture1);
	glBindTexture(GL_TEXTURE_2D, texture1);

	// 加载并生成纹理1
	data = stbi_load("../../pics/awesomeface.png", &width, &height, &nrChannels, 0);
	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		// 设置纹理包裹/过滤选项（在当前绑定的纹理对象上）
		float borderColor[] = { 1.0f, 1.0f, 0.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);//GL_LINEAR大多数更清晰相对开销比GL_NEAREST大一点点
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);//

	}
	else {
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);

	//OpenGL保证至少有16个纹理单元，也就是说你可以激活从GL_TEXTURE0到GL_TEXTRUE15。它们都是按顺序定义的，GL_TEXTURE0 + 8可以获得GL_TEXTURE8。
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texture1);
	ourShader.use();
	ourShader.setInt("texture0", 0);
	ourShader.setInt("texture1", 1);

	//glm::mat4 view = glm::mat4(1.0f);
	////请注意，我们正在以与想要移动的方向相反的方向平移场景
	//				  //照相机位置                 //看向物体的位置             //照相机向上方向     
	//view = glm::lookAt(glm::vec3(0.0f, 0.0f, 15.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	//ourShader.setMat4("view", view);
	// 渲染循环
	while (!glfwWindowShouldClose(window)) {

		const float radius = 3.0f;
		float camX = sin(glfwGetTime()) * radius;
		float camZ = cos(glfwGetTime()) * radius;
		glm::mat4 view;
		                  //照相机位置                 //看向物体的位置             //照相机向上方向     
		view = glm::lookAt(glm::vec3(camX, 0.0, camZ), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
		ourShader.setMat4("view", view);

		glm::mat4 projection;
					      //垂直方向视角范围    //视口宽高比      //最近绘制距离  //最远绘制距离
		projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f          , 100.0f);
		int projectionLoc = glGetUniformLocation(ourShader.ID, "projection");
		glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));


		processInput(window);
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f); //状态设置
		glEnable(GL_DEPTH_TEST);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		ourShader.use();

		ourShader.setFloat("ratio", ratio);
		glBindVertexArray(VAO);
		for (unsigned int i = 0; i < 10; i++) {
			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, cubePositions[i]);

			float angle = 20.0f * i;
			if ((i+1)%3 == 0) {
				model = glm::rotate(model, (float)glfwGetTime() * glm::radians(55.0f), glm::vec3(-1.0f, 1.0f, 0.0f));
			}
			ourShader.setMat4("model", model);
			//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		// glfw: 交换缓冲区和轮询IO事件（按键按下/释放、鼠标移动等）
			glDrawArrays(GL_TRIANGLES, 0, 36);
			//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		}

		
		
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	// glfw: 回收前面分配的GLFW先关资源. 
	glfwTerminate();
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	//glDeleteBuffers(1, &EBO);
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
