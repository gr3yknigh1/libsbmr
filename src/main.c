#include <stdio.h>
#include <stdlib.h>

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <cglm/vec3.h>

#include <nostdlib/buf.h>
#include <nostdlib/fs.h>
#include <nostdlib/macros.h>
#include <nostdlib/types.h>

typedef char *const path_t;
typedef unsigned short rc_t;

#define OUT

#define WINDOW_WIDTH 900
#define WINDOW_HEIGHT 600

#define SHADER_VERT_PATH "./assets/shaders/basic.vert"

#define LOG_INFO(...) fprintf(stdout, "I: " __VA_ARGS__)
#define LOG_DEBUG(...) fprintf(stdout, "D: " __VA_ARGS__)
#define LOG_ERROR(...) fprintf(stderr, "E: " __VA_ARGS__)

static vec4 CLEAR_COLOR = {0.2f, 0.3f, 0.3f};

static f32 TRIANGLE_VERTICES[] = {
    -0.5f, -0.5f, 0.0f, //
    0.5f,  -0.5f, 0.0f, //
    0.0f,  0.5f,  0.0f  //
};

static void
glfw_error_callback(int code, const char *description) {
    LOG_ERROR("[GLFW] (%i) %s\n", code, description);
}

static void
glfw_frame_buffer_size_callback(GLFWwindow *, int width, int height) {
    glViewport(0, 0, width, height);
}

static void
tick_update(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE)) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

static void
tick_render(GLFWwindow *window) {
    KEEP(window);

    glClearColor(CLEAR_COLOR[0], CLEAR_COLOR[1], CLEAR_COLOR[2], 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

static u64
fs_file_size(FILE *f) {
    u64 size = 0;

    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fseek(f, 0, SEEK_SET);

    return size;
}

static enum {
    LOAD_FILE_OK,
    LOAD_FILE_FAILED,
    LOAD_FILE_NOT_EXISTS,
    LOAD_FILE_NOT_OPENED,
    LOAD_FILE_READ_ERR,
} load_file(const path_t path, char **const OUT file_content,
            u64 *OUT file_size) {
    if (!noc_fs_is_exists(path)) {
        return LOAD_FILE_NOT_EXISTS;
    }

    if (file_size == nullptr) {
        return LOAD_FILE_FAILED;
    }

    FILE *f = fopen(path, "r");

    if (f == nullptr) {
        return LOAD_FILE_NOT_OPENED;
    }

    *file_size = fs_file_size(f);

    if (file_size == 0) {
        return LOAD_FILE_OK;
    }

    *file_content = malloc(*file_size);
    fread(*file_content, *file_size, 1, f);
    fclose(f);

    return LOAD_FILE_OK;
}

typedef enum {
    SHADER_TYPE_VERT,
    SHADER_TYPE_FRAG,
    SHADER_TYPE_COUNT,
} shader_type_t;

static bool
shader_type_is_valid(shader_type_t value) {
    return value >= 0 && value < SHADER_TYPE_COUNT;
}

typedef struct {
    GLuint id;
    shader_type_t type;
} shader_t;

typedef enum {
    SHADER_COMPILE_OK,
    SHADER_COMPILE_ERR,
    SHADER_COMPILE_INVALID_VALUE,
    SHADER_COMPILE_FILE_NOT_FOUND,
    SHADER_COMPILE_FILE_LOAD_ERR,
} shader_compile_rc;

static shader_compile_rc
shader_compile(const char *source, u64 source_length, shader_type_t type,
               shader_t *shader) {
    if (source == nullptr || source_length == 0 || shader == nullptr ||
        !shader_type_is_valid(type)) {
        return SHADER_COMPILE_INVALID_VALUE;
    }

    GLuint id = 0;

    switch (type) {
    case SHADER_TYPE_VERT:
        id = glCreateShader(GL_VERTEX_SHADER);
        break;
    case SHADER_TYPE_FRAG:
        id = glCreateShader(GL_FRAGMENT_SHADER);
        break;
    case SHADER_TYPE_COUNT:
        return SHADER_COMPILE_INVALID_VALUE;
    }

    glShaderSource(id, 1, &source, (GLint *)&source_length);

    // TODO(i.akkuzin): Add handling of compilation status.
    glCompileShader(id);

    *shader = (shader_t){
        .id = id,
        .type = type,
    };

    return SHADER_COMPILE_OK;
}

static shader_compile_rc
shader_load_and_compile(const path_t source_file, shader_type_t type,
                        shader_t *shader) {
    char *source = nullptr;
    u64 source_length = 0;

    rc_t rc = load_file(source_file, (char **const)&source, &source_length);

    switch (rc) {
    case LOAD_FILE_OK:
        break;
    case LOAD_FILE_NOT_EXISTS:
        return SHADER_COMPILE_FILE_NOT_FOUND;
    case LOAD_FILE_FAILED:
    case LOAD_FILE_NOT_OPENED:
    case LOAD_FILE_READ_ERR:
        return SHADER_COMPILE_FILE_LOAD_ERR;
    }

    rc = shader_compile(source, source_length, type, shader);
    free(source);
    return rc;
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
#ifdef GL_USE_COMPAT
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif // GL_USE_COMPAT

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

    glfwSetFramebufferSizeCallback(window, glfw_frame_buffer_size_callback);

    LOG_INFO("Successfuly initialized!\n");

    GLuint vbo = 0;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(TRIANGLE_VERTICES), TRIANGLE_VERTICES,
                 GL_STATIC_DRAW);

    // GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);

    shader_t vertex_shader = {0};
    rc_t vertex_shader_comp_rc = shader_load_and_compile(
        SHADER_VERT_PATH, SHADER_TYPE_VERT, &vertex_shader);

    if (vertex_shader_comp_rc != SHADER_COMPILE_OK) {
        LOG_ERROR("Failed to load vertex shader source: rc: %hi\n", vertex_shader_comp_rc);
        glfwDestroyWindow(window);
        glfwTerminate();
        return EXIT_FAILURE;
    }

    while (!glfwWindowShouldClose(window)) {
        tick_update(window);
        tick_render(window);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return EXIT_SUCCESS;
}
