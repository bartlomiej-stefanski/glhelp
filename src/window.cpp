#include <iostream>
#include <stdexcept>
#include <string>

#include <glad/gl.h>

#include <GLFW/glfw3.h>

#include <glhelp/glfw_context.hpp>
#include <glhelp/window.hpp>

namespace glhelp {

Window::Window(int width, int height, const std::string& name)
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

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
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
  std::cerr << "Loaded OpenGL " << GLAD_VERSION_MAJOR(version) << '.' << GLAD_VERSION_MINOR(version) << std::endl;

  glfwGetWindowSize(window, &this->width, &this->height);

  // Setup Face culling
  glEnable(GL_CULL_FACE);
  glFrontFace(GL_CCW); // Calculate normals counter-clockwise
  glCullFace(GL_BACK); // Cull back faces

  // Setup z-buffer
  glEnable(GL_DEPTH_TEST);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glViewport(0, 0, this->width, this->height);
  glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

  glfwSetWindowUserPointer(window, this);
  glfwSetWindowSizeCallback(window, Window::resize_callback);
  glfwSetKeyCallback(window, Window::key_callback);

  glfwSwapInterval(1); // VSync
  glEnable(GL_MULTISAMPLE);
};

Window::Window(Window&& other) noexcept
    : width(other.width), height(other.height), window(other.window), monitor(other.monitor)
{
  std::swap(key_callbacks, other.key_callbacks);
  std::swap(resize_callbacks, other.resize_callbacks);

  other.window = nullptr;
  other.monitor = nullptr;
  other.width = 0;
  other.height = 0;
}

Window& Window::operator=(Window&& other) noexcept
{
  if (this != &other) {
    glfwDestroyWindow(window);
    window = other.window;
    monitor = other.monitor;
    width = other.width;
    height = other.height;

    other.window = nullptr;
    other.monitor = nullptr;
    other.width = 0;
    other.height = 0;
  }
  return *this;
}

Window::~Window()
{
  if (window != nullptr)
    glfwDestroyWindow(window);
}

float Window::aspect_ratio() const noexcept { return (float)width / (float)height; }

void Window::resize_cb(int new_width, int new_heigth)
{
  width = new_width;
  height = new_heigth;
  glViewport(0, 0, new_width, new_heigth);

  for (const auto& callback : resize_callbacks)
    callback(new_width, new_heigth);
}

void Window::key_cb([[maybe_unused]] int key, [[maybe_unused]] int scancode, [[maybe_unused]] int action,
                    [[maybe_unused]] int mods)
{
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);

  for (const auto& callback : key_callbacks[key])
    callback(key, scancode, action, mods);
}

void Window::run_synchronously(std::function< void(Window&, double) > main_loop)
{
  double prev_time{glfwGetTime()};
  GLenum error;

  while (!glfwWindowShouldClose(window)) {
    error = glGetError();
    if (error != GL_NO_ERROR)
      throw std::runtime_error("OpenGL error before main loop: " + std::to_string(error));

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    main_loop(*this, glfwGetTime() - prev_time);
    prev_time = glfwGetTime();

    glfwSwapBuffers(window);
    glfwPollEvents();

    error = glGetError();
    if (error != GL_NO_ERROR)
      throw std::runtime_error("OpenGL error after main loop: " + std::to_string(error));
  }
}

void Window::resize_callback(GLFWwindow* window, int new_width, int new_height)
{
  void* ptr{glfwGetWindowUserPointer(window)};
  if (Window* winPtr = static_cast< Window* >(ptr)) {
    winPtr->resize_cb(new_width, new_height);
  }
}

void Window::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  void* ptr{glfwGetWindowUserPointer(window)};
  if (Window* winPtr = static_cast< Window* >(ptr))
    winPtr->key_cb(key, scancode, action, mods);
}

} // namespace glhelp
