#version 330 core
layout (location = 0) in vec3 aPos;

uniform vec2 screenSize;
uniform vec2 pixelOffset;

void main() {
    vec2 currentPixel = aPos.xy + pixelOffset;

    float normalizedX = currentPixel.x / screenSize.x;
    float normalizedY = currentPixel.y / screenSize.y;

    float ndcX = normalizedX * 2.0 - 1.0;
    float ndcY = normalizedY * 2.0 - 1.0;

   
    float aspect = screenSize.x / screenSize.y;
    ndcX *= aspect; 
    
    gl_Position = vec4(ndcX, ndcY, aPos.z, 1.0);
}