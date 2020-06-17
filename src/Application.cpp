#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>

//基于你选定的Shader和Buffer去绘制某个三角形，OpenGL是一种状态机

//顶点属性 Vertex Attribute 和 shader
/*
Vertex Attribute:

opengl pipeline 的工作方式：提供图形类型给数据，然后存储在GPU上的内存里（内存里包含了我们想要绘制的所有数据）

然后我们使用shader（在GPU上执行的一种程序）读取这部分数据，然后在屏幕上显示下来

有代表性的是，我们实际上绘制图形使用的是一个Vertex buffer （存储在GPU上的一部分内存的 buffer ）

当shader 实际上开始读取 Vertex buffer 时，它需要知道 buffer 的布局（ buffer 里面有什么）

如果我们不说这个，它只是和c++的其他数据没什么两样



glVertexAttribPointer() 参数：

stride: the amount of bytes between each vertex 12 for coordinate(index1), 8 for texture(index2), 12 for normal(index3)(bytes) so the stride is 32 bytes

pointer: 指向属性的指针 coordinate offset = 0 ,texture offset = 12, normal offset = 20

*/

int main(void)
{
	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	if (glewInit() != GLEW_OK)
		std::cout << "Error!" << std::endl;
	//First you need to create a valid OpenGL rendering context and call glewInit() to initialize the extension entry points. 

	std::cout << glGetString(GL_VERSION) << std::endl;

	float positions[6] = {
		-0.5f, -0.5f,
		 0.0f,  0.5f,
		 0.5f, -0.5f
	};

	unsigned int buffer;
	glGenBuffers(1, &buffer);//create an ID :buffer

	//就像 PS 一样在指定层上画画
	glBindBuffer(GL_ARRAY_BUFFER, buffer);

	//specify the buffer
	glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(float), positions, GL_STATIC_DRAW);

	//!!!!! REMEMBER!!!!!!!
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float)*2, 0);//2: component count
															//stride 实际上是一个点（比如说一个2维点是两个顶点组成，所以在这里是两个 float 类型长度）

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		/* Render here */
		glClear(GL_COLOR_BUFFER_BIT);

		/*glBegin(GL_TRIANGLES);
		glVertex2d(-0.5f, -0.5f);
		glVertex2d(0.0f, 0.5f);
		glVertex2d(0.5f, -0.5f);
		glEnd();*/
		glDrawArrays(GL_TRIANGLES, 0, 3);//从buffer里第零组开始draw，一共3组点

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}