#include "game.hpp"
#include <cmath>
#include <queue>

Game::Game():
      mWindow(sf::VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT), TITLE_OF_MAIN_WINDOW,
              sf::Style::Close)
{
    mWindow.setFramerateLimit(FPS);
    centralizeWindow();
    initHeuristics();
    initNodes();
    mStart = &mGrid[0][0];
    mGoal = &mGrid[GRID_HEIGHT - 1][GRID_WIDTH - 1];
}

void Game::run()
{
    sf::Clock clock;
    sf::Time timeSinceLastUpdate = sf::Time::Zero;
    while (mWindow.isOpen())
    {
        auto elapsedTime = clock.restart();
        timeSinceLastUpdate += elapsedTime;
        while (timeSinceLastUpdate > mFrameTime) {
            timeSinceLastUpdate -= mFrameTime;
            inputPhase();
            updatePhase(mFrameTime);
        }
        inputPhase();
        updatePhase(mFrameTime);
        renderPhase();
    }
}

void Game::initNodes()
{
    for(int y = 0; y < GRID_HEIGHT; ++y)
    {
        for(int x = 0; x < GRID_WIDTH; ++x)
        {
            mGrid[y][x].x = x;
            mGrid[y][x].y = y;
            mGrid[y][x].isObstacle = false;
            mGrid[y][x].isVisited = false;
            mGrid[y][x].parent = nullptr;
            mGrid[y][x].localGoal = INFINITY;
            mGrid[y][x].globalGoal = INFINITY;
        }
    }
    createConnections();
}

void Game::initHeuristics()
{
    mHeuristics.clear();
    mHeuristics[HeuristicType::MANHATTAN_DISTANCE] = [](Node* a, Node* b){
        return abs(a->x - b->x) + abs(a->y - b->y);
    };

    mHeuristics[HeuristicType::PYTHAGOREAN_DISTANCE] = [](Node* a, Node* b){
        return pow(a->x - b->x, 2) + pow(a->y - b->y, 2);
    };

    mHeuristics[HeuristicType::ZERO] = [](Node* a, Node* b){
        return 0;
    };
}

void Game::createConnections()
{
    const int dx[] { 1, 0, -1, 0 };
    const int dy[] { 0, 1, 0, -1 };
    for(int y = 0; y < GRID_HEIGHT; ++y)
    {
        for(int x = 0; x < GRID_WIDTH; ++x)
        {
            mGrid[y][x].neighbours.clear();
            for(int dir = 0; dir < 4; ++dir)
            {
                int nx = x + dx[dir];
                int ny = y + dy[dir];
                if(isValidCoordinates(nx, ny) && !mGrid[ny][nx].isObstacle)
                {
                    mGrid[y][x].neighbours.push_back(&mGrid[ny][nx]);
                }
            }
        }
    }
}

bool Game::isValidCoordinates(int x, int y) const
{
    return x >= 0 && x < GRID_WIDTH && y >= 0 && y < GRID_HEIGHT;
}

void Game::drawLine(int x1, int y1, int x2, int y2, int thickness, sf::Color color)
{
    sf::RectangleShape line;
    line.setPosition(x1, y1);
    auto length = sqrt(pow(x1 - x2, 2) + pow(y1 - y2, 2));
    line.setSize(sf::Vector2f(length, thickness / 2));
    line.setFillColor(color);
    auto angle = atan2(y2 - y1, x2 - x1) / M_PI * 180.f;
    line.setRotation(angle);
    mWindow.draw(line);
}

void Game::inputPhase()
{
    sf::Event event;
    while (mWindow.pollEvent(event))
    {
        if(event.type == sf::Event::Closed)
        {
            mWindow.close();
        }
        else if(event.type == sf::Event::MouseButtonReleased)
        {
            int mx = event.mouseButton.x / TILE_SIZE;
            int my = event.mouseButton.y / TILE_SIZE;
            if(isValidCoordinates(mx, my))
            {
                if(event.mouseButton.button == sf::Mouse::Left)
                {
                    mStart = &mGrid[my][mx];
                }
                else if(event.mouseButton.button == sf::Mouse::Middle)
                {
                    mGrid[my][mx].isObstacle = !mGrid[my][mx].isObstacle;
                }
                else if(event.mouseButton.button == sf::Mouse::Right)
                {
                    mGoal = &mGrid[my][mx];
                }
                solve();
            }

        }
    }
}

