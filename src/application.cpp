#include "application.h"

#include "gl/gl_errors.h"

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

//Adapted from https://github.com/SFML/SFML/blob/master/src/SFML/Graphics/Text.cpp
// void addGlyphQuad and ensureGeometryUpdate
void addCharacter(Mesh &mesh, const sf::Glyph &glyph, char c,
                  const sf::Vector2u &imageSize, const sf::Vector2f& position, float maxHeight)
{
    // Edge case: Chars that go over a line
    const std::string HALF_DOWN = "yqjpgQ()[]{}@\\/";

    // Edge case: Chars that appear at the top of text
    const std::string FLIP_POS  = "\"'*^";

    // Short hand for width and height of image
    float width = static_cast<float>(imageSize.x);
    float height = static_cast<float>(imageSize.y);

    float pad = 0.1f;

    //Find the vertex positions of the the quad that will render this character
    float left = glyph.bounds.left - pad;
    float top = glyph.bounds.top - pad + (glyph.bounds.height - maxHeight);
    float right = glyph.bounds.left + glyph.bounds.width + pad;
    float bottom = glyph.bounds.top + glyph.bounds.height + pad + (glyph.bounds.height - maxHeight);

    //Handle edge cases
    if (HALF_DOWN.find(c) != std::string::npos) {
        top -= glyph.bounds.height / 2;
        bottom -= glyph.bounds.height / 2;
    }

    if (FLIP_POS.find(c) != std::string::npos) {
        top += maxHeight;
        bottom += maxHeight;
    }

    //Find the texture coords in the texture
    float texLeft = (static_cast<float>(glyph.textureRect.left) - pad) / width;
    float texRight = (static_cast<float>(glyph.textureRect.left + glyph.textureRect.width) + pad) / width;

    float texTop = (static_cast<float>(glyph.textureRect.top) - pad) / height;
    float texBottom  = (static_cast<float>(glyph.textureRect.top + glyph.textureRect.height) + pad) / height;
    std::swap(texTop, texBottom);

    float scale = 256;
    mesh.vertices.insert(mesh.vertices.end(), {
        (position.x + left) / scale,  (position.y + top) / scale,
        (position.x + right) / scale, (position.y + top) / scale,
        (position.x + right) / scale, (position.y + bottom) / scale,
        (position.x + left) / scale, (position.y + bottom) / scale,
    });

    mesh.textureCoords.insert(mesh.textureCoords.end(), {
        texLeft, texTop,
        texRight, texTop,
        texRight, texBottom,
        texLeft, texBottom,
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

    float max = 0;
    for (auto i : str) {
        auto& g  = font.getGlyph(i, size, false);
        max = std::max(max, g.bounds.height);
    }
    auto &texture = font.getTexture(size);
    auto image = texture.copyToImage();
  //  image.flipVertically();
   

    Text text;
    text.fontTexture.create(image);
    sf::Vector2f position;
    char prev = 0;
    float height = 0;
    for (auto i : str) {
        position.x += font.getKerning(prev, i, size);
        prev = i;
        
        if (i == '\n') {
            position.y -= max;
            position.x = 0;
            continue;
        }

        auto &glyph = font.getGlyph(i, size, false);
        addCharacter(mesh, glyph, i, image.getSize(), position, max);
        position.x += glyph.advance;
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
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

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

    std::string test = "Did I ever tell you?\nThe \"story\"!?\n'The quick brown fox jumps over the lazy, bad dog'!![]\n()\n{}\nTHE QUICK BROWN FOX JUMPED OVER THE LAZY DOG.\n-=_+@~$£!/\\*<>,#";
    m_font.loadFromFile("res/ubuntu.ttf");
    m_text = createText(m_font, test, 128);
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
    //m_texture.bind();
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