#include "application.h"

#include <cmath>
#include <iostream>

#include "gl/primitive.h"
#include "maths.h"

struct Mesh {
    std::vector<GLfloat> vertices;
    std::vector<GLfloat> textureCoords;
    std::vector<GLuint> indices;
    GLuint icount = 0;
};

void addCharacter(Mesh &mesh, const sf::Glyph &glyph,
                  const sf::Vector2u &imageSize)
{
    float width = static_cast<float>(imageSize.x);
    float height = static_cast<float>(imageSize.y);

    float pad = 1.0f;

    float left = glyph.bounds.left - pad;
    float top = glyph.bounds.top - pad;
    float right = glyph.bounds.left + glyph.bounds.width + pad;
    float bottom = glyph.bounds.top + glyph.bounds.height + pad;

    float u1 = (static_cast<float>(glyph.textureRect.left) - pad) / width;
    float u2 = (static_cast<float>(glyph.textureRect.left + glyph.textureRect.width) + pad) / width;

    float v1 = (static_cast<float>(glyph.textureRect.top) - pad) / height;
    float v2  = (static_cast<float>(glyph.textureRect.top + glyph.textureRect.height) + pad) / height;

    mesh.vertices.insert(mesh.vertices.end(), {
        left, top,
        right, top,
        right, bottom,
        left, bottom
    });

    mesh.textureCoords.insert(mesh.textureCoords.end(), {
        u1, v1,
        u2, v1,
        u2, v2,
        u1, v2,
    });
    mesh.indices.push_back(mesh.icount);
    mesh.indices.push_back(mesh.icount + 1);
    mesh.indices.push_back(mesh.icount + 2);
    mesh.indices.push_back(mesh.icount + 2);
    mesh.indices.push_back(mesh.icount + 3);
    mesh.indices.push_back(mesh.icount);
    mesh.icount += 4;
}

Text createText(const sf::Font &font, const std::string str, int size)
{
    Mesh mesh;

    for (auto i : str) {
        font.getGlyph(i, size, false);
    }
    auto &texture = font.getTexture(size);
    auto image = texture.copyToImage();
   

    Text text;
    text.fontTexture.create(image);
    for (auto i : str) {
        auto &glyph = font.getGlyph(i, size, false);
        addCharacter(mesh, glyph, image.getSize());
    }

    text.vao.bind();
    text.vao.addVertexBuffer(2, mesh.vertices);
    text.vao.addVertexBuffer(2, mesh.textureCoords);
    text.vao.addIndexBuffer(mesh.indices);

    return text;
}

Application::Application(sf::Window &window)
    : m_window(window)
{
    glViewport(0, 0, 1280, 720);
    glEnable(GL_DEPTH_TEST);

    m_quadShader.program.create("static", "static");
    m_quadShader.program.bind();
    m_quadShader.projViewLocation =
        m_quadShader.program.getUniformLocation("projectionViewMatrix");
    m_quadShader.modelLocation =
        m_quadShader.program.getUniformLocation("modelMatrix");

    m_quad = makeQuadVertexArray(1.0f, 1.0f);

    m_projectionMatrix =
        glm::perspective(3.14f / 2.0f, 1280.0f / 720.0f, 0.01f, 100.0f);

    m_texture.create("logo");

    std::string test = "Hello world!";
    m_font.loadFromFile("res/ubuntu.ttf");
    m_text = createText(m_font, test, 64);
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

    // Render the quad
    m_quadShader.program.bind();
    glm::mat4 modelMatrix{1.0f};
    rotateMatrix(modelMatrix, {45.0f, 0.0f, 0.0f});

    gl::loadUniform(m_quadShader.projViewLocation, projectionViewMatrix);
    gl::loadUniform(m_quadShader.modelLocation, modelMatrix);

    // Render
    m_texture.bind();
    m_quad.bind();
    m_quad.getDrawable().bindAndDraw();

    //Render the text
    {
        glm::mat4 modelMatrix{1.0f};
        translateMatrix(modelMatrix, {0, 0, 1});
        gl::loadUniform(m_quadShader.modelLocation, modelMatrix);

        m_text.fontTexture.bind();
        m_text.vao.bind();
        m_text.vao.getDrawable().bindAndDraw();
    }
}