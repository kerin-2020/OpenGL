#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "Renderer.h"

#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexArray.h"
#include "VertexBufferLayout.h"

//P11 Attribute set per vertex while the uniform is set per draw

//P12 vertex array: a way to bind vertex buffer with a certain kind of specification 
//for the layout of that actual vertex buffer
//change from{ bind shader--> vertex buffer--> set up the vetex layout(attribpoint)--> bind index buffer-
//--> issue the draw call}
//to         { bind shader--> vertex array-------------------------------------------> bind index buffer-
//--> issue the draw call}

//P4:基于你选定的Shader和Buffer去绘制某个三角形，OpenGL是一种状态机，意思是OpenGL不是一个对象
//instead of me just saying hey draw me a triangle and pass everything that opengl needs to 
//know to draw the that triangle it actually knows what it needs to do
//OpenGL work draw a triangle and base on which buffer and which shader you have selected, to determine
//what triangle get drawn and where and all of that stuff

//Vertex Attribute 和 shader
/*
Vertex Attribute:

opengl pipeline 的工作方式：提供图形类型给数据，然后存储在GPU上的内存里（内存里包含了我们想要绘制的所有数据）

然后我们使用shader（在GPU上执行的一种程序）读取这部分数据，然后在屏幕上显示下来

有代表性的是，我们实际上绘制图形使用的是一个Vertex buffer （存储在GPU上的一部分内存的 buffer ）

当shader 实际上开始读取 Vertex buffer 时，它需要知道 buffer 的布局（ buffer 里面有什么）

如果我们不说这个，它只是和c++的其他数据没什么两样



glVertexAttribPointer() 告诉OpenGLposition中的数据分布

stride: the amount of bytes between each vertex 12 for coordinate(index1), 8 for texture(index2), 12 for normal(index3)(bytes) so the stride is 32 bytes

pointer: 指向属性的指针 coordinate offset = 0 ,texture offset = 12, normal offset = 20

*/

/*
最常用的两种 shader ：

vertex shader OR fragment(pixel) shader

 data(CPU) -> GPU -> draw call -> shader

 Draw Call就是CPU调用图形编程接口，比如DirectX或OpenGL，来命令GPU进行渲染的操作。

 vertex shader: 告诉 OpenGL 你想要 vertex 出现在屏幕空间的何处

 fragment(pixel) shader : 栅格化，每个小三角形什么颜色

 记得 enable shader

*/

/*

uniform 实际上是一种从 CPU 中取数据到 shader 里的一种方法

*/


struct ShaderProgramSources
{
	std::string VertexSource;
	std::string FragmentSource;
};

static ShaderProgramSources ParseShader(const std::string& filepath)
{
	std::ifstream stream(filepath);

	enum class ShaderType
	{
		NONE = -1, VERTEX = 0, FRAGMENT = 1
	};

	std::string line;
	std::stringstream ss[2];
	ShaderType type = ShaderType::NONE;
	while (getline(stream, line))
	{
		if (line.find("#shader") != std::string::npos)
		{
			if (line.find("vertex") != std::string::npos)
				type = ShaderType::VERTEX;
			else if (line.find("fragment") != std::string::npos)
				type = ShaderType::FRAGMENT;
		}
		else
		{
			ss[(int)type] << line << '\n';
		}
	}

	return { ss[0].str(), ss[1].str() };
}

static unsigned int CompiledShader(unsigned int type, const std::string& source)
{
	unsigned int id = glCreateShader(type);
	const char* src = source.c_str();// &source[0]; ��ȷ source ָ�����һ����Ч�ĵ�ַ
	glShaderSource(id, 1, &src, nullptr);
	glCompileShader(id);

	//Error Handling
	int result;
	glGetShaderiv(id, GL_COMPILE_STATUS, &result);
	if (result == GL_FALSE)
	{
		int length;
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
		char* message = (char*)alloca(length * sizeof(char));//allocate on the stack
		glGetShaderInfoLog(id, length, &length, message);
		std::cout << "Failed to compiled "<< 
			(type==GL_VERTEX_SHADER?"Vertex":"Fragment") <<" Shader!" << std::endl;
		std::cout << message << std::endl;
		glDeleteShader(id);

		return 0;
	}

	return id;
}

// bind vertexShader & fragmentShader and link them into a single shader program
// get a buffer return an ID 
static unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader)
{
	unsigned int program = glCreateProgram();
	unsigned int vs = CompiledShader(GL_VERTEX_SHADER, vertexShader);
	unsigned int fs = CompiledShader(GL_FRAGMENT_SHADER, fragmentShader);

	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);
	glValidateProgram(program);

	glDeleteShader(vs);
	glDeleteShader(fs);

	return program;
}

