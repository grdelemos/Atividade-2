#include "asteroids.hpp"

#include <glm/gtx/fast_trigonometry.hpp>

void Asteroids::create(GLuint program, int quantity) {
  destroy();

  m_randomEngine.seed(
      std::chrono::steady_clock::now().time_since_epoch().count());

  m_program = program;

  // Get location of uniforms in the program
  m_colorLoc = abcg::glGetUniformLocation(m_program, "color");
  m_rotationLoc = abcg::glGetUniformLocation(m_program, "rotation");
  m_scaleLoc = abcg::glGetUniformLocation(m_program, "scale");
  m_translationLoc = abcg::glGetUniformLocation(m_program, "translation");

  // Create asteroids
  m_asteroids.clear();
  m_asteroids.resize(quantity);

  for (auto &asteroid : m_asteroids) {
    asteroid = makeAsteroid();

    do {
      asteroid.m_translation = {m_randomDist(m_randomEngine),
                                m_randomDist(m_randomEngine) + 1.5f};
    } while (glm::length(asteroid.m_translation) > 1.0f);
  }
}

void Asteroids::paint() {
  abcg::glUseProgram(m_program);

  for (auto const &asteroid : m_asteroids) {
    abcg::glBindVertexArray(asteroid.m_VAO);

    abcg::glUniform4fv(m_colorLoc, 1, &asteroid.m_color.r);
    abcg::glUniform1f(m_scaleLoc, asteroid.m_scale);
    abcg::glUniform1f(m_rotationLoc, asteroid.m_rotation);

    abcg::glUniform2f(m_translationLoc, asteroid.m_translation.x,
                      asteroid.m_translation.y);

    abcg::glDrawArrays(GL_TRIANGLE_FAN, 0, asteroid.m_polygonSides + 2);

    abcg::glBindVertexArray(0);
  }

  abcg::glUseProgram(0);
}

void Asteroids::destroy() {
  for (auto &asteroid : m_asteroids) {
    abcg::glDeleteBuffers(1, &asteroid.m_VBO);
    abcg::glDeleteVertexArrays(1, &asteroid.m_VAO);
  }
}

void Asteroids::update(float deltaTime, GameData &gameData, bool hard) {
  int interval;
  if (hard == true) {
    interval = 3;
  } else {
    interval = 5;
  }

  for (auto &asteroid : m_asteroids) {

    asteroid.m_rotation = glm::wrapAngle(
        asteroid.m_rotation + asteroid.m_angularVelocity * deltaTime);

    asteroid.m_velocity -= 0.02f * deltaTime;
    asteroid.m_translation += asteroid.m_velocity * deltaTime;

    // Wrap-around
    if (asteroid.m_translation.x < -0.9f)
      asteroid.m_velocity = glm::normalize(glm::vec2{+500, -500}) / 2.0f;
    if (asteroid.m_translation.x > +0.9f)
      asteroid.m_velocity = glm::normalize(glm::vec2{-500, -500}) / 2.0f;
    if (asteroid.m_translation.y > +0.9f)
      asteroid.m_velocity = glm::normalize(glm::vec2{-1, -500}) / 2.0f;
    if (asteroid.m_translation.y < -0.999f &&
        gameData.m_state == State::Playing) {
      gameData.m_state = State::GameOver;
    }
  }

  if (asTimer.elapsed() > interval && gameData.m_state == State::Playing) {
    Asteroid asteroid = makeAsteroid();
    do {
      asteroid.m_translation = {m_randomDist(m_randomEngine),
                                m_randomDist(m_randomEngine) + 1.5f};
    } while (glm::length(asteroid.m_translation) > 1.0f);

    m_asteroids.emplace_back(asteroid);
    asTimer.restart();
  }
}

Asteroids::Asteroid Asteroids::makeAsteroid(glm::vec2 translation,
                                            float scale) {
  Asteroid asteroid;

  auto &re{m_randomEngine}; // Shortcut

  asteroid.m_polygonSides = 10;

  asteroid.m_color = glm::vec4(0.9f);

  asteroid.m_color.a = 1.0f;
  asteroid.m_rotation = 0.0f;
  asteroid.m_scale = scale;
  asteroid.m_translation = translation;

  asteroid.m_angularVelocity = 10;

  // Get a random direction
  glm::vec2 const direction{m_randomDist(re) * 500, -500};
  asteroid.m_velocity = glm::normalize(direction) / 2.0f;

  // Create geometry data
  std::vector<glm::vec2> positions{{0, 0}};
  auto const step{M_PI * 2 / asteroid.m_polygonSides};
  std::uniform_real_distribution randomRadius(0.8f, 1.0f);
  for (auto const angle : iter::range(0.0, M_PI * 2, step)) {
    auto const radius{randomRadius(re)};
    positions.emplace_back(radius * std::cos(angle), radius * std::sin(angle));
  }
  positions.push_back(positions.at(1));

  // Generate VBO
  abcg::glGenBuffers(1, &asteroid.m_VBO);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, asteroid.m_VBO);
  abcg::glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec2),
                     positions.data(), GL_STATIC_DRAW);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Get location of attributes in the program
  auto const positionAttribute{
      abcg::glGetAttribLocation(m_program, "inPosition")};

  // Create VAO
  abcg::glGenVertexArrays(1, &asteroid.m_VAO);

  // Bind vertex attributes to current VAO
  abcg::glBindVertexArray(asteroid.m_VAO);

  abcg::glBindBuffer(GL_ARRAY_BUFFER, asteroid.m_VBO);
  abcg::glEnableVertexAttribArray(positionAttribute);
  abcg::glVertexAttribPointer(positionAttribute, 2, GL_FLOAT, GL_FALSE, 0,
                              nullptr);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);

  // End of binding to current VAO
  abcg::glBindVertexArray(0);

  return asteroid;
}