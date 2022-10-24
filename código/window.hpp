#ifndef WINDOW_HPP_
#define WINDOW_HPP_

#include <random>

#include "abcgOpenGL.hpp"

#include "asteroids.hpp"
#include "bullets.hpp"
#include "ship.hpp"
#include "starlayers.hpp"

class Window : public abcg::OpenGLWindow {
protected:
  void onEvent(SDL_Event const &event) override;
  void onCreate() override;
  void onUpdate() override;
  void onPaint() override;
  void onPaintUI() override;
  void onResize(glm::ivec2 const &size) override;
  void onDestroy() override;

private:
  glm::ivec2 m_viewportSize{};

  GLuint m_starsProgram{};
  GLuint m_objectsProgram{};

  GameData m_gameData;

  Asteroids m_asteroids;
  Ship m_ship;
  StarLayers m_starLayers;

  abcg::Timer m_restartWaitTimer;

  ImFont *m_font{};
  ImFont *m_font2{};

  std::default_random_engine m_randomEngine;

  void start();
  void restart();
  void checkCollisions(float deltaTime);
  void checkWinCondition();
  std::uniform_real_distribution<float> m_randomDist{-1.0f, 1.0f};
};

#endif