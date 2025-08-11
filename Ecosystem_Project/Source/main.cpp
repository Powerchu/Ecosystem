#include <stdio.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <GL/gl3w.h>
#include <GLFW/glfw3.h>

#include "EcoSystem/EcoSystem.h"
#include "EcoSystem/ParallelEcoSystem.h"

// Ecosystem mode selection
enum class EcosystemMode {
  NOT_SELECTED = 0,
  SINGLE_THREADED = 1,
  MULTI_THREADED = 2
};

static void glfw_error_callback(int error, const char* description) {
  fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

// Launcher UI for ecosystem mode selection
bool ShowEcosystemLauncher(EcosystemMode& selected_mode) {
  // Center the launcher window
  ImGuiViewport* viewport = ImGui::GetMainViewport();
  ImVec2 center = viewport->GetCenter();
  ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
  ImGui::SetNextWindowSize(ImVec2(500, 350), ImGuiCond_Appearing);

  // Create launcher window
  if (!ImGui::Begin("Ecosystem Launcher", nullptr, 
                    ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)) {
    ImGui::End();
    return false;
  }

  // Title and description
  ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]); // Use default font, larger
  ImGui::SetWindowFontScale(1.2f);
  ImGui::Text("🌱 Ecosystem Simulation");
  ImGui::SetWindowFontScale(1.0f);
  ImGui::PopFont();
  
  ImGui::Separator();
  ImGui::Spacing();

  // Description text
  ImGui::TextWrapped(
    "Choose your ecosystem simulation mode. Each mode offers different "
    "performance characteristics and capabilities:");
  
  ImGui::Spacing();
  ImGui::Spacing();

  // Single-threaded option
  ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.2f, 0.8f));
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.8f, 0.3f, 1.0f));
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.6f, 0.1f, 1.0f));
  
  if (ImGui::Button("🔧 Single-Threaded Mode", ImVec2(-1, 60))) {
    selected_mode = EcosystemMode::SINGLE_THREADED;
    ImGui::PopStyleColor(3);
    ImGui::End();
    return true;
  }
  ImGui::PopStyleColor(3);

  ImGui::BulletText("Classic ecosystem simulation");
  ImGui::BulletText("Stable and reliable performance");
  ImGui::BulletText("Lower CPU usage");
  ImGui::BulletText("Easier debugging and analysis");
  
  ImGui::Spacing();
  ImGui::Spacing();

  // Multi-threaded option
  ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.8f, 0.8f));
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.3f, 0.9f, 1.0f));
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.1f, 0.7f, 1.0f));
  
  if (ImGui::Button("⚡ Multi-Threaded Mode (Parallel)", ImVec2(-1, 60))) {
    selected_mode = EcosystemMode::MULTI_THREADED;
    ImGui::PopStyleColor(3);
    ImGui::End();
    return true;
  }
  ImGui::PopStyleColor(3);

  ImGui::BulletText("High-performance parallel processing");
  ImGui::BulletText("Supports larger creature populations");
  ImGui::BulletText("Multi-core CPU utilization");
  ImGui::BulletText("Advanced spatial partitioning");
  
  ImGui::Spacing();
  ImGui::Spacing();
  
  // Footer info
  ImGui::Separator();
  ImGui::TextDisabled("Tip: You can switch modes by restarting the application");

  ImGui::End();
  return false;
}

int main(int, char**) {
  // Setup window
  glfwSetErrorCallback(glfw_error_callback);
  if (!glfwInit()) return 1;

  // Decide GL+GLSL versions
#ifdef __APPLE__
  // GL 3.2 + GLSL 150 (required for macOS)
  const char* glsl_version = "#version 150";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);  // Required on Mac
#else
  // GL 3.0 + GLSL 130 (for other platforms)
  const char* glsl_version = "#version 130";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#endif

  // Create window with graphics context
  GLFWwindow* window = glfwCreateWindow(
      1280, 720, "Ecosystem Simulation - Choose Mode", NULL, NULL);
  if (window == NULL) return 1;
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);  // Enable vsync

#ifdef _WIN32
  FreeConsole();  // Hide console window on Windows
