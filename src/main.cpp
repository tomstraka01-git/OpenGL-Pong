#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>


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



void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

int width = 800;

int height = 600;

int main() {
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
    

    // Debugging 
    int success;

    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);

    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << "\n";

    }

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);

    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << "\n";

    }

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << "\n";
    }

    // X movement offset for uniform
    float offsetX = 450.0f;
    float offsetY = 100.0f;

    float ballX = 450.0f;
    float ballY = 400.0f;
    
    float ballVelX = 5.0f;
    float ballVelY = 5.0f;
    float ballHalfSize = 10.0f;

    // Game Loop
    while (!glfwWindowShouldClose(window)) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        // Get live framebuffer dimensions every frame
        int winWidth, winHeight;
        glfwGetWindowSize(window, &winWidth, &winHeight); // Will return 800 and 600
        float currentWidth = static_cast<float>(winWidth);
        float currentHeight = static_cast<float>(winHeight);

        float paddleHalfWidth = 160.0f; 

        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
            offsetX -= 5.0f;
            
            if (offsetX - paddleHalfWidth < 0.0f) {
                offsetX = paddleHalfWidth;
            }
        }

        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
            offsetX += 5.0f;
      
            if (offsetX + paddleHalfWidth > currentWidth) {
                offsetX = currentWidth - paddleHalfWidth;
            }
        }

        // Ball movement

        ballX += ballVelX;
        ballY += ballVelY;

        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);

        // Pass the screen size uniform once per frame
        glUniform2f(glGetUniformLocation(shaderProgram, "screenSize"), currentWidth, currentHeight);

        // draw paddle
        glBindVertexArray(paddleVAO); 
        glUniform2f(glGetUniformLocation(shaderProgram, "pixelOffset"), offsetX, offsetY);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // draw ball
        glBindVertexArray(ballVAO); 
        glUniform2f(glGetUniformLocation(shaderProgram, "pixelOffset"), ballX, ballY);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Clean up vbo and vao
    glDeleteVertexArrays(1, &paddleVAO);
    glDeleteBuffers(1, &paddleVBO);
    glDeleteVertexArrays(1, &ballVAO);
    glDeleteBuffers(1, &ballVBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}