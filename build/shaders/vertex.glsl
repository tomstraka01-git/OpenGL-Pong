#version 330 core
layout (location = 0) in vec3 aPos;

uniform float offsetX;
uniform float aspect;

void main() {
  
    float scaledX = aPos.x / aspect;
    
   
    gl_Position = vec4(scaledX + offsetX, aPos.y, aPos.z, 1.0);
}