void Game::solve()
{
    initNodes();
    std::priority_queue<Node*, std::vector<Node*>, std::function<bool(Node*, Node*)>> openSet(
        [](Node* a, Node* b){
               return a->globalGoal < b->globalGoal;
        });

    openSet.push(mStart);
    auto heuristic = mHeuristics[HeuristicType::MANHATTAN_DISTANCE];
    while (!openSet.empty()) {
        auto current = openSet.top();
        current->isVisited = true;
        for(auto &neighbour: current->neighbours)
        {
            if(!neighbour->isVisited)
            {
                openSet.push(neighbour);
                float possiblyLowerGoal = current->localGoal
                        + heuristic(current, neighbour);
                if(possiblyLowerGoal < neighbour->localGoal)
                {
                    neighbour->parent = current;
                    neighbour->localGoal = possiblyLowerGoal;
                    neighbour->globalGoal = possiblyLowerGoal + heuristic(neighbour, mGoal);
                }
            }
        }
        openSet.pop();
    }
}

void Game::updatePhase(sf::Time frameTime)
{

}

void Game::renderPhase()
{
    mWindow.clear();
    for(int y = 0; y < GRID_HEIGHT; ++y)
    {
        for(int x = 0; x < GRID_WIDTH; ++x)
        {
            auto neighbours = mGrid[y][x].neighbours;
            for(auto &n: neighbours)
            {
                /*sf::VertexArray connection(sf::LineStrip, 2);
                connection[0].position.x = x * TILE_SIZE + TILE_SIZE / 2 + BORDER_WIDTH / 2;
                connection[0].position.y = y * TILE_SIZE + TILE_SIZE / 2 + BORDER_WIDTH / 2;
                connection[0].color = sf::Color::Blue;
                connection[1].position.x = n->x * TILE_SIZE + TILE_SIZE / 2 + BORDER_WIDTH / 2;
                connection[1].position.y = n->y * TILE_SIZE + TILE_SIZE / 2 + BORDER_WIDTH / 2;
                connection[1].color = sf::Color::Blue;
                mWindow.draw(connection);*/
                int x1 = mGrid[y][x].x * TILE_SIZE + TILE_SIZE / 2 + BORDER_WIDTH / 2;
                int y1 = mGrid[y][x].y * TILE_SIZE + TILE_SIZE / 2 + BORDER_WIDTH / 2;
                int x2 = n->x * TILE_SIZE + TILE_SIZE / 2 + BORDER_WIDTH / 2;
                int y2 = n->y * TILE_SIZE + TILE_SIZE / 2  + BORDER_WIDTH / 2;
                drawLine(x1, y1, x2, y2, 4, sf::Color::Blue);
            }
        }

    }

    for(int y = 0; y < GRID_HEIGHT; ++y)
    {
        for(int x = 0; x < GRID_WIDTH; ++x)
        {
            sf::RectangleShape node;
            node.setPosition(x * TILE_SIZE + BORDER_WIDTH, y * TILE_SIZE + BORDER_WIDTH);
            node.setSize(sf::Vector2f(TILE_SIZE - BORDER_WIDTH, TILE_SIZE - BORDER_WIDTH));
            node.setFillColor(sf::Color::Blue);
            if(x == mStart->x && y == mStart->y)
            {
                node.setFillColor(sf::Color::Green);
            }
            else if(x == mGoal->x && y == mGoal->y)
            {
                node.setFillColor(sf::Color::Red);
            }

            if(mGrid[y][x].isVisited)
            {
                node.setFillColor(sf::Color::Cyan);
            }if(mGrid[y][x].isObstacle)
            {
                node.setFillColor(sf::Color(255,233,127));
            }

            if(mGrid[y][x].isObstacle)
            {
                node.setFillColor(sf::Color(255,233,127));
            }
            mWindow.draw(node);
        }
    }

    if(mGoal->parent != nullptr)
    {
        Node* curr = mGoal;
        while(curr->parent)
        {
            int x1 = curr->x * TILE_SIZE + TILE_SIZE / 2 + BORDER_WIDTH / 2;
            int y1 = curr->y * TILE_SIZE + TILE_SIZE / 2 + BORDER_WIDTH / 2;
            int x2 = curr->parent->x * TILE_SIZE + TILE_SIZE / 2 + BORDER_WIDTH / 2;
            int y2 = curr->parent->y * TILE_SIZE + TILE_SIZE / 2  + BORDER_WIDTH / 2;
            drawLine(x1, y1, x2, y2, 8, sf::Color::Yellow);
            curr = curr->parent;
        }
    }
    mWindow.display();
}

void Game::centralizeWindow()
{
    const int screenWidth = sf::VideoMode::getDesktopMode().width;
    const int screenHeight = sf::VideoMode::getDesktopMode().height;
    mWindow.setPosition(sf::Vector2i((screenWidth - SCREEN_WIDTH) / 2,
                                         (screenHeight - SCREEN_HEIGHT) / 2));
}
