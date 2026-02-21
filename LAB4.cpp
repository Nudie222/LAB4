#define _CRT_SECURE_NO_WARNINGS
#define GLEW_STATIC
#define GLFW_DLL

#include <iostream>
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

GLuint load_shader_from_file(const char* file_path, GLenum shader_type) {
    FILE* file = fopen(file_path, "r");
    if (!file) {
        fprintf(stderr, "ERROR: Could not open shader file: %s\n", file_path);
        return 0;
    }
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    if (length <= 0) {
        fclose(file);
        return 0;
    }
    char* shader_code = new char[length + 1];
    size_t read = fread(shader_code, 1, length, file);
    shader_code[read] = '\0';
    fclose(file);
    GLuint shader = glCreateShader(shader_type);
    glShaderSource(shader, 1, &shader_code, NULL);
    glCompileShader(shader);
    delete[] shader_code;
    GLint success;
    GLchar infoLog[1024];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 1024, NULL, infoLog);
        fprintf(stderr, "ERROR::SHADER_COMPILATION_ERROR\n%s\n", infoLog);
        return 0;
    }
    return shader;
}

GLuint create_shader_program(const char* vertex_path, const char* fragment_path) {
    GLuint vertex_shader = load_shader_from_file(vertex_path, GL_VERTEX_SHADER);
    GLuint fragment_shader = load_shader_from_file(fragment_path, GL_FRAGMENT_SHADER);
    if (vertex_shader == 0 || fragment_shader == 0) return 0;
    GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    GLint success;
    GLchar infoLog[1024];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 1024, NULL, infoLog);
        fprintf(stderr, "ERROR::PROGRAM_LINKING_ERROR\n%s\n", infoLog);
        return 0;
    }
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    return program;
}

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
float yaw = -90.0f;
float pitch = 0.0f;
float lastX = 512.0f / 2.0f;
float lastY = 512.0f / 2.0f;
bool firstMouse = true;
bool fullscreen = false;
int windowWidth = 512;
int windowHeight = 512;

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = (float)xpos;
        lastY = (float)ypos;
        firstMouse = false;
    }

    float xoffset = (float)xpos - lastX;
    float yoffset = (float)ypos - lastY;

    lastX = (float)xpos;
    lastY = (float)ypos;

    float sensitivity = 0.05f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

void toggle_fullscreen(GLFWwindow* window) {
    fullscreen = !fullscreen;
    if (fullscreen) {
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
        windowWidth = mode->width;
        windowHeight = mode->height;
    }
    else {
        glfwSetWindowMonitor(window, NULL, 100, 100, 512, 512, GLFW_DONT_CARE);
        windowWidth = 512;
        windowHeight = 512;
    }
    lastX = windowWidth / 2.0f;
    lastY = windowHeight / 2.0f;
    firstMouse = true;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    windowWidth = width;
    windowHeight = height;
    glViewport(0, 0, width, height);
}

int main() {
    if (!glfwInit()) {
        fprintf(stderr, "ERROR: could not start GLFW3.\n");
        return 1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "Lab 4: Camera (ESC-exit, F11-fullscreen)", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return -1;
    }

    printf("OpenGL version: %s\n", glGetString(GL_VERSION));
    printf("OpenGL device: %s\n", glGetString(GL_RENDERER));
    printf("Controls: WASD-move, Mouse-look, ESC-exit, F11-fullscreen\n");

    GLuint shader_program = create_shader_program("vertex_shader.glsl", "fragment_shader.glsl");
    if (shader_program == 0) return -1;

    float vertices[] = {
         0.5f,  0.0f, 0.0f,
         0.25f, 0.433f, 0.0f,
        -0.25f, 0.433f, 0.0f,
        -0.5f,  0.0f, 0.0f,
        -0.25f,-0.433f, 0.0f,
         0.25f,-0.433f, 0.0f
    };

    GLuint indices[] = { 0, 1, 2, 0, 2, 3, 0, 3, 4, 0, 4, 5 };

    GLuint VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);


    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    const float areaLimit = 5.0f;
    float aspectRatio = (float)windowWidth / (float)windowHeight;

    while (!glfwWindowShouldClose(window)) {
  
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Выход по ESC
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, true);
        }

        // Полный экран по F11
        if (glfwGetKey(window, GLFW_KEY_F11) == GLFW_PRESS) {
            static bool keyPressed = false;
            if (!keyPressed) {
                toggle_fullscreen(window);
                keyPressed = true;
            }
        }
        else {
            static bool keyPressed = false;
            keyPressed = false;
        }

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(shader_program);

       
        glm::vec3 newPos = cameraPos;
        const float cameraSpeed = 2.0f; 

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            newPos += cameraSpeed * deltaTime * cameraFront;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            newPos -= cameraSpeed * deltaTime * cameraFront;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            newPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed * deltaTime;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            newPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed * deltaTime;

  
        if (newPos.x > -areaLimit && newPos.x < areaLimit &&
            newPos.y > -areaLimit && newPos.y < areaLimit &&
            newPos.z > -areaLimit && newPos.z < areaLimit) {
            cameraPos = newPos;
        }

        aspectRatio = (float)windowWidth / (float)windowHeight;


        glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, (float)glfwGetTime() * glm::radians(50.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        GLint projLoc = glGetUniformLocation(shader_program, "projection");
        GLint viewLoc = glGetUniformLocation(shader_program, "view");
        GLint modelLoc = glGetUniformLocation(shader_program, "model");
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        float timeValue = (float)glfwGetTime();
        float red = sin(timeValue) * 0.5f + 0.5f;
        float green = sin(timeValue * 1.5f) * 0.5f + 0.5f;
        float blue = cos(timeValue * 0.7f) * 0.5f + 0.5f;
        GLint colorLocation = glGetUniformLocation(shader_program, "ourColor");
        glUniform4f(colorLocation, red, green, blue, 1.0f);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shader_program);
    glfwTerminate();
    return 0;
}
