#include <iomanip>
#include <iostream>
#include <print>
#include <stdexcept>
#include <string>

#include <glad/gl.h>

#include <GLFW/glfw3.h>

#include <glhelp/Error.hpp>
#include <glhelp/Window.hpp>
#include <glhelp/utils/GLFWContext.hpp>

#ifdef DEBUG_GLHELP
static void GLAPIENTRY MessageCallback(GLenum source [[maybe_unused]],
                                       GLenum type,
                                       GLuint id [[maybe_unused]],
                                       GLenum severity,
                                       GLsizei length [[maybe_unused]],
                                       const GLchar* message,
                                       const void* userParam [[maybe_unused]])
{
  if (severity == GL_DEBUG_SEVERITY_NOTIFICATION)
    return;

  std::println(stderr, "GL CALLBACK: {} type = 0x{:x}, severity = 0x{:x}, message = {}",
               (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
               type, severity, message);
}
#endif

namespace glhelp {

Window::Window(int width, int height, const std::string& name, bool write_fps, GLuint cursor_mode)
    : write_fps(write_fps)
{
  if (!GLFWContext::is_initialized()) {
    throw std::runtime_error("Cannot create a window without GLFW context initialized");
  }

  if (width == 0 || height == 0) {
    throw std::invalid_argument("Window dimensions cannot be zero");
  }

  // Initialize GLFW monitors
  int count;
  GLFWmonitor** mons{glfwGetMonitors(&count)};
  if (mons == nullptr) {
    throw std::runtime_error("Error in glfwGetMonitors getting monitors");
  }
  monitor = mons[0];

  // Hints for creating GLFW window
  glfwWindowHint(GLFW_SAMPLES, 4);
  glfwWindowHint(GLFW_AUTO_ICONIFY, GL_FALSE);

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // Craete glfw window
  window = glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);
  if (window == nullptr) {
    throw std::runtime_error("Error initializing window !");
  }
  glfwMakeContextCurrent(window);

  // Initialize GLAD. Must come after craeting a window.
  int version = gladLoadGL(glfwGetProcAddress);
  if (version == 0) {
    throw std::runtime_error("Failed to initialize GLAD");
  }
  std::cerr << "Loaded OpenGL " << GLAD_VERSION_MAJOR(version) << '.' << GLAD_VERSION_MINOR(version) << '\n';

  glfwGetWindowSize(window, &this->width, &this->height);

  // Setup Face culling
  glEnable(GL_CULL_FACE);
  glFrontFace(GL_CCW); // Calculate normals counter-clockwise
  glCullFace(GL_BACK); // Cull back faces

  // Setup z-buffer
  glEnable(GL_DEPTH_TEST);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glViewport(0, 0, this->width, this->height);
  glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

  glfwSetWindowUserPointer(window, this);
  glfwSetWindowSizeCallback(window, Window::resize_callback);
  glfwSetKeyCallback(window, Window::key_callback);

  glfwSetInputMode(window, GLFW_CURSOR, cursor_mode);
  glfwSetCursorPosCallback(window, Window::initial_mouse_callback);
  glfwSetMouseButtonCallback(window, Window::initial_mouse_button_callback);
  glfwSetScrollCallback(window, Window::scroll_callback);

  glfwSwapInterval(0); // VSync
  glEnable(GL_MULTISAMPLE);

  glGenQueries(1, &primitiveQuery);

#ifdef DEBUG_GLHELP
  glEnable(GL_DEBUG_OUTPUT);
  glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
  glDebugMessageCallback(MessageCallback, nullptr);
#endif
};

Window::~Window()
{
  if (window != nullptr)
    glfwDestroyWindow(window);
}

auto Window::aspect_ratio() const noexcept -> float { return (float)width / (float)height; }
auto Window::get_size() const noexcept -> glm::vec2 { return {width, height}; }

void Window::resize_cb(int new_width, int new_heigth)
{
  width = new_width;
  height = new_heigth;
  glViewport(0, 0, new_width, new_heigth);

  resize_event(new_width, new_heigth);
}

void Window::key_cb(int key, [[maybe_unused]] int scancode, [[maybe_unused]] int action, [[maybe_unused]] int mods)
{
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);

  key_event[key](key, action, mods, last_frame_time);
}

void Window::mouse_cb(double xpos, double ypos)
{
  mouse_event((xpos - last_xpos) / width, (last_ypos - ypos) / height);
  last_xpos = xpos;
  last_ypos = ypos;
}

