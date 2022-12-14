#include "spikes.hpp"

#include <glm/gtx/fast_trigonometry.hpp>

void Spikes::create(GLuint program, int quantity) {
  destroy();

  m_randomEngine.seed(
      std::chrono::steady_clock::now().time_since_epoch().count());

  m_program = program;

  // Get location of uniforms in the program
  m_colorLoc = abcg::glGetUniformLocation(m_program, "color");
  m_rotationLoc = abcg::glGetUniformLocation(m_program, "rotation");
  m_scaleLoc = abcg::glGetUniformLocation(m_program, "scale");
  m_translationLoc = abcg::glGetUniformLocation(m_program, "translation");

  // Create spikes
  m_spikes.clear();
  m_spikes.resize(quantity);

  float i{0.0f}; // ACUMULADOR PARA SEPARAR OS ESPINHOS NO COMEÇO
  maxQtdTopSpikes = (quantity / 2) - 1; // LIMITANDO ESPINHOS NO TOPO
  qtdTopSpikes = 0;                     // CONTANDO ESPINHOS NO TOPO

  for (auto &spike : m_spikes) {
    spike = makeSpike(); // CRIANDO ESPINHO
    // iniciando o espinho no canto direito e um pouco abaixo
    spike.m_translation = {1.0f + i, -0.05f};
    randomizeTopSpikes(spike);
    i += 0.2f;
  }
}

void Spikes::randomizeTopSpikes(Spike &spike) {
  // gerando espinhos no topo aleatoriamente
  if (qtdTopSpikes < maxQtdTopSpikes) {
    std::uniform_real_distribution<float> randomDists(0.0f, 1.0f);
    auto &re{m_randomEngine};
    // randomicamente coloca o espinho no topo e gira ele
    if (randomDists(re) >= 0.5f) {
      spike.m_translation.y = 0.65f;
      spike.m_rotation = -1.6f;
      qtdTopSpikes++;
    }
  }
}

void Spikes::paint() {
  abcg::glUseProgram(m_program);

  for (auto const &spike : m_spikes) {
    abcg::glBindVertexArray(spike.m_VAO); // BIND DOS ATRIBUTOS DO VERTEX AO VAO
    abcg::glUniform4fv(m_colorLoc, 1, &spike.m_color.r); // VARIÁVEL UNIFORME
    abcg::glUniform1f(m_scaleLoc, spike.m_scale);        // VARIÁVEL UNIFORME
    abcg::glUniform1f(m_rotationLoc, spike.m_rotation);  // VARIÁVEL UNIFORME
    abcg::glUniform2f(m_translationLoc, spike.m_translation.x,
                      spike.m_translation.y); // VARIÁVEL UNIFORME
    abcg::glDrawArrays(GL_TRIANGLE_FAN, 0,
                       spike.m_polygonSides + 2); // DESENHANDO OS PONTOS
    abcg::glBindVertexArray(0);
  }

  abcg::glUseProgram(0);
}

void Spikes::destroy() {
  // liberação dos VBOs e VAOs
  for (auto &spike : m_spikes) {
    abcg::glDeleteBuffers(1, &spike.m_VBO);
    abcg::glDeleteVertexArrays(1, &spike.m_VAO);
  }
}

void Spikes::update(const Ball &ball, GameData &gameData, float deltaTime) {
  for (auto &spike : m_spikes) {
    // condicionando a translação à velocidade da bola
    spike.m_translation.x -= ball.m_velocity.x * deltaTime;

    // reposicionando quando chega no fim da tela
    if (spike.m_translation.x < -1.0f) {
      // calculando uma distância randômica do ponto de origem para separar os
      // espinhos
      std::uniform_real_distribution<float> randomDists(0.0f, 0.9f);
      auto &re{m_randomEngine};
      float randomDist = randomDists(re) + 2.0f;
      spike.m_translation.x += randomDist;

      // aumentando Score
      if (gameData.m_state == State::Playing)
        gameData.m_score += 1;
    }
  }
}

Spikes::Spike Spikes::makeSpike() {
  Spike spike;
  auto &re{m_randomEngine}; // Shortcut

  // Get a random color (actually, a grayscale)
  std::uniform_real_distribution randomIntensity(0.5f, 1.0f);
  spike.m_color = glm::vec4(randomIntensity(re));

  // Create geometry data
  std::vector<glm::vec2> positions{{0, 0}};
  auto const step{M_PI * 2 / spike.m_polygonSides};
  std::uniform_real_distribution randomRadius(0.8f, 1.0f);
  for (auto const angle : iter::range(0.0, M_PI * 2, step)) {
    auto const radius{randomRadius(re)};
    positions.emplace_back(radius * std::cos(angle), radius * std::sin(angle));
  }
  positions.push_back(positions.at(1));

  // GErando VBO
  abcg::glGenBuffers(1, &spike.m_VBO);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, spike.m_VBO);
  abcg::glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec2),
                     positions.data(), GL_STATIC_DRAW);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Localização dos atributos no programa
  auto const positionAttribute{
      abcg::glGetAttribLocation(m_program, "inPosition")};

  // Criando VAO
  abcg::glGenVertexArrays(1, &spike.m_VAO);

  // Bind dos vertices ao VAO
  abcg::glBindVertexArray(spike.m_VAO);

  abcg::glBindBuffer(GL_ARRAY_BUFFER, spike.m_VBO);
  abcg::glEnableVertexAttribArray(positionAttribute);
  abcg::glVertexAttribPointer(positionAttribute, 2, GL_FLOAT, GL_FALSE, 0,
                              nullptr);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Fim do binding
  abcg::glBindVertexArray(0);

  return spike;
}