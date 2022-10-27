#ifndef GAMEDATA_HPP_
#define GAMEDATA_HPP_

#include <bitset>

enum class Input { Up, Space };
enum class State { Playing, GameOver };

struct GameData {
  State m_state{State::Playing};
  std::bitset<2> m_input;
  int m_score;
};

#endif