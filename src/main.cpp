#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <random>
#include <cmath>


#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"


enum class GameState {
    game,
    gameOver
};


std::string loadShaderSource(const std::string& filepath) {
    std::ifstream file;
    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
        file.open(filepath);
        std::stringstream stream;
        stream << file.rdbuf();
        file.close();
        return stream.str();
    }

    catch (std::ifstream::failure& e) {
        std::cerr << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ: " << filepath << "\n";
        return "";
    }
}


struct TextVertex {
    float x, y, s, t;
};

void renderText(unsigned int textShaderProgram, unsigned int fontTexture, unsigned int textVAO, unsigned int textVBO, 
                float x, float y, std::string text, float scale, const stbtt_bakedchar* cdata, float screenWidth, float screenHeight) {
    
    glUseProgram(textShaderProgram);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, fontTexture);
    glUniform1i(glGetUniformLocation(textShaderProgram, "tex"), 0);

    // Pass screen dimensions for projection inside shader or handle via pixel coords
    glUniform2f(glGetUniformLocation(textShaderProgram, "screenSize"), screenWidth, screenHeight);

    std::vector<TextVertex> vertices;

    float startX = x;
    float startY = y;

    for (char c : text) {
        if (c >= 32 && c < 128) {
            stbtt_aligned_quad q;
            stbtt_GetBakedQuad(cdata, 512, 512, c - 32, &x, &y, &q, 1); // 1 = opengl coordinate scale

            // Convert stbtt_aligned_quad into triangles (2 triangles per character = 6 vertices)
            vertices.push_back({q.x0, q.y0, q.s0, q.t0});
            vertices.push_back({q.x1, q.y0, q.s1, q.t0});
            vertices.push_back({q.x1, q.y1, q.s1, q.t1});

            vertices.push_back({q.x0, q.y0, q.s0, q.t0});
            vertices.push_back({q.x1, q.y1, q.s1, q.t1});
            vertices.push_back({q.x0, q.y1, q.s0, q.t1});
        }
    }

    glBindVertexArray(textVAO);
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(TextVertex), vertices.data(), GL_DYNAMIC_DRAW);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertices.size()));

    glDisable(GL_BLEND);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}





void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

int width = 800;

