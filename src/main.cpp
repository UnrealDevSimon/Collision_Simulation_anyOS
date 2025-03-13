#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <iostream>
#include <random>
#include <string>
#include <stdexcept> 
#include <cstdlib>
#include <algorithm>
#include <cmath>
#include <unordered_map>

using namespace std;

//Forward declare classes
class Grid;
struct GridCell;
class Particle;

struct GridCell {
    int xPos = 0;
    int yPos = 0;
    unordered_map<Particle*, GridCell*> particlesMap;
};

class Particle
{
public:

    sf::CircleShape pShape;
    sf::Vector2f velocity;
    sf::Vector2f position;
    float gravity;
    float widthBound;
    float heightBound;
    float mass = 1.f;
    size_t cellSize = 0;
    GridCell* myCell = nullptr;

    Particle(float radius, sf::Vector2f position, float gravity, float widthBound, float heightBound, sf::Color color, sf::Vector2f velocity, int cellSize)
        : position(position), gravity(gravity), widthBound(widthBound), heightBound(heightBound), velocity(velocity), cellSize(cellSize)
    {
        pShape.setRadius(radius);
        pShape.setOrigin(sf::Vector2f(radius, radius));
        pShape.setFillColor(color);
        pShape.setPosition(position);
        mass /= radius;
    }

    void update(float deltaTime);
    void borderCheck();
    void particleCollisionCheck(Particle* other);
    void checkCurrentCell(Grid& grid);

};

class Grid {
public:
    int cellSize;
    int width;
    int height;
    bool bParticlesIsColliding = false;
    vector< vector<GridCell> > cells;


    Grid(int maxParticleRadius, uint16_t windowWidth, uint16_t windowHeight)
    {
        cellSize = maxParticleRadius * 5.f;
        width = (windowWidth / cellSize);
        height = (windowHeight / cellSize);

        cells.resize(width, vector<GridCell>(height));

        for (int y = 0; y < width; y++)
        {
            for (int x = 0; x < height; x++)
            {
                cells[y][x].xPos = x;
                cells[y][x].yPos = y;
            }
        }
    }

    void isCellsColliding(GridCell& current, GridCell& other);
    void findCollisionsGrid();
    void addParticle(Particle* p, int pXPos, int pYPos);
    void removeParticle(Particle* p);
};

