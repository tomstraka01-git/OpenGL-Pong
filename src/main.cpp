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

    float vertices[] = {
    // first triangle 
    -0.5f,  0.2f, 0.0f,  // Vertex 1: Top-Left corner
    -0.5f, -0.2f, 0.0f,  // Vertex 2: Bottom-Left corner
     0.5f, -0.2f, 0.0f,  // Vertex 3: Bottom-Right corner

    //  second triangle
     0.5f, -0.2f, 0.0f,  // Vertex 4: Bottom-Right corner
     0.5f,  0.2f, 0.0f,  // Vertex 5: Top-Right corner
    -0.5f,  0.2f, 0.0f   // Vertex 6: Top-Left corner
    };

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);


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
    float offsetX = 0.0f;
    float aspect = (float)width / (float)height;

    // Game Loop
    while (!glfwWindowShouldClose(window)) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        float maxOffset = 1.0f - (0.5f / aspect);
        float minOffset = -1.0f + (0.5f / aspect);

        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
            offsetX -= 0.01f;
            if (offsetX < minOffset) offsetX = minOffset;
        }

        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
            offsetX += 0.01f;
            if (offsetX > maxOffset) offsetX = maxOffset;
        }

        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

       
        glUseProgram(shaderProgram);

        int offsetLocation = glGetUniformLocation(shaderProgram, "offsetX");

        int aspectLocation = glGetUniformLocation(shaderProgram, "aspect");

        
        glUniform1f(offsetLocation, offsetX);

        glUniform1f(aspectLocation, aspect);

        glBindVertexArray(VAO); 
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}