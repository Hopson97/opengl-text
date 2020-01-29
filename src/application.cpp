#include "application.h"

#include "gl/gl_errors.h"

#include <cmath>
#include <iostream>

#include "gl/primitive.h"
#include "maths.h"

Application::Application(sf::Window &window)
    : m_window(window)
{
    glViewport(0, 0, 1600, 900);
    glEnable(GL_DEPTH_TEST);
   // glEnable(GL_CULL_FACE);
    //glCullFace(GL_BACK);

    m_quadShader.program.create("static", "static");
    m_quadShader.program.bind();
    m_quadShader.projViewLocation =
        m_quadShader.program.getUniformLocation("projectionViewMatrix");
    m_quadShader.modelLocation =
        m_quadShader.program.getUniformLocation("modelMatrix");

    m_quad = makeQuadVertexArray(1.0f, 1.0f);

    m_projectionMatrix =
        glm::perspective(3.14f / 2.0f, 1600.0f / 900.0f, 0.01f, 100.0f);
        
    m_orthoMatrix = glm::ortho(0.0f, 1600.0f, 0.0f, 900.0f, -1.0f, 1.0f);
    m_texture.create("logo");


    m_text.setPosition({200, 500, 0});
    m_font.init("res/Montserrat-Bold.ttf", 256);
    m_text.setCharSize(32.f);
    m_text.setFont(m_font);
    m_text.setText("Hello world\n");
}

void Application::run()
{
    while (m_window.isOpen()) {
        sf::Event e;
        while (m_window.pollEvent(e)) {
            onEvent(e);
        }

        onInput();

        onUpdate();

        // Render
        glClearColor(0.0, 1.0, 1.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        onRender();

        m_window.display();
    }
}

void Application::onEvent(sf::Event e)
{
    m_keyboard.update(e);
    switch (e.type) {
        case sf::Event::Closed:
            m_window.close();
            break;

        case sf::Event::KeyReleased:
            switch (e.key.code) {
                case sf::Keyboard::Escape:
                    m_window.close();
                    break;

                case sf::Keyboard::L:
                    m_isMouseLocked = !m_isMouseLocked;
                    break;

                default:
                    break;
            }
            break;

        default:
            break;
    }
}

void Application::onInput()
{
    if (!m_isMouseLocked) {
        static auto lastMousePosition = sf::Mouse::getPosition(m_window);
        auto change = sf::Mouse::getPosition(m_window) - lastMousePosition;
        player.rot.x += static_cast<float>(change.y / 10);
        player.rot.y += static_cast<float>(change.x / 10);
        sf::Mouse::setPosition({static_cast<int>(m_window.getSize().x / 2),
                                static_cast<int>(m_window.getSize().y / 2)},
                               m_window);
        lastMousePosition = sf::Mouse::getPosition(m_window);
        player.rot.x = glm::clamp(player.rot.x, -170.0f, 170.0f);
    }

    // Input
    const float SPEED = 0.05f;
    if (m_keyboard.isKeyDown(sf::Keyboard::W)) {
        player.pos += forwardsVector(player.rot) * SPEED;
    }
    else if (m_keyboard.isKeyDown(sf::Keyboard::S)) {
        player.pos += backwardsVector(player.rot) * SPEED;
    }
    if (m_keyboard.isKeyDown(sf::Keyboard::A)) {
        player.pos += leftVector(player.rot) * SPEED;
    }
    else if (m_keyboard.isKeyDown(sf::Keyboard::D)) {
        player.pos += rightVector(player.rot) * SPEED;
    }
}

void Application::onUpdate()
{
}

void Application::onRender()
{
    glm::mat4 projectionViewMatrix =
        createProjectionViewMatrix(player.pos, player.rot, m_projectionMatrix);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  


    // Render the quad
    m_quadShader.program.bind();
    glm::mat4 modelMatrix{1.0f};
    rotateMatrix(modelMatrix, {0.0f, 0.0f, 0.f});
    scaleMatrix(modelMatrix, {2.0f});

    gl::loadUniform(m_quadShader.projViewLocation, projectionViewMatrix);
    gl::loadUniform(m_quadShader.modelLocation, modelMatrix);

    m_texture.bind();
    m_quad.bind();
    m_quad.getDrawable().bindAndDraw();

    //Render text
    gl::loadUniform(m_quadShader.projViewLocation, m_orthoMatrix);
    m_text.render(m_quadShader.modelLocation);
}