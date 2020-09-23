#version 100

attribute vec4 a_position;
uniform mat4 mvp_matrix;
uniform vec4 color;
varying vec4 fcolor;

void main()
{
    gl_Position = mvp_matrix * a_position;
    fcolor = color;
}
