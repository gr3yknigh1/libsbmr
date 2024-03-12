#include <stdio.h>
#include <stdlib.h>

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <cglm/vec3.h>

#include <nostdlib/buf.h>
#include <nostdlib/fs.h>
#include <nostdlib/macros.h>
#include <nostdlib/types.h>

#include "io.h"
#include "shader.h"

typedef unsigned short rc_t;

#define WINDOW_WIDTH 900
#define WINDOW_HEIGHT 600

#define SHADER_VERT_PATH "./assets/shaders/basic.vert"
#define SHADER_FRAG_PATH "./assets/shaders/basic.frag"

#define LOG_INFO(...) fprintf(stdout, "I: " __VA_ARGS__)
#define LOG_DEBUG(...) fprintf(stdout, "D: " __VA_ARGS__)
#define LOG_ERROR(...) fprintf(stderr, "E: " __VA_ARGS__)

static vec4 CLEAR_COLOR = {0.2f, 0.3f, 0.3f};

static f32 VERTICES[] = {
    -0.5f, 0.5f,  0.0f, //
    -0.5f, -0.5f, 0.0f, //
    0.5f,  0.5f,  0.0f, //
    0.5f,  0.5f,  0.0f, //
    0.5f,  -0.5f, 0.0f, //
    -0.5f, -0.5f, 0.0f, //
};


#define VEC3_ITEM_COUNT 3

static void
glad_message_callback(GLenum source, GLenum type, GLuint id, GLenum severity,
                      GLsizei length, const GLchar *message,
                      const void *user_param) {
    KEEP(source);
    KEEP(id);
    KEEP(length);
    KEEP(user_param);

    LOG_ERROR("GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
              (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), type,
              severity, message);
}

static void
glfw_error_callback(int code, const char *description) {
    LOG_ERROR("[GLFW] (%i) %s\n", code, description);
}

static void
glfw_frame_buffer_size_callback(GLFWwindow *, int width, int height) {
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

    GLFWwindow *window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT,
                                          "Breakout", nullptr, nullptr);
    glfwSetErrorCallback(glfw_error_callback);

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
    glDebugMessageCallback(glad_message_callback, nullptr);
#endif

    glfwSetFramebufferSizeCallback(window, glfw_frame_buffer_size_callback);

    LOG_INFO("Successfully initialized!\n");

    GLuint program = glCreateProgram();

    {
        shader_t vert_shader = 0;
        shader_compile_status_t vert_shader_status = shader_load_and_compile(
            SHADER_VERT_PATH, SHADER_TYPE_VERT, &vert_shader);

        if (vert_shader_status != SHADER_COMPILE_OK) {
            LOG_ERROR("Failed to load vertex shader source: rc: %hi\n",
                      vert_shader_status);
            glfwDestroyWindow(window);
            glfwTerminate();
            return EXIT_FAILURE;
        }

        shader_t fragment_shader = 0;
        shader_compile_status_t frag_shader_status = shader_load_and_compile(
            SHADER_FRAG_PATH, SHADER_TYPE_FRAG, &fragment_shader);

        if (frag_shader_status != SHADER_COMPILE_OK) {
            LOG_ERROR("Failed to load vertex shader source: rc: %hi\n",
                      vert_shader_status);
            glfwDestroyWindow(window);
            glfwTerminate();
            return EXIT_FAILURE;
        }

        glAttachShader(program, vert_shader);
        glAttachShader(program, fragment_shader);
        glLinkProgram(program);

        {
            int success;
            char info_log[GL_INFO_LOG_LENGTH];
            glGetProgramiv(program, GL_LINK_STATUS, &success);
            if (!success) {
                glGetProgramInfoLog(program, sizeof(info_log), nullptr,
                                    info_log);
                LOG_ERROR("Failed to link shader program: %s\n", info_log);
                glfwDestroyWindow(window);
                glfwTerminate();
                return EXIT_FAILURE;
            }
        }

        glDeleteShader(vert_shader);
        glDeleteShader(fragment_shader);
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
        glBufferData(GL_ARRAY_BUFFER, sizeof(VERTICES), VERTICES,
                     GL_STATIC_DRAW);
    }

    {
        glVertexAttribPointer(0, VEC3_ITEM_COUNT, GL_FLOAT, GL_FALSE,
                              VEC3_ITEM_COUNT * sizeof(f32), nullptr);
        glEnableVertexAttribArray(0);
    }

    {
        GLenum err = GL_NO_ERROR;
        while ((err = glGetError()) != GL_NO_ERROR) {
            LOG_ERROR("GL error cought: %i\n", err);
        }
    }

#if 0
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
#endif

    while (!glfwWindowShouldClose(window)) {

        if (glfwGetKey(window, GLFW_KEY_ESCAPE)) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }

        glClearColor(CLEAR_COLOR[0], CLEAR_COLOR[1], CLEAR_COLOR[2], 1.0f);
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