#endif
  // Initialize OpenGL loader
  bool err = gl3wInit() != 0;
  if (err) {
    fprintf(stderr, "Failed to initialize OpenGL loader!\n");
    return 1;
  }

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  (void)io;

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();

  // Setup Platform/Renderer bindings
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init(glsl_version);

  ImVec4 clear_color = ImVec4(0.f, 0.f, 0.f, 1.00f);

  // Mode selection state
  EcosystemMode selected_mode = EcosystemMode::NOT_SELECTED;
  bool ecosystem_initialized = false;
  
  // Ecosystem pointers - only one will be used based on selection
  Ecosystem::EcoSystem* single_threaded_eco = nullptr;
  Ecosystem::ParallelEcoSystem* multi_threaded_eco = nullptr;

  // Main loop
  while (!glfwWindowShouldClose(window)) {
    int display_w, display_h;
    glfwMakeContextCurrent(window);
    glfwGetFramebufferSize(window, &display_w, &display_h);

    glfwPollEvents();

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Show launcher if mode not selected
    if (selected_mode == EcosystemMode::NOT_SELECTED) {
      bool mode_selected = ShowEcosystemLauncher(selected_mode);
      
      if (mode_selected) {
        // Initialize the selected ecosystem mode
        if (selected_mode == EcosystemMode::SINGLE_THREADED) {
          single_threaded_eco = &Ecosystem::EcoSystem::GetInst();
          single_threaded_eco->Init();
          glfwSetWindowTitle(window, "Ecosystem Simulation - Single-Threaded Mode");
          printf("🔧 Initialized Single-Threaded Ecosystem\n");
        } else if (selected_mode == EcosystemMode::MULTI_THREADED) {
          multi_threaded_eco = &Ecosystem::ParallelEcoSystem::GetInst();
          multi_threaded_eco->Init();
          multi_threaded_eco->InitializeParallel(); // Initialize parallel processing
          glfwSetWindowTitle(window, "Ecosystem Simulation - Multi-Threaded Mode");
          printf("⚡ Initialized Multi-Threaded Ecosystem with Parallel Processing\n");
        }
        ecosystem_initialized = true;
      }
    } else if (ecosystem_initialized) {
      // Update the selected ecosystem
      if (selected_mode == EcosystemMode::SINGLE_THREADED && single_threaded_eco) {
        single_threaded_eco->UpdateWindowSize(display_w, display_h);
        single_threaded_eco->Update(1.f / ImGui::GetIO().Framerate);
      } else if (selected_mode == EcosystemMode::MULTI_THREADED && multi_threaded_eco) {
        multi_threaded_eco->UpdateWindowSize(display_w, display_h);
        multi_threaded_eco->ParallelUpdate(1.f / ImGui::GetIO().Framerate);
      }

      // Show mode status indicator in top-right corner
      ImGuiViewport* viewport = ImGui::GetMainViewport();
      ImVec2 work_pos = viewport->WorkPos;
      ImVec2 work_size = viewport->WorkSize;
      ImVec2 window_pos = ImVec2(work_pos.x + work_size.x - 10, work_pos.y + 10);
      ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, ImVec2(1.0f, 0.0f));
      ImGui::SetNextWindowBgAlpha(0.8f);
      
      if (ImGui::Begin("Mode Status", nullptr, 
                       ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                       ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize |
                       ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing |
                       ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoTitleBar)) {
        if (selected_mode == EcosystemMode::SINGLE_THREADED) {
          ImGui::TextColored(ImVec4(0.3f, 0.8f, 0.3f, 1.0f), "🔧 Single-Threaded");
        } else if (selected_mode == EcosystemMode::MULTI_THREADED) {
          ImGui::TextColored(ImVec4(0.3f, 0.3f, 0.9f, 1.0f), "⚡ Multi-Threaded");
        }
        ImGui::End();
      }
    }

    // Rendering
    ImGui::Render();
    glViewport(0, 0, display_w, display_h);
    glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwMakeContextCurrent(window);
    glfwSwapBuffers(window);
  }

  // Cleanup
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
