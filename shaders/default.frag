#version 330 core//we want to use version 330 core
//since we are using opengl 3.3 core

out vec4 FragColor; //tell our shader to send out
//a vec4 called 'FragColor'

in vec4 color; //tell our shader to look for
//a vec4 called 'color' being sent out by something else

void main()
{
   FragColor = vec4(color);
}