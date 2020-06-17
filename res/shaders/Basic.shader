#shader vertex
#version 330 core

layout(location = 0) in vec4 position;

void mian()
{
	gl_Position = position;
};

#shader fragment
#version 330 core

layout(location = 0) out vec4 color;

void mian()
{
	color = vec4(1.0, 0.0, 0.0, 1.0);
};