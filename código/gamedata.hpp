#ifndef GAMEDATA_HPP_
#define GAMEDATA_HPP_

#include <bitset>

enum class Input { Right, Left };
enum class State { Start, Playing, GameOver, Win };

struct GameData {
  State m_state{State::Playing};
  std::bitset<5> m_input;
};

#endif