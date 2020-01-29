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

/**
 * @brief Creates a quad that contains a character, and adds to a mesh
 * 
 * @param mesh The mesh to add the character to
 * @param glyph The glyph being added
 * @param c The character being added
 * @param imageSize The size of the texture atlas
 * @param position The world position to put this char at
 * @param maxHeight The maximum height of the chars
 */
void addCharacter(Mesh &mesh, const sf::Glyph &glyph, char c,
                  const sf::Vector2u &imageSize, const sf::Vector2f& position)
{
    // Short hand for width and height of image
    float width = static_cast<float>(imageSize.x);
    float height = static_cast<float>(imageSize.y);

    //Find the vertex positions of the the quad that will render this character
    float left = glyph.bounds.left;
    float top = glyph.bounds.top;
    float right = glyph.bounds.left + glyph.bounds.width;
    float bottom = glyph.bounds.top + glyph.bounds.height;

    // Find the texture coords in the texture
    float pad = 1.0f;
    float texLeft = (static_cast<float>(glyph.textureRect.left) - pad) / width;
    float texRight = (static_cast<float>(glyph.textureRect.left + glyph.textureRect.width) + pad) / width;
    float texTop = (static_cast<float>(glyph.textureRect.top) - pad) / height;
    float texBottom  = (static_cast<float>(glyph.textureRect.top + glyph.textureRect.height) + pad) / height;

    // Add the vertex positions to the mesh
    float scale = 1;
    mesh.vertices.insert(mesh.vertices.end(), {
         (position.x + left) / scale,  (position.y + top) / scale,
        (position.x + right) / scale, (position.y + top) / scale,
        (position.x + right) / scale, (position.y + bottom) / scale,
        (position.x + left) / scale, (position.y + bottom) / scale,

    });

    // Add the textrue coords to the mesh
    mesh.textureCoords.insert(mesh.textureCoords.end(), {
        texLeft, texTop,
        texRight, texTop,
        texRight, texBottom,
        texLeft, texBottom,
    });

    // Add indices to the mesh
    mesh.indices.push_back(mesh.icount);
    mesh.indices.push_back(mesh.icount + 1);
    mesh.indices.push_back(mesh.icount + 2);
    mesh.indices.push_back(mesh.icount + 2);
    mesh.indices.push_back(mesh.icount + 3);
    mesh.indices.push_back(mesh.icount);
    mesh.icount += 4;
}

/**
 * @brief Create a Text object
 * 
 * @param font The font of the text
 * @param str THe string to set the text to
 * @param size The size of characters
 * @return Text A texture and VAO for that text
 */
Text createText(const sf::Font &font, const std::string str, int size)
{
    Mesh mesh;

    // Pre-render the glyphs of the font for the text
    for (auto i : str) {
        font.getGlyph(i, size, false);
    }
    // Grab the image texture
    auto &texture = font.getTexture(size);
    auto image = texture.copyToImage();

    // The VAO/ Texture
    Text text;
    text.fontTexture.create(image);

    // The character position
    sf::Vector2f position{0, size};

    // The previous character (For kerning offset)
    char prev = 0;

    // Loop through all chars of the string
    for (auto character : str) {
        // Add some kerning offset
        position.x += font.getKerning(prev, character, size);
        std::cout << position.x << std::endl;
        prev = character;
        
        // Handle a new line
        if (character == '\n') {
            position.y += font.getLineSpacing(size);
            position.x = 0;
            continue;
        }
        
        // Get the character glyph and add it to the mesh
        auto &glyph = font.getGlyph(character, size, false);
        addCharacter(mesh, glyph, character, image.getSize(), position);
        position.x += glyph.advance;
    }

    //Bind buffer etc
    text.vao.bind();
    text.vao.addVertexBuffer(2, mesh.vertices);
    text.vao.addVertexBuffer(2, mesh.textureCoords);
    text.vao.addIndexBuffer(mesh.indices);

    return text;
}

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
        
    m_projectionMatrix = glm::ortho(0.0f, 1600.0f, 0.0f, 900.0f, -1.0f, 1.0f);
    m_texture.create("logo");

    std::string test = "Single Player\n\nMultiplayer\n\nSettings\n\nExit Game";
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
    rotateMatrix(modelMatrix, {0.0f, 0.0f, 0.f});
    scaleMatrix(modelMatrix, {2.0f});

    gl::loadUniform(m_quadShader.projViewLocation, m_projectionMatrix);
    gl::loadUniform(m_quadShader.modelLocation, modelMatrix);

    
    // Render
    //m_texture.bind();
    m_quad.bind();
    m_quad.getDrawable().bindAndDraw();

    //Render the text
    m_text.fontTexture.bind();
    auto d = m_text.vao.getDrawable();
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  
    d.bind();
    {
        glm::mat4 modelMatrix{1.0f};
        translateMatrix(modelMatrix, {10, 900, 0});
        rotateMatrix(modelMatrix, {180.0f, 0.0f, 0.f});
        scaleMatrix(modelMatrix, 1.f);
        gl::loadUniform(m_quadShader.modelLocation, modelMatrix);
        d.draw();
    }
}