int main(void)
{
	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	/*GLCall(glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3));
	GLCall(glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3));
	GLCall(glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE));*/
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);//GLFW_OPENGL_FORWARD_COMPAT是指定版本


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

	{

		float positions[] = {

			-0.5f, -0.5f, //0
			 0.5f, -0.5f, //1
			 0.5f,  0.5f, //2

			  //0.5f, 0.5f,
			 -0.5f, 0.5f, //3
			 //-0.5f,-0.5f
		};//vertex shader 会被调用三次

		unsigned int indices[] = {
			0,1,2,
			2,3,0
		};

		// vertex array 给 buffer 指定 layout 

		unsigned int vao;//vertex array object;
		GLCall(glGenVertexArrays(1, &vao));
		GLCall(glBindVertexArray(vao));

		VertexArray va;
		VertexBuffer vb(positions, 4 * 2 * sizeof(float));
		VertexBufferLayout layout;
		layout.Push<float>(2);//GLCall(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0));
		va.AddBuffer(vb, layout);
		

		//unsigned int buffer;
		//GLCall(glGenBuffers(1, &buffer));//create an ID :buffer

		////就像 PS 一样在指定层上画画
		//GLCall(glBindBuffer(GL_ARRAY_BUFFER, buffer));

		////specify the buffer
		//GLCall(glBufferData(GL_ARRAY_BUFFER, 4 * 2 * sizeof(float), positions, GL_STATIC_DRAW));

		//!!!!! REMEMBER!!!!!!!
		/*GLCall(glEnableVertexAttribArray(0));*/

		/* 执行此句的时候，0表示bind buffer和vao*/
		/*GLCall(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALS   E, sizeof(float) * 2, 0));*///2: component count

		IndexBuffer ib(indices, 6);

		//unsigned int ibo; // index buffer object
		//GLCall(glGenBuffers(1, &ibo));
		//GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo));
		//GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), indices, GL_STATIC_DRAW)); // indices 里有六个元素,必须是 unsigned 的
																			//stride 实际上是一个点（比如说一个2维点是两个顶点组成，所以在这里是两个 float 类型长度）
		////与上面一行联系，如何获取上面的数据？ L146in vec4 position;


		//std::string vertexShader =
		//	"#version 330 core\n"
		//	"\n"
		//	"layout(location = 0) in vec4 position;"// OpenGL��gl_Position��Ҫ��һ��4ά��vector
		//	"\n"
		//	"void mian()\n"
		//	"{\n"glEnableVertexAttribArra
		//	"	gl_Position = position;\n"
		//	"}\n";

		//std::string fragmentShader =
		//	"#version 330 core\n"
		//	"\n"
		//	"layout(location = 0) out vec4 color;"// OpenGL��gl_Position��Ҫ��һ��4ά��vector
		//	"\n"
		//	"void mian()\n"
		//	"{\n"
		//	"	color = vec4(0.0, 1.0, 0.0, 1.0);\n"
		//	"}\n";

		ShaderProgramSources source = ParseShader("res/shaders/Basic.shader");
		/*std::cout << "Vertex********" << std::endl;
		std::cout << source.VertexSource << std::endl;
		std::cout << "Fragment*******" << std::endl;
		std::cout << source.FragmentSource << std::endl;*/
		GLCall(unsigned int shader = CreateShader(source.VertexSource, source.FragmentSource));//这里有问题
		GLCall(glUseProgram(shader));//预测和显卡有关

		GLCall(int location = glGetUniformLocation(shader, "u_Color"));
		ASSERT(location != -1);
		GLCall(glUniform4f(location, 0.2f, 0.3f, 0.8f, 1.0f));

		//unbound everything
		GLCall(glBindVertexArray(0));
		GLCall(glUseProgram(0));
		GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
		GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));

		float r = 0.0f;
		float increment = 0.05f;

		/* Loop until the user closes the window */
		while (!glfwWindowShouldClose(window))
		{
			/* Render here */
			glClear(GL_COLOR_BUFFER_BIT);

			GLCall(glUseProgram(shader));
			GLCall(glUniform4f(location, r, 0.3f, 0.8f, 1.0f));

			/*有了vao以后，可以不用声明下面这些*/
			/*GLCall(glBindBuffer(GL_ARRAY_BUFFER, buffer));
			GLCall(glEnableVertexAttribArray(0));
			GLCall(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0));*/

			//只需要绑定vao和ibo
			//GLCall(glBindVertexArray(vao));
			/*GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo));*/
			va.Bind();
			ib.Bind();
			GLCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));

			if (r > 1.0f)
				increment = -0.05f;
			else if (r < 0.0f)
				increment = 0.05f;

			r += increment;


			/*glBegin(GL_TRIANGLES);
			glVertex2d(-0.5f, -0.5f);
			glVertex2d(0.0f, 0.5f);
			glVertex2d(0.5f, -0.5f);
			glEnd();*/

			//glDrawArrays(GL_TRIANGLES, 0, 6);

			//GLClearError();
			//Draw Call
			//GLCall(glDrawElements(GL_TRIANGLES, 6/* 代表indices里面6个index */, GL_UNSIGNED_INT, nullptr));//因为前面glBindBuffer已经创建绑定所以空指针
			// GL_UNSIGNED_INT 如果被写成了GL_INT，会导致黑屏
			//GLCheckError();
			//ASSERT(GLLogCall());



			/* Swap front and back buffers */
			glfwSwapBuffers(window);

			/* Poll for and process events */
			glfwPollEvents();
		}

		glDeleteProgram(shader);

	}

	glfwTerminate();
	return 0;
}
