#version 330 core
layout (location = 0) in vec3 aPos;

uniform vec2 screenSize;
uniform vec2 pixelOffset;


void main() {
  
    vec2 currentPixel = aPos.xy + pixelOffset;

    float ndcX = (currentPixel.x / screenSize.x) * 2.0 - 1.0;
    float ndcY = (currentPixel.y / screenSize.y) * 2.0 - 1.0;

    gl_Position = vec4(ndcX, ndcY, aPos.z, 1.0);
    
}