//Main loop taking command arguments
int main(int argc, char* argv[])
{
    //Check if we have correct amount of arguments needed to run application
    if (argc != 7)
    {
        cerr << "Error: You must provide following arguments: 'windowWidth' 'windowHeight' 'spawnLimit' 'minParticleRadius' 'maxParticleRadius' 'gravity'" << endl;
        return 1;
    }

    //Check if arguments are convertable and valid
    for (int i = 1; i < argc; ++i)
    {
        cout << "Argument " << i << ": " << argv[i] << endl;

        try
        {
            if (i > 3)
            {
                float floatArg = stof(argv[i]);
                cout << "Float argument: " << floatArg << endl;
            }
            else
            {
                int intArg = stoi(argv[i]);
                cout << "Integer argument: " << intArg << endl;
            }
        }
        catch (const invalid_argument& e)
        {
            cerr << "Error: Argument" << i << "is of wrong type!" << endl;
            return 1;
        }
        catch (const out_of_range& e)
        {
            cerr << "Error: The" << i << "argument is out of range!" << endl;
            return 1;
        }
    }

    //Print string to give indication that arguments passed through correctly
    cout << "Application is running!" << endl;

    //Storing command arguments
    unsigned int windowWidth = stoi(argv[1]);
    unsigned int windowHeight = stoi(argv[2]);
    unsigned int spawnLimit = stoi(argv[3]);
    float minParticleRadius = stof(argv[4]);
    float maxParticleRadius = stof(argv[5]);
    float gravity = stof(argv[6]);

    //Initiate random varibles
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> color(0, 255);
    uniform_real_distribution<float> radius(minParticleRadius, maxParticleRadius);
    uniform_real_distribution<float> vel(0.5f, 1.0f);

    sf::RenderWindow window(sf::VideoMode({ windowWidth, windowHeight }), "Simulation window");

    Grid grid(maxParticleRadius, windowWidth, windowHeight);

    //Instanciate particles objects
    vector<Particle*> particles;
    for (int i = 0; i < spawnLimit; i++)
    {
        particles.push_back(new Particle(
            radius(gen),
            sf::Vector2f(maxParticleRadius * 4.f, maxParticleRadius * 4.f),
            gravity,
            static_cast<float>(windowWidth),
            static_cast<float>(windowHeight),
            sf::Color(color(gen), color(gen), color(gen)),
            sf::Vector2f(vel(gen), 0.0f),
            maxParticleRadius * 5.f
        ));
    }

    //Clock for deltaTime
    sf::Clock clock;
    int particlesProcessed = 0; //Tracks how many particles we've processed so far
    float timeSinceLastIncrease = 0.0f; //Time passed since last time we increased processed particles
    float increaseInterval = .05f; //How often to increase particlesProcessed (in seconds)
    float maxDeltaTime = 0.0f;

    //Main loop
    while (window.isOpen())
    {
        //Get time between frames and convert to second
        float deltaTime = clock.restart().asSeconds();

        if (deltaTime > maxDeltaTime)
            maxDeltaTime = deltaTime;

        //Loops if there is an queued event
        while (const optional event = window.pollEvent())
        {
            //Close application if "ESC" key is pressed
            if (event->is<sf::Event::Closed>() || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape))
            {
                cout << "Number of particles spawned: " << particlesProcessed << endl;
                cout << "DeltaTime: " << maxDeltaTime << endl;
                window.close();
            }
        }

        //Increment timeSinceLastIncrease
        timeSinceLastIncrease += deltaTime;

        //Every 'increaseInterval' seconds, increase the number of particles to process
        if (timeSinceLastIncrease >= increaseInterval)
        {
            particlesProcessed += 1; //Increase the number of particles to process by 1 every interval
            timeSinceLastIncrease = 0.0f; //Reset the timer
        }

        grid.findCollisionsGrid();

        //Process particles incrementally
        for (int i = 0; i <= particlesProcessed && i < particles.size(); ++i)
        {
            Particle* p = particles[i];

            p->checkCurrentCell(grid);
            p->update(deltaTime);
        }

        //Clear the window
        window.clear();

        for (int i = 0; i <= particlesProcessed && i < particles.size(); ++i) {
            Particle* p = particles[i];
            //Draw all processed particles to screen
            window.draw(p->pShape);
        }

        //Display the window contents
        window.display();

        //Ensure we don't process more than the total number of particles
        if (particlesProcessed > particles.size())
        {
            particlesProcessed = particles.size();
        }

    }

    //Clean up the particles
    for (Particle* p : particles)
    {
        delete p;
    }

    return 0;
};

void Particle::update(float deltaTime)
{
    //Change velocity with time affected by gravity
    velocity.y += gravity * deltaTime;
    if (velocity.x != 0.0f)
        velocity.x *= 0.9999f;

    //Checks if collision with border and updates position.
    borderCheck();
    position = pShape.getPosition();
    pShape.setPosition({ position.x + velocity.x, position.y + velocity.y });
};

void Particle::borderCheck()
{
    //Check if we collide with border and change velocity accordingly
    if (position.y + pShape.getRadius() > heightBound)
    {
        pShape.setPosition({ position.x, heightBound - pShape.getRadius() });
        velocity.y *= -0.9f;
    }
    else if (position.y - pShape.getRadius() < 0.0f)
    {
        pShape.setPosition({ position.x, 0.0f + pShape.getRadius() });
        velocity.y *= -0.9f;
    }
    else if (position.x + pShape.getRadius() > widthBound)
    {
        pShape.setPosition({ widthBound - pShape.getRadius(), position.y });
        velocity.x *= -0.9f;
    }
    else if (position.x - pShape.getRadius() < 0.0f)
    {
        pShape.setPosition({ 0.0f + pShape.getRadius(), position.y });
        velocity.x *= -0.9f;
    }
};

