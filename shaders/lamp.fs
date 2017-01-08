#version 330 core
out vec4 color;

uniform vec3 lampColor;
uniform float alpha;

void main()
{
    color = vec4(lampColor,alpha);
}
