#include "window.hpp"

void Window::onEvent(SDL_Event const &event) {
  // Keyboard events
  if (event.type == SDL_KEYDOWN) {

    if (event.key.keysym.sym == SDLK_LEFT || event.key.keysym.sym == SDLK_a)
      m_gameData.m_input.set(gsl::narrow<size_t>(Input::Left));
    if (event.key.keysym.sym == SDLK_RIGHT || event.key.keysym.sym == SDLK_d)
      m_gameData.m_input.set(gsl::narrow<size_t>(Input::Right));
  }
  if (event.type == SDL_KEYUP) {

    if (event.key.keysym.sym == SDLK_LEFT || event.key.keysym.sym == SDLK_a)
      m_gameData.m_input.reset(gsl::narrow<size_t>(Input::Left));
    if (event.key.keysym.sym == SDLK_RIGHT || event.key.keysym.sym == SDLK_d)
      m_gameData.m_input.reset(gsl::narrow<size_t>(Input::Right));
  }
}

void Window::onCreate() {
  auto const assetsPath{abcg::Application::getAssetsPath()};

  // Load a new font
  auto const filename{assetsPath + "Inconsolata-Medium.ttf"};
  m_font = ImGui::GetIO().Fonts->AddFontFromFileTTF(filename.c_str(), 45.0f);
  m_font2 = ImGui::GetIO().Fonts->AddFontFromFileTTF(filename.c_str(), 40.0f);
  if (m_font == nullptr) {
    throw abcg::RuntimeError("Cannot load font file");
  }

  // Create program to render the other objects
  m_objectsProgram =
      abcg::createOpenGLProgram({{.source = assetsPath + "objects.vert",
                                  .stage = abcg::ShaderStage::Vertex},
                                 {.source = assetsPath + "objects.frag",
                                  .stage = abcg::ShaderStage::Fragment}});

  // Create program to render the stars
  m_starsProgram =
      abcg::createOpenGLProgram({{.source = assetsPath + "stars.vert",
                                  .stage = abcg::ShaderStage::Vertex},
                                 {.source = assetsPath + "stars.frag",
                                  .stage = abcg::ShaderStage::Fragment}});

  abcg::glClearColor(0, 0, 0, 1);

#if !defined(__EMSCRIPTEN__)
  abcg::glEnable(GL_PROGRAM_POINT_SIZE);
#endif

  // Start pseudo-random number generator
  m_randomEngine.seed(
      std::chrono::steady_clock::now().time_since_epoch().count());

  start();
}

void Window::start() {

  m_gameData.m_state = State::Start;

  m_starLayers.create(m_starsProgram, 25);
}

void Window::restart() {
  m_restartWaitTimer.restart();

  m_gameData.m_state = State::Playing;

  m_starLayers.create(m_starsProgram, 25);
  m_ship.create(m_objectsProgram);
  m_asteroids.create(m_objectsProgram, 3);
}

void Window::onUpdate() {
  auto const deltaTime{gsl::narrow_cast<float>(getDeltaTime())};

  m_ship.update(m_gameData, hard);
  m_starLayers.update(deltaTime, m_gameData);
  m_asteroids.update(deltaTime, m_gameData, hard);

  if (m_gameData.m_state == State::Playing) {
    checkCollisions(deltaTime);
    checkWinCondition();
  }
}

void Window::onPaint() {
  abcg::glClear(GL_COLOR_BUFFER_BIT);
  abcg::glViewport(0, 0, m_viewportSize.x, m_viewportSize.y);

  m_starLayers.paint();
  m_asteroids.paint();
  m_ship.paint(m_gameData);
}

