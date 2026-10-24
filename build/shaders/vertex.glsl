#version 330 core
layout (location = 0) in vec3 aPos;

uniform vec2 screenSize;
uniform vec2 pixelOffset;

void main() {
    vec2 currentPixel = aPos.xy + pixelOffset;

    // Use screenSize.y for BOTH X and Y so pixels stay perfectly square
    float ndcX = (currentPixel.x / screenSize.y) * 2.0 - (screenSize.x / screenSize.y);
    float ndcY = (currentPixel.y / screenSize.y) * 2.0 - 1.0;

    // Flip Y if your origin (0,0) is at the top-left of the screen
    ndcY = -ndcY;

    gl_Position = vec4(ndcX, ndcY, aPos.z, 1.0);
}