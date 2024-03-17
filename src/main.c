#include <stdio.h>
#include <stdlib.h>

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <cglm/vec3.h>

#include "io.h"
#include "log.h"
#include "shader.h"

#define WINDOW_WIDTH 900
#define WINDOW_HEIGHT 600

#define KEEP(__X) ((void)__X)

#define SHADER_VERT_PATH "./assets/shaders/basic.vert"
#define SHADER_FRAG_PATH "./assets/shaders/basic.frag"

static float VERTICES[] = {
    -0.5f, 0.5f,  0.0f, //
    -0.5f, -0.5f, 0.0f, //
    0.5f,  0.5f,  0.0f, //
    0.5f,  0.5f,  0.0f, //
    0.5f,  -0.5f, 0.0f, //
    -0.5f, -0.5f, 0.0f, //
};

static void
GLADDebugMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message,
                         const void *userParam) {
    KEEP(source);
    KEEP(id);
    KEEP(length);
    KEEP(userParam);

    LOG_ERROR("GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
              (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), type, severity, message);
}

static void
GLFWErrorCallback(int code, const char *description) {
    LOG_ERROR("[GLFW] (%i) %s\n", code, description);
}

static void
GLFWFrameBufferSizeCallback(GLFWwindow *, int width, int height) {
    glViewport(0, 0, width, height);
}

int
main(void) {
    if (glfwInit() != GLFW_TRUE) {
        LOG_ERROR("Failed to initialize GLFW\n");
        return EXIT_FAILURE;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Breakout", nullptr, nullptr);
    glfwSetErrorCallback(GLFWErrorCallback);

    int major, minor, revision;
    glfwGetVersion(&major, &minor, &revision);
    LOG_INFO("Running against GLFW %i.%i.%i\n", major, minor, revision);

    if (window == nullptr) {
        LOG_ERROR("Failed to initialize GLFW window\n");
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        LOG_ERROR("Failed to initialize GLAD\n");
        glfwDestroyWindow(window);
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

#if defined(BUILD_CONFIG_DEBUG)
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(GLADDebugMessageCallback, nullptr);
#endif

    glfwSetFramebufferSizeCallback(window, GLFWFrameBufferSizeCallback);

    LOG_INFO("Successfully initialized!\n");

    GLuint program = glCreateProgram();

    {
        Shader vertShader = 0;
        ShaderCompileStatus vertCompStatus = ShaderLoadAndCompile(SHADER_VERT_PATH, SHADER_TYPE_VERT, &vertShader);

        if (vertCompStatus != SHADER_COMPILE_OK) {
            LOG_ERROR("Failed to load vertex shader source: rc: %hi\n", vertCompStatus);
            glfwDestroyWindow(window);
            glfwTerminate();
            return EXIT_FAILURE;
        }

        Shader fragShader = 0;
        ShaderCompileStatus fragCompStatus = ShaderLoadAndCompile(SHADER_FRAG_PATH, SHADER_TYPE_FRAG, &fragShader);

        if (fragCompStatus != SHADER_COMPILE_OK) {
            LOG_ERROR("Failed to load vertex shader source: rc: %hi\n", fragCompStatus);
            glfwDestroyWindow(window);
            glfwTerminate();
            return EXIT_FAILURE;
        }

        glAttachShader(program, vertShader);
        glAttachShader(program, fragShader);
        glLinkProgram(program);

        {
            int success;
            char infoLog[GL_INFO_LOG_LENGTH];
            glGetProgramiv(program, GL_LINK_STATUS, &success);
            if (!success) {
                glGetProgramInfoLog(program, sizeof(infoLog), nullptr, infoLog);
                LOG_ERROR("Failed to link shader program: %s\n", infoLog);
                glfwDestroyWindow(window);
                glfwTerminate();
                return EXIT_FAILURE;
            }
        }

        glDeleteShader(vertShader);
        glDeleteShader(fragShader);
    }

    // NOTE(gr3yknigh1):
    //     * Vertex Array Object
    //     * Vertex Buffer Object
    GLuint vao, vbo;

    {
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
    }

    {
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(VERTICES), VERTICES, GL_STATIC_DRAW);
    }

    {
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
        glEnableVertexAttribArray(0);
    }

#if 0
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
#endif

    while (!glfwWindowShouldClose(window)) {

        if (glfwGetKey(window, GLFW_KEY_ESCAPE)) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(program);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteProgram(program);

    glfwDestroyWindow(window);
    glfwTerminate();

    return EXIT_SUCCESS;
}