void Window::scroll_cb(double xoffset, double yoffset)
{
  scroll_event(xoffset, yoffset);
}

void Window::mouse_button_cb(int button, int action, int mods [[maybe_unused]])
{
  mouse_button_event(button, action);
}

void Window::run_synchronously(const std::function< bool(Window&, double, double) >& main_loop)
{
  if (glfwWindowShouldClose(window)) {
    throw std::runtime_error("Window: 'run_synchronously' called after exit request!");
  }

  double prev_time{glfwGetTime()};
  bool should_continue{true};

  while (!glfwWindowShouldClose(window) && should_continue) {
    CHECK_GL("Before main loop");

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glfwPollEvents();

    const double curr_time{glfwGetTime()};
    last_frame_time = curr_time - prev_time;

    if (write_fps) {
      debug_print_fps(last_frame_time);
    }

    if (!query_started) {
      query_started = true;
      glBeginQuery(GL_PRIMITIVES_GENERATED, primitiveQuery);
      should_continue = main_loop(*this, curr_time, last_frame_time);
      glEndQuery(GL_PRIMITIVES_GENERATED);
    }
    else {
      should_continue = main_loop(*this, curr_time, last_frame_time);
    }

    prev_time = curr_time;

    glfwSwapBuffers(window);

    if (query_started) {
      // Never wait for the triangles-drawn query.
      GLuint triangles_drawn_available{0};
      glGetQueryObjectuiv(primitiveQuery, GL_QUERY_RESULT_AVAILABLE, &triangles_drawn_available);
      if (triangles_drawn_available && query_started) {
        glGetQueryObjectuiv(primitiveQuery, GL_QUERY_RESULT, &triangles_drawn);
        query_started = false;
      }
    }

    CHECK_GL("After main loop");
  }
}

#define ANSI_ESCAPE "\r"
#define ANSI_CLEAR_LINE "\033[2K"

void Window::debug_print_fps(float frame_time)
{
  static unsigned past_frame_count{0};
  static float past_worst_fps{1.0F};

  static float time_passed{0.0F};
  static unsigned frame_count{0};
  static float longest_frame{0.000000001F};

  frame_count++;
  time_passed += frame_time;
  longest_frame = std::max(longest_frame, frame_time);

  if (time_passed > 1.0F) {
    past_frame_count = frame_count;
    past_worst_fps = 1.0F / longest_frame;
    frame_count = 0;
    time_passed = 0.0F;
    longest_frame = 0.000000001F;
    std::cout << ANSI_ESCAPE ANSI_CLEAR_LINE
              << std::fixed << std::setprecision(2)
              << "\t\tAVERAGE_FPS: " << past_frame_count
              << "\t\tWORST_FPS: " << past_worst_fps
              << "\t\tTRIANGELS_DRAWN: " << triangles_drawn;

    if (fps_callback.has_value()) {
      std::cout << "\t\t" << fps_callback.value()();
    }

    std::cout << std::flush;
  }
}

void Window::resize_callback(GLFWwindow* window, int new_width, int new_height)
{
  // Do not emit resize event immediatley after resize
  glfwSetCursorPosCallback(window, Window::initial_mouse_callback);

  void* ptr{glfwGetWindowUserPointer(window)};
  if (auto winPtr{static_cast< Window* >(ptr)}) {
    winPtr->resize_cb(new_width, new_height);
  }
}

void Window::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  void* ptr{glfwGetWindowUserPointer(window)};
  if (auto winPtr{static_cast< Window* >(ptr)})
    winPtr->key_cb(key, scancode, action, mods);
}

void Window::mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
  void* ptr{glfwGetWindowUserPointer(window)};
  if (auto winPtr{static_cast< Window* >(ptr)})
    winPtr->mouse_cb(xpos, ypos);
}

void Window::initial_mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
  void* ptr{glfwGetWindowUserPointer(window)};
  if (auto winPtr{static_cast< Window* >(ptr)}) {
    winPtr->last_xpos = xpos;
    winPtr->last_ypos = ypos;
    glfwSetCursorPosCallback(window, Window::mouse_callback);
  }
}

void Window::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
  void* ptr{glfwGetWindowUserPointer(window)};
  if (auto winPtr{static_cast< Window* >(ptr)})
    winPtr->scroll_event((float)xoffset, (float)yoffset);
}

void Window::initial_mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
  void* ptr{glfwGetWindowUserPointer(window)};
  if (auto winPtr{static_cast< Window* >(ptr)}) {
    winPtr->mouse_button_cb(button, action, mods);
  }
}

} // namespace glhelp