int height = 600;
int main() {
    GameState state = GameState::game;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> distrib(-0.5f, 0.5f);

    std::uniform_real_distribution<float> colorDistrib(0.0f, 1.0f);

    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(width, height, "OpenGL Pong", NULL, NULL);
    if (window == NULL) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    glViewport(0, 0, fbWidth, fbHeight);

    // STB Truetype Font Setup 
    unsigned char ttf_buffer[1 << 20];
    unsigned char temp_bitmap[512 * 512];
    stbtt_bakedchar cdata[96]; 
    unsigned int fontTexture = 0;

    std::ifstream fontFile("/home/tom/Desktop/Projects/OpenGL-Pong/Roboto-VariableFont_wdth,wght.ttf", std::ios::binary);
    if (fontFile.is_open()) {
        fontFile.read((char*)ttf_buffer, sizeof(ttf_buffer));
        fontFile.close();

        stbtt_BakeFontBitmap(ttf_buffer, 0, 32.0f, temp_bitmap, 512, 512, 32, 96, cdata);

        glGenTextures(1, &fontTexture);
        glBindTexture(GL_TEXTURE_2D, fontTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 512, 512, 0, GL_RED, GL_UNSIGNED_BYTE, temp_bitmap);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);
    } else {
        std::cerr << "WARNING: Could not open ttf! Text rendering will not appear.\n";
    }

    // Shaders for game objects
    std::string vertexCode = loadShaderSource("shaders/vertex.glsl");
    std::string fragmentCode = loadShaderSource("shaders/fragment.glsl");
    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();
  
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vShaderCode, NULL);
    glCompileShader(vertexShader);
    
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fShaderCode, NULL);
    glCompileShader(fragmentShader);

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Simple text shader source strings 
    std::string textVertSource = 
        "#version 330 core\n"
        "layout (location = 0) in vec4 vertex;\n" // vec2 pos, vec2 tex
        "out vec2 TexCoords;\n"
        "uniform vec2 screenSize;\n"
        "void main() {\n"
        "   TexCoords = vertex.zw;\n"
        "   // Transform from pixel coordinates to NDC [-1, 1]\n"
        "   float x = (vertex.x / screenSize.x) * 2.0 - 1.0;\n"
        "   float y = 1.0 - (vertex.y / screenSize.y) * 2.0;\n"
        "   gl_Position = vec4(x, y, 0.0, 1.0);\n"
        "}\n";

    std::string textFragSource = 
        "#version 330 core\n"
        "in vec2 TexCoords;\n"
        "out vec4 color;\n"
        "uniform sampler2D tex;\n"
        "void main() {\n"
        "   float alpha = texture(tex, TexCoords).r;\n"
        "   color = vec4(1.0, 1.0, 1.0, alpha);\n" // White text
        "}\n";

    unsigned int textVShader = glCreateShader(GL_VERTEX_SHADER);
    const char* tvSrc = textVertSource.c_str();
    glShaderSource(textVShader, 1, &tvSrc, NULL);
    glCompileShader(textVShader);

    unsigned int textFShader = glCreateShader(GL_FRAGMENT_SHADER);
    const char* tfSrc = textFragSource.c_str();
    glShaderSource(textFShader, 1, &tfSrc, NULL);
    glCompileShader(textFShader);

    unsigned int textShaderProgram = glCreateProgram();
    glAttachShader(textShaderProgram, textVShader);
    glAttachShader(textShaderProgram, textFShader);
    glLinkProgram(textShaderProgram);
    glDeleteShader(textVShader);
    glDeleteShader(textFShader);

    // Text VAO/VBO setup
    unsigned int textVAO, textVBO;
    glGenVertexArrays(1, &textVAO);
    glGenBuffers(1, &textVBO);
    glBindVertexArray(textVAO);
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(TextVertex) * 6, NULL, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // 1. Paddle Shape 
    float paddleVertices[] = {
        -60.0f,  10.0f, 0.0f,
        -60.0f, -10.0f, 0.0f,
         60.0f, -10.0f, 0.0f,

         60.0f, -10.0f, 0.0f,
         60.0f,  10.0f, 0.0f,
        -60.0f,  10.0f, 0.0f
    };

    // 2. Ball Shape 
    float ballVertices[] = {
        -10.0f,  10.0f, 0.0f,  
        -10.0f, -10.0f, 0.0f,  
         10.0f, -10.0f, 0.0f,  

         10.0f, -10.0f, 0.0f,  
         10.0f,  10.0f, 0.0f,  
        -10.0f,  10.0f, 0.0f   
    };

    // Paddle VBO and VAO
    unsigned int paddleVAO, paddleVBO;
    glGenVertexArrays(1, &paddleVAO);
    glGenBuffers(1, &paddleVBO);
    glBindVertexArray(paddleVAO);
    glBindBuffer(GL_ARRAY_BUFFER, paddleVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(paddleVertices), paddleVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Ball VAO and VBO
    unsigned int ballVAO, ballVBO;
    glGenVertexArrays(1, &ballVAO);
    glGenBuffers(1, &ballVBO);
    glBindVertexArray(ballVAO);
    glBindBuffer(GL_ARRAY_BUFFER, ballVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(ballVertices), ballVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glBindVertexArray(0);

    float offsetX = 450.0f;
    float offsetY = 500.0f;

    float ballX = 450.0f;
    float ballY = 400.0f;
    

    float baseSpeed = 7.0f;

    float targetSpeed = 7.0f;

    // Create a distribution for a random angle between 45 and 135
    std::uniform_real_distribution<float> angleDist(0.785398f, 2.356194f);

    float ballVelX = 5.0f;
    float ballVelY = 5.0f;
    float ballHalfSize = 10.0f;


    float offsetSpeed = 5.0f;

    int scoreInt = 0;

    std::string scoreText = "0";

    int highScoreInt = 0;

    std::string highScoreText = "0";
    // Game Loop
    
    float colorA = 0.5f;
    float colorB = 0.5f;
    float colorC = 0.5f;

    float randomAngle = angleDist(gen);

    ballVelX = baseSpeed * std::cos(randomAngle);
    ballVelY = baseSpeed * std::sin(randomAngle);

    while (!glfwWindowShouldClose(window)) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        int winWidth, winHeight;
        glfwGetWindowSize(window, &winWidth, &winHeight);
        float currentWidth = static_cast<float>(winWidth);
        float currentHeight = static_cast<float>(winHeight);

        float paddleHalfWidth = 160.0f; 

        

        

        if (state == GameState::game) {
            if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
                offsetX -= offsetSpeed;
                if (offsetX - paddleHalfWidth < 0.0f) {
                    offsetX = paddleHalfWidth;
                }
            }

            if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
                offsetX += offsetSpeed;
                
                if (offsetX + paddleHalfWidth > currentWidth) {
                    offsetX = currentWidth - paddleHalfWidth;
                }
            }

            // Ball movement
            ballX += ballVelX;
            ballY += ballVelY;

            if ((ballX + ballHalfSize) > currentWidth - 100 || (ballX - ballHalfSize) < 100) {
                ballVelX = -ballVelX;
                float random_num = distrib(gen);
                ballVelX += random_num;
                ballVelY += random_num;
            }

            if ((ballY - ballHalfSize) < 0) {
                ballVelY = -ballVelY;
                float random_num = distrib(gen);
                ballVelX += random_num;
                ballVelY += random_num;
            }
            
            
            float paddleLeft   = offsetX - 60.0f;
            float paddleRight  = offsetX + 60.0f;
            float paddleBottom = offsetY - 10.0f;
            float paddleTop    = offsetY + 10.0f;

            float ballLeft   = ballX - ballHalfSize;
            float ballRight  = ballX + ballHalfSize;
            float ballBottom = ballY - ballHalfSize;
            float ballTop    = ballY + ballHalfSize;

            if ((ballRight >= paddleLeft) && (ballLeft <= paddleRight) &&
            (ballTop >= paddleBottom) && (ballBottom <= paddleTop)) {

                if (ballVelY > 0) {
                    ballVelY = -ballVelY; // Reverse vertical direction (now negative)
                    ballY = paddleBottom - ballHalfSize - 0.1f;

                    scoreInt++;
                    scoreText = std::to_string(scoreInt);

                    // 1. Calculate target ball speed cleanly with a hard cap (e.g., max 14.0f)
                    float targetBallSpeed = std::min(baseSpeed + (scoreInt * 0.35f), 14.0f);

                    // 2. Recalculate ball velocity maintaining direction
                    ballVelX += distrib(gen); // Small random variation
                    float currentSpeed = std::hypot(ballVelX, ballVelY);
                    
                    colorA = colorDistrib(gen);
                    colorB = colorDistrib(gen);
                    colorC = colorDistrib(gen);

                    if (currentSpeed > 0.0001f) {
                        ballVelX = (ballVelX / currentSpeed) * targetBallSpeed;
                        ballVelY = (ballVelY / currentSpeed) * targetBallSpeed;
                    }

                    // 3. Scale paddle speed cleanly with a hard cap (e.g., max 11.0f)
                    offsetSpeed = std::min(5.0f + (scoreInt * 0.25f), 11.0f);

                    if (scoreInt > highScoreInt) {
                        highScoreInt = scoreInt;
                        highScoreText = std::to_string(highScoreInt);
                    }
                }
            }
            
            
            if ((ballY + ballHalfSize) > currentHeight) {
                state = GameState::gameOver;
            }
        } 

        // Render pass
        if (state == GameState::game) {
            glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            
            glUseProgram(shaderProgram);
            glUniform2f(glGetUniformLocation(shaderProgram, "screenSize"), currentWidth, currentHeight);

            int colorLoc = glGetUniformLocation(shaderProgram, "objectColor");




            glUniform4f(colorLoc, colorA, colorB, colorC, 1.0f);
            // Draw paddle
            glBindVertexArray(paddleVAO); 
            glUniform2f(glGetUniformLocation(shaderProgram, "pixelOffset"), offsetX, offsetY);
            glDrawArrays(GL_TRIANGLES, 0, 6);

            // Draw ball
            glBindVertexArray(ballVAO); 
            glUniform2f(glGetUniformLocation(shaderProgram, "pixelOffset"), ballX, ballY);
            glDrawArrays(GL_TRIANGLES, 0, 6);

            renderText(textShaderProgram, fontTexture, textVAO, textVBO, 25.0f, 50.0f, 
            "Score:" + scoreText, 1.5f, cdata, currentWidth, currentHeight);

            renderText(textShaderProgram, fontTexture, textVAO, textVBO, 25.0f, 90.0f, 
            "High score:" + highScoreText, 1.5f, cdata, currentWidth, currentHeight);
            
        } 
        else if (state == GameState::gameOver) {
            glClearColor(0.3f, 0.05f, 0.05f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            

            
            // Render Game Over text
            renderText(textShaderProgram, fontTexture, textVAO, textVBO, 
                       currentWidth / 2.0f - 110.0f, currentHeight / 2.0f - 20.0f, 
                       "GAME OVER", 1.0f, cdata, currentWidth, currentHeight);

            renderText(textShaderProgram, fontTexture, textVAO, textVBO, 
                       currentWidth / 2.0f - 140.0f, currentHeight / 2.0f + 30.0f, 
                       "PRESS R TO RESTART", 1.0f, cdata, currentWidth, currentHeight);
            
                       renderText(textShaderProgram, fontTexture, textVAO, textVBO, 
                       currentWidth / 2.0f - 70.0f, currentHeight / 2.0f - 60.0f, 
                       "Score:" + scoreText, 1.0f, cdata, currentWidth, currentHeight);
                


            if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
                offsetX = 450.0f;
                offsetY = 500.0f;
                ballX = 450.0f;
                ballY = 400.0f; 


                float randomAngle = angleDist(gen);
                ballVelX = baseSpeed * std::cos(randomAngle);
                ballVelY = baseSpeed * std::sin(randomAngle);

                offsetSpeed = 5.0f;

                state = GameState::game;
                scoreInt = 0;
                scoreText = "0";
            }
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Clean up
    glDeleteVertexArrays(1, &paddleVAO);
    glDeleteBuffers(1, &paddleVBO);
    glDeleteVertexArrays(1, &ballVAO);
    glDeleteBuffers(1, &ballVBO);
    glDeleteVertexArrays(1, &textVAO);
    glDeleteBuffers(1, &textVBO);
    glDeleteProgram(shaderProgram);
    glDeleteProgram(textShaderProgram);

    glfwTerminate();
    return 0;
}