void Window::onPaintUI() {
  abcg::OpenGLWindow::onPaintUI();

  if (m_gameData.m_state == State::Start) {
    auto const sizeStart{ImVec2(m_viewportSize.x, m_viewportSize.y)};
    auto const positionStart{
        ImVec2((m_viewportSize.x) / 23.0f, (m_viewportSize.y) / 10.0f)};
    ImGui::SetNextWindowPos(positionStart);
    ImGui::SetNextWindowSize(sizeStart);
    ImGuiWindowFlags const flags{ImGuiWindowFlags_NoBackground |
                                 ImGuiWindowFlags_NoTitleBar |
                                 ImGuiWindowFlags_NoNavInputs};
    ImGui::Begin(" ", nullptr, flags);
    ImGui::PushFont(m_font);

    ImGui::Text("Rebata os meteoros");
    ImGui::Text("por 40 segundos");

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 1.0f, 1.00f));
    ImGui::Text("Use as setas do teclado");
    ImGui::Text("ou as teclas A e D");
    ImGui::PopStyleColor();

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0, 0.5f, 0, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0, 0.4f, 0, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0, 0.5f, 0, 1.0f});
    if (ImGui::Button("Iniciar -Fácil", ImVec2(380, 60))) {
      hard = false;
      restart();
    }
    ImGui::PopStyleColor(3);

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.5f, 0, 0, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.4f, 0, 0, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.5f, 0, 0, 1.0f});
    if (ImGui::Button("Iniciar -Difícil", ImVec2(380, 60))) {
      hard = true;
      restart();
    }
    ImGui::PopStyleColor(3);

    ImGui::PopFont();
    ImGui::End();
  }

  if (m_gameData.m_state == State::Playing) {
    auto count{
        fmt::format("{}", (40 - std::round(m_restartWaitTimer.elapsed())))};
    auto const sizeStart{ImVec2(m_viewportSize.x, m_viewportSize.y)};
    auto const positionStart{
        ImVec2((m_viewportSize.x) / 23.0f, (m_viewportSize.y) / 17.0f)};
    ImGui::SetNextWindowPos(positionStart);
    ImGui::SetNextWindowSize(sizeStart);
    ImGuiWindowFlags const flags{ImGuiWindowFlags_NoBackground |
                                 ImGuiWindowFlags_NoTitleBar |
                                 ImGuiWindowFlags_NoNavInputs};
    ImGui::Begin(" ", nullptr, flags);
    ImGui::PushFont(m_font2);

    ImGui::Text(count.c_str());

    ImGui::PopFont();
    ImGui::End();
  }

  {
    auto const size{ImVec2(m_viewportSize.x, m_viewportSize.y)};
    auto const position{
        ImVec2((m_viewportSize.x) / 15.0f, (m_viewportSize.y) / 8.0f)};
    ImGui::SetNextWindowPos(position);
    ImGui::SetNextWindowSize(size);
    ImGuiWindowFlags const flags{ImGuiWindowFlags_NoBackground |
                                 ImGuiWindowFlags_NoTitleBar |
                                 ImGuiWindowFlags_NoNavInputs};
    ImGui::Begin(" ", nullptr, flags);
    ImGui::PushFont(m_font);

    if (m_gameData.m_state == State::GameOver) {
      ImGui::Text("Game Over");

      ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0, 0.5f, 0, 1.0f});
      ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0, 0.4f, 0, 1.0f});
      ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0, 0.5f, 0, 1.0f});
      if (ImGui::Button("Reiniciar -Fácil", ImVec2(400, 60))) {
        hard = false;
        restart();
      }
      ImGui::PopStyleColor(3);

      ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.5f, 0, 0, 1.0f});
      ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.4f, 0, 0, 1.0f});
      ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.5f, 0, 0, 1.0f});
      if (ImGui::Button("Reiniciar -Difícil", ImVec2(400, 60))) {
        hard = true;
        restart();
      }
      ImGui::PopStyleColor(3);

    } else if (m_gameData.m_state == State::Win) {
      ImGui::Text("*Você Venceu!*");

      ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0, 0.5f, 0, 1.0f});
      ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0, 0.4f, 0, 1.0f});
      ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0, 0.5f, 0, 1.0f});
      if (ImGui::Button("Reiniciar -Fácil", ImVec2(400, 60))) {
        hard = false;
        restart();
      }
      ImGui::PopStyleColor(3);

      ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.5f, 0, 0, 1.0f});
      ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.4f, 0, 0, 1.0f});
      ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.5f, 0, 0, 1.0f});
      if (ImGui::Button("Reiniciar -Difícil", ImVec2(400, 60))) {
        hard = true;
        restart();
      }
      ImGui::PopStyleColor(3);
    }

    ImGui::PopFont();
    ImGui::End();
  }
}

void Window::onResize(glm::ivec2 const &size) {
  m_viewportSize = size;

  abcg::glClear(GL_COLOR_BUFFER_BIT);
}

void Window::onDestroy() {
  abcg::glDeleteProgram(m_starsProgram);
  abcg::glDeleteProgram(m_objectsProgram);

  m_asteroids.destroy();
  m_ship.destroy();
  m_starLayers.destroy();
}

void Window::checkCollisions(float deltaTime) {
  auto &re{m_randomEngine};

  // Check collision between ship and asteroids
  for (auto &asteroid : m_asteroids.m_asteroids) {
    auto const asteroidTranslation{asteroid.m_translation};
    auto const distance{
        glm::distance(m_ship.m_translation, asteroidTranslation)};

    if (distance < m_ship.m_scale * 0.9f + asteroid.m_scale * 0.85f) {

      asteroid.m_velocity =
          glm::normalize(glm::vec2{m_randomDist(re) * 500, +500}) / 2.0f;
      asteroid.m_velocity += 5.0f * deltaTime;
      asteroid.m_translation += asteroid.m_velocity * deltaTime;
    }
  }
}

void Window::checkWinCondition() {
  if (m_restartWaitTimer.elapsed() > 40) {
    m_gameData.m_state = State::Win;
  }
}