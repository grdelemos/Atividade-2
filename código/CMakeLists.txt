project(atividade2)
add_executable(${PROJECT_NAME} main.cpp window.cpp asteroids.cpp
                               ship.cpp starlayers.cpp)
enable_abcg(${PROJECT_NAME})
