#ifndef GAME_HPP
#define GAME_HPP

#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <vector>
#include <functional>

class Game
{
public:
    explicit Game();
    void run();
private:
    enum
    {
        FPS = 30,
        GRID_WIDTH = 16,
        GRID_HEIGHT = 16,
        TILE_SIZE = 40,
        BORDER_WIDTH = 10,
        SCREEN_WIDTH = TILE_SIZE * GRID_WIDTH + BORDER_WIDTH,
        SCREEN_HEIGHT = TILE_SIZE * GRID_HEIGHT + BORDER_WIDTH
    };
    const std::string TITLE_OF_MAIN_WINDOW { "A*-Pathfinder" };
    const sf::Time mFrameTime { sf::seconds(1.0f /FPS) };
    sf::RenderWindow mWindow;

    struct Node
    {
        int x, y;
        bool isObstacle;
        bool isVisited;
        float localGoal;
        float globalGoal;
        std::vector<Node*> neighbours;
        Node* prev;
    };

    Node mGrid[GRID_HEIGHT][GRID_WIDTH];
    Node* mStart;
    Node* mTarget;
    enum class HeuristicType
    {
        PYTHAGOREAN_DISTANCE,
        MANHATTAN_DISTANCE,
        ZERO
    };
    std::map<HeuristicType, std::function<float(Node*, Node*)>> mHeuristics;
    void initNodes(bool resetObstacles = true);
    void initHeuristics();
    void createConnections();
    bool isValidCoordinates(int x, int y) const;
    void drawLine(int x1, int y1, int x2, int y2, int thickness = 1,
                  sf::Color color = sf::Color::White);
    void drawConnections();
    void drawGrid();
    void drawPath();
    void inputPhase();
    void solve();
    void updatePhase(sf::Time frameTime);
    void renderPhase();
    void centralizeWindow();
};

#endif // GAME_HPP
