# Atividade 2
Computação Gráfica 


Gustavo Gualberto Rocha de Lemos 


 **RA**: 21008313


O programa é um jogo chamado Space Blocks, onde o usuário deve rebater asteróides evitando que eles caiam abaixo da sua nave. Com o passar do tempo vão aparecendo mais asteróides, e o usuário não pode deixar nenhum passar por 40 segundos para ganhar o jogo. Os controles da nave são feitos pelas setas esquerda e direita do teclado, ou as teclas A e D. Existem dois modos de dificuldade: Fácil e Difícil, que alteram a velocidade da nave e o intervalo em que aparecem novos asteróides.
O projeto está dividido em 4 módulos principais: asteroids, ship, starlayers e window.
O módulo Window contém o funcionamento principal do programa: ele carrega os shaders para renderizar os objetos, a fonte utilizada pela UI e define os comandos que serão aceitos pelo jogo, nesse caso sendo esquerda e direita. O jogo começa no estado Start, que tem instruções sobre como jogar, e dois botões: um para jogar no modo fácil, na cor verde e outro para jogar no modo difícil, na cor vermelha. Ao clicar em um dos dois botões a variável booleana "hard" é definida como true ou false, dependendo do que foi escolhido, e a função restart() é chamada, para iniciar o jogo. Quando o jogo está no estado "Playing", a UI mostra quantos segundos faltam para o usuário sobreviver. Isso é feito com a variável count:

    auto count{
        fmt::format("{}", (40 - std::round(m_restartWaitTimer.elapsed())))};

o timer "m_restartWaitTimer" é reiniciado no começo da partida, e a variável count é o resultado do número 40 menos o tempo passado desde o seu reinício. O resultado é um contador regressivo, para mostrar ao jogador quanto tempo falta. Quando esse contador chega a 0, significa que passaram 40 segundos e o jogo acabou. A função checkCOndition() faz esse controle:

    void Window::checkWinCondition() {
        if (m_restartWaitTimer.elapsed() > 40) {
        m_gameData.m_state = State::Win;
        }
    }

Para que a nave possa rebater os meteoros, há a detecção de colisão entre esses elementos:

    if (distance < m_ship.m_scale * 0.9f + asteroid.m_scale * 0.85f) {

      asteroid.m_velocity =
          glm::normalize(glm::vec2{m_randomDist(re) * 500, +500}) / 2.0f;
      asteroid.m_velocity += 5.0f * deltaTime;
      asteroid.m_translation += asteroid.m_velocity * deltaTime;
    }

caso a colisão seja detectada, o asteróide é mandado de volta para cima, com uma direção no eixo x entre -1 e 1 definida aleatoriamente por "m_randomDist(re)", que é multiplicada por 500. Para o asteróide subir na tela, a direção no eixo y é sempre 500 positivo.

O jogo começa com 3 asteróides, mas para aumentar o desafio mais vão sendo adicionados com o tempo. Isso é feito pela função update() de Asteroids:

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

O intervalo de criação de novos asteróides pode ser de 3 ou 5 segundos, dependendo do valor da variável "hard". Os asteróides giram com uma velocidade de rotação fixa, têm cor fixa o mesmo número de lados e a sua orientação inicial é sempre para baixo no eixo y, mas aleatória no eixo x. Todos esses atributos, mais o seu VBO e VAO são definidos na função makeAsteroid. A posição inicial deles é aleatória no eixo x, mas sempre na parte de cima da tela, para dar tempo do jogador vê-los. Para evitar que os asteróides saiam da tela, a função também põe limite nas laterais da tela e na parte de cima, para que se o asteróide chegar nelas ele vá para a direção oposta. Caso ele passe da posição -0.999 no eixo y, isso significa que ele passou da nave e o jogo termina, passando para o estado GameOver.

A nave é criada na função create de Ship, mas só aparece na tela com a função paint, que só mostra a nave quando o estado do jogo é "playing". A sua posição inicial fica no centro do eixo x, mas na posição -0.7 no eixo y, para que a nave fique na parte de baixo da tela. Os triângulos que representam o fogo dos exaustores da nave aparecem sempre que um comando de movimento da nave acontece. O movimento da nave é definido na sua função update:

    void Ship::update(GameData const &gameData, bool hard) {

        double speed;
        if (hard == true) {
            speed = 18.0;
        } else {
            speed = 16.0;
        }

        if (gameData.m_input[gsl::narrow<size_t>(Input::Left)]) {
            if (m_translation.x > -0.85) {
            m_translation.x -= (1 / speed);
            }
        }
        if (gameData.m_input[gsl::narrow<size_t>(Input::Right)]) {
            if (m_translation.x < 0.85) {
            m_translation.x += (1 / speed);
            }
        }
    }

A variável "speed" determina o incremento ou decremento na translação da nave, e por isso afeta a velocidade com que a nave se movimenta pela tela. Ela também é definida dependendo da opção de jogo fácil ou difícil. Existem limites nas laterais da tela para evitar que a nave saia da área visível, com os valores -0.85 e 0.85.

O módulo starlayers define a criação e movimentação das estrelas de fundo. Elas são criadas com a quantidade inicial de 25, e a sua direção é definida pela variável direction:

    glm::vec2 direction{1.0f, 1.0f};
    if (gameData.m_state != State::Playing) {
        direction = {0.5f, 0.0f};
    }

Caso o jogo não esteja no estado "Playing", a direção das estrelas é da direita para a esquerda porque o vetor direction multiplica a sua translação por 0.5 no eixo x, e quando o jogo está no estado "Playing", elas se movem diagonalmente do canto superior direito para o canto inferior esquerdo, para ajudar a dar a sensação de movimento ao jogo.