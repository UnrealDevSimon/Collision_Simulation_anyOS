#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <iostream>
#include <random>
#include <string>
#include <stdexcept> 
#include <cstdlib>
#include <algorithm>
#include <cmath>

using namespace std;

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

    Particle(float radius, sf::Vector2f position, float gravity, float widthBound, float heightBound, sf::Color color, sf::Vector2f velocity)
        : position(position), gravity(gravity), widthBound(widthBound), heightBound(heightBound), velocity(velocity)
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

    //Check if arguments are convertable
    for (int i = 1; i < argc; ++i)
    {
        cout << "Argument " << i << ": " << argv[i] << endl;

        try
        {
            if (i > 3)
            {
                float intArg = stof(argv[i]);
                cout << "Float argument: " << intArg << endl;
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

    //Instanciate a objects
    vector<Particle*> particles;
    for (int i = 0; i < spawnLimit; i++)
    {
        particles.push_back(new Particle(radius(gen), sf::Vector2f(maxParticleRadius * 2.f, maxParticleRadius * 2.f), gravity, static_cast<float>(windowWidth), static_cast<float>(windowHeight), sf::Color(color(gen), color(gen), color(gen)), sf::Vector2f(vel(gen), 0.0f)));
    }


    sf::Clock clock; // Clock for deltaTime
    int particlesProcessed = 0; // Tracks how many particles we've processed so far
    float timeSinceLastIncrease = 0.0f; // Time passed since last time we increased the processed particles
    float increaseInterval = .1f; // How often to increase particlesProcessed (in seconds)


    // Main loop
    while (window.isOpen())
    {
        // Get time between frames and convert to second
        float deltaTime = clock.restart().asSeconds();

        while (const optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>() || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape))
            {
                cout << particlesProcessed << endl;
                window.close();
            }
        }

        // Increment timeSinceLastIncrease
        timeSinceLastIncrease += deltaTime;

        // Every 'increaseInterval' seconds, increase the number of particles to process
        if (timeSinceLastIncrease >= increaseInterval) {
            particlesProcessed += 1; // Increase the number of particles to process by 1 every interval
            timeSinceLastIncrease = 0.0f; // Reset the timer
        }

        // Process particles incrementally
        for (int i = 0; i <= particlesProcessed && i < particles.size(); ++i)
        {
            Particle* p = particles[i];

            for (int j = 0; j <= particlesProcessed && j < particles.size(); ++j)
            {
                Particle* pOther = particles[j];
                p->particleCollisionCheck(pOther);
            }

            p->update(deltaTime);

        }

        // Clear the window
        window.clear();


        for (int i = 0; i <= particlesProcessed && i < particles.size(); ++i) {
            Particle* p = particles[i];
            //Draw all processed particles to screen
            window.draw(p->pShape);
        }

        // Display the window contents
        window.display();

        // Ensure we don't process more than the total number of particles
        if (particlesProcessed > particles.size())
        {
            particlesProcessed = particles.size();
        }

    }

    // Clean up the particles
    for (Particle* p : particles)
    {
        delete p;
    }

    return 0;
};

void Particle::update(float deltaTime)
{
    velocity.y += gravity * deltaTime;
    if (velocity.x != 0.0f)
        velocity.x *= 0.9999f;

    borderCheck();
    position = pShape.getPosition();
    pShape.setPosition({ position.x + velocity.x, position.y + velocity.y });
}

void Particle::borderCheck()
{
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
}

void Particle::particleCollisionCheck(Particle* other)
{
    sf::Vector2f delta = this->position - other->position;
    float distance = sqrt(delta.x * delta.x + delta.y * delta.y);
    float overlap = (this->pShape.getRadius() + other->pShape.getRadius()) - distance;

    if (distance < (this->pShape.getRadius() + other->pShape.getRadius()))
    {
        if (distance == 0)
            distance += 0.1f;

        sf::Vector2f normal = delta / distance;

        this->position += normal * (overlap / 2.f);
        other->position -= normal * (overlap / 2.f);

        // Calculate relative velocity along the normal direction
        sf::Vector2f relativeVelocity = this->velocity - other->velocity;
        float dotProduct = relativeVelocity.x * normal.x + relativeVelocity.y * normal.y;

        // If the particles are moving towards each other (dotProduct is negative), apply the collision response
        if (dotProduct < 0)
        {
            float coefficientOfRestitution = .75f; // This can be modified to simulate inelastic collisions (less than 1)

            // Calculate the impulse scalar
            float impulse = -(1 + coefficientOfRestitution) * dotProduct;
            impulse /= (1 / this->mass) + (1 / other->mass); // Consider the mass of the particles (if needed)

            // Apply the impulse to each particle's velocity
            sf::Vector2f impulseVector = normal * impulse;

            this->velocity += impulseVector / this->mass;
            other->velocity -= impulseVector / other->mass;
        }
    }
};