void Particle::particleCollisionCheck(Particle* other)
{
    //Calculates the distance and overlap between particles
    sf::Vector2f delta = this->position - other->position;
    float distance = sqrt(delta.x * delta.x + delta.y * delta.y);
    float overlap = (this->pShape.getRadius() + other->pShape.getRadius()) - distance;

    //Checks if distance is smaller than both particles radius combined
    if (distance < (this->pShape.getRadius() + other->pShape.getRadius()))
    {
        //Safety check so distance can not be equal to 0.0f
        if (distance == 0)
            distance += 0.1f;

        //Gets the direction
        sf::Vector2f normal = delta / distance;

        //Move both particles away from each other as a safety check
        this->position += normal * (overlap / 2.f);
        other->position -= normal * (overlap / 2.f);

        //Calculate relative velocity along the normal direction
        sf::Vector2f relativeVelocity = this->velocity - other->velocity;
        float dotProduct = relativeVelocity.x * normal.x + relativeVelocity.y * normal.y;

        //If the particles are moving towards each other, apply collision response
        if (dotProduct < 0)
        {
            //This can be modified to simulate inelastic collisions (less than 1)
            float coefficientOfRestitution = .75f;

            //Calculate the force scalar
            float force = -(1 + coefficientOfRestitution) * dotProduct;
            //Taking the mass of the particles into count
            force /= (1 / this->mass) + (1 / other->mass);

            //Apply the force to to the correct direction
            sf::Vector2f forceVector = normal * force;

            //Apply force to ecah particles velocities
            this->velocity += forceVector / this->mass;
            other->velocity -= forceVector / other->mass;
        }
    }
};

void Particle::checkCurrentCell(Grid& grid)
{
    //Calulates particles cellPosition
    int cellXPos = (static_cast<int>(position.x) / cellSize);
    int cellYPos = (static_cast<int>(position.y) / cellSize);

    if (myCell != nullptr)
    {
        //Make sure the position is within bounds
        cellXPos = cellXPos < 0 ? 0 : cellXPos;
        cellXPos = cellXPos > grid.width - 1 ? grid.width - 1 : cellXPos;
        cellYPos = cellYPos < 0 ? 0 : cellYPos;
        cellYPos = cellYPos > grid.height - 1 ? grid.height - 1 : cellYPos;

        //If we have a new cellPosition we add the particle to the new cell
        if (sf::Vector2i(cellXPos, cellYPos) != sf::Vector2i(myCell->xPos, myCell->yPos))
        {
            grid.addParticle(this, cellXPos, cellYPos);
        }
    }
    else
    {
        //If myCell is null we add particle to current cell (Inital cell)
        grid.addParticle(this, cellXPos, cellYPos);
    }
};

void Grid::findCollisionsGrid()
{
    //Looping through all cells
    for (int x = 0; x < width; ++x)
    {
        for (int y = 0; y < height; ++y)
        {
            auto& currentCell = cells[x][y];

            //Check surrounding cells, including itself
            for (int dx = -1; dx <= 1; ++dx)
            {
                for (int dy = -1; dy <= 1; ++dy)
                {
                    //Get the neighboring cell coordinates
                    int nx = x + dx;
                    int ny = y + dy;

                    //Ensure the neighboring cells are within bounds
                    if (nx >= 0 && nx < width && ny >= 0 && ny < height)
                    {
                        //Skip duplicate checks: only compare (x, y) with (nx, ny) if we haven't already checked (nx, ny) with (x, y)
                        if (nx > x || (nx == x && ny >= y))
                        {
                            auto& otherCell = cells[nx][ny];
                            isCellsColliding(currentCell, otherCell);
                        }
                    }
                }
            }
        }
    }
};

void Grid::addParticle(Particle* p, int pXPos, int pYPos)
{
    //Check if myCell has a value and remove particle from current Map
    if (p->myCell != nullptr)
        removeParticle(p);

    p->myCell = &cells[pXPos][pYPos];
    //Adds particle to current Map
    p->myCell->particlesMap.insert(pair<Particle*, GridCell*>(p, p->myCell));
}

void Grid::removeParticle(Particle* p)
{
    //Finds and remove particle from current Map
    if (p->myCell->particlesMap.find(p) != p->myCell->particlesMap.end())
    {
        p->myCell->particlesMap.erase(p);
        p->myCell = nullptr;
    }
};

void Grid::isCellsColliding(GridCell& current, GridCell& other)
{
    //Loops through particlesMap
    for (auto& particlePair : current.particlesMap)
    {
        //Get key value from map index
        Particle* particle = particlePair.first;

        //Loops through neighbouring particleMaps
        for (auto& otherParticlePair : other.particlesMap)
        {
            //Get key value from map index
            Particle* otherParticle = otherParticlePair.first;

            if (particle != otherParticle)
            {
                particle->particleCollisionCheck(otherParticle);
            }
        }
    }
};