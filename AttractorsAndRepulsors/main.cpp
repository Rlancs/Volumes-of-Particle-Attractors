#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <common/shader.h>
#include <common/camera.h>
#include <common/model.h>
#include <common/attractor.h>

#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void RenderSphere();

void CreateAttractorVolume(int volLength, int volWidth, int volHeight);
void SimulateAttractors();
void SimulateRepellers(Shader shader);
void SimulateGravity();
void SimulateCollisions();
void TranslateAndRender(Shader shader);
void CullParticles();

Attractor SpawnAttractor(glm::vec3 attractorPosition, float attractorScale, float attractorCollisionMult,
                         float attractorMass, float attractorMinDist, float attractorMaxDist, std::vector<Attractor> attractorArray);
glm::vec3 AttractorForce(Attractor attractor, Attractor mover, float mult);

// Window size settings
const unsigned int screenWidth = 1920;
const unsigned int screenHeight = 1080;

// Camera settings
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = screenWidth / 2.0f;
float lastY = screenHeight / 2.5f;
bool firstMouse = true;

// Attractor Volume
glm::vec3 volumeStartingPos = glm::vec3(-5.0f, -5.0f, -20.0f);
int volumeLength = 6;
int volumeWidth = 6;
int volumeHeight = 6;

// Attractor and Repeller Arrays
std::vector<Attractor> attractors;
std::vector<Attractor> repellers;

// Attractor Config
float attractorScale = 1.0f;
float attractorCollisionMult = 1.5f;
float attractorMass = 1.0f;
float attractorMinDist = 1.0f;
float attractorMaxDist = 2.5f;

// Repeller Config
float repellerScale = 1.0f;
float repellerCollisionMult = 1.75f;
float repellerMass = 10.0f;
float repellerMinDist = 0.1f;
float repellerMaxDist = 50.0f;
float repellerSpawnDistance = 5.0f;
float repellerSpeed = 15.0f;

// Time
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Lighting
glm::vec3 lightPos(0.0f, 30.0f, 0.0f);

// Physics
float gravity = 2.0f;
glm::vec3 particleCutoff = glm::vec3(30.0f, -30.0f, 30.0f);
bool unlockPhysics = false;

int main()
{
    // Initalize the window
    glfwInit();

    // Window configuration
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create window object
    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "AttractorsAndRepulsors", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "No GLFW window." << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // GLFW mouse capture
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "No GLAD." << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    
    attractors.clear();
    CreateAttractorVolume(volumeLength, volumeWidth, volumeHeight);

    // Shader
    Shader shader("vertexShaderDiffuse.txt", "fragmentShaderDiffuse.txt");
    
    shader.use();
    
    // Set up shader colour (starting with orange)
    shader.setVec3("objectColor", 1.0f, 0.5f, 0.0f);
    shader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
    shader.setVec3("lightPos", lightPos);

    // Render loop
    while (!glfwWindowShouldClose(window))
    {
        // Time variables
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);
        shader.setMat4("projection", projection);

        glm::mat4 view = camera.GetViewMatrix();
        shader.setMat4("view", view);

        // READ THE FUNCTION SUMMARIES
        if (unlockPhysics)
        {
            SimulateAttractors();
        }

        // Blue particle for repellers
        shader.setVec3("objectColor", 0.0f, 0.5f, 1.0f);

        SimulateRepellers(shader);
        
        if (unlockPhysics) 
        {
            SimulateGravity();

            SimulateCollisions();
        }

        // Orange particle for the attractors6
        shader.setVec3("objectColor", 1.0f, 0.5f, 0.0f);

        TranslateAndRender(shader);

        CullParticles();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Terminate the program
    glfwTerminate();
    return 0;
}

/// <summary>
/// Creates the full cubic attractor volume with specified length, width and height
/// </summary>
void CreateAttractorVolume(int volLength, int volWidth, int volHeight) 
{
    float particleDistance = attractorScale * attractorCollisionMult;

    for (size_t l = 0; l < volLength; l++)
    {
        for (size_t w = 0; w < volWidth; w++)
        {
            attractors.push_back(SpawnAttractor(volumeStartingPos + (glm::vec3(l, 0.0f, w) * particleDistance),
                attractorScale, attractorCollisionMult, attractorMass, attractorMinDist, attractorMaxDist, attractors));
            for (size_t h = 1; h < volHeight; h++)
            {
                attractors.push_back(SpawnAttractor(volumeStartingPos + (glm::vec3(l, h, w) * particleDistance),
                    attractorScale, attractorCollisionMult, attractorMass, attractorMinDist, attractorMaxDist, attractors));
            }
        }
    }
}

/// <summary>
/// Each attractor iterates through every other attractor, applying attracting forces to any within its maximum distance.
/// </summary>
void SimulateAttractors()
{
    for (unsigned int i = 0; i < attractors.size(); i++)
    {
        for (unsigned int j = 0; j < attractors.size(); j++)
        {
            if (attractors[i].ID != attractors[j].ID)
            {
                float distance = 0;
                distance = glm::length(attractors[i].Position - attractors[j].Position);

                // If within maximum distance of attraction, force is applied.
                if (distance < attractors[i].MaxDistance)
                {
                    attractors[j].Velocity += AttractorForce(attractors[i], attractors[j], 1.0f);

                    // Increments the number of attracting forces registered on the attractor
                    attractors[j].AttractingForces++;
                }
            }
        }
    }
}

/// <summary>
/// Each repeller iterates through every attractor, applying repelling forces to any that are not strongly affected by other attractors.
/// Also renders repellers.
/// </summary>
void SimulateRepellers(Shader shader) 
{
    for (unsigned int i = 0; i < repellers.size(); i++)
    {
        for (unsigned int j = 0; j < attractors.size(); j++)
        {                
            // Distance between attractor and repeller
            float distance = 0;
            distance = glm::length(repellers[i].Position - attractors[j].Position);

            if (distance < repellers[i].MaxDistance)
            {
                // Attractors resist repelling forces by the multiplication of their mass with the number of attracting forces applied to them
                float attractorResistance = 0;
                attractorResistance = attractors[j].Mass * attractors[j].AttractingForces;

                // Calculate repelling force on the attractor
                glm::vec3 repellingVelocity = glm::vec3(0.0f, 0.0f, 0.0f);
                repellingVelocity = AttractorForce(repellers[i], attractors[j], -0.5f);
                float velocityMag = 0;
                velocityMag = glm::length(repellingVelocity);

                // If repelling force is greater than the attraction resistance, the attractor can be repelled
                if (velocityMag >= attractorResistance)
                {
                    float magnitude = 0;
                    magnitude = attractorResistance / velocityMag;

                    // The attractor now engages in fluid physics when repelled
                    attractors[j].LiquidPhysics = true;

                    // Determined by the magnitude of the repelling force, the attractor has their attracting range reduced.
                    attractors[j].MaxDistance = attractorMaxDist * magnitude;
                    attractors[j].Velocity += repellingVelocity * (1 - magnitude);
                }
                else
                {
                    // Restore to default
                    attractors[j].LiquidPhysics = false;
                    attractors[j].MaxDistance = attractorMaxDist;
                }
            }
        }

        // Move repeller
        if (unlockPhysics)
        {
            repellers[i].AttractorData = glm::translate(repellers[i].AttractorData, repellers[i].Velocity * deltaTime);
            repellers[i].Position = repellers[i].AttractorData[3];
        }

        // Render repellers
        shader.setMat4("model", repellers[i].AttractorData);
        RenderSphere();
    }
}

/// <summary>
/// Iterates through all attractors, applying gravity forces.
/// Gravity forces are scaled to the attractorResistance, as with repelling forces.
/// </summary>
void SimulateGravity() 
{
    for (unsigned int i = 0; i < attractors.size(); i++)
    {
        attractors[i].Velocity += attractors[i].Fall(gravity) * 3.0f;
    }
}

/// <summary>
/// Iterates each attractor with every other attractor, checking if they are colliding.
/// If they are, the velocities of both are checked to see if attractors would overlap once translated
/// If they would, the particles are either locked, or collide fluidly depending on the physics mode
/// </summary>
/// <param name="mover"></param>
void SimulateCollisions() 
{
    for (unsigned int i = 0; i < attractors.size(); i++)
    {
        for (unsigned int j = 0; j < attractors.size(); j++)
        {
            if (attractors[i].ID != attractors[j].ID)
            {
                glm::vec3 vecDiff = glm::vec3(0.0f, 0.0f, 0.0f);
                vecDiff = attractors[j].Position - attractors[i].Position;
                float distance = 0;
                distance = glm::length(vecDiff);

                if (distance < attractors[i].Size)
                {
                    glm::vec3 direction = glm::vec3(0.0f, 0.0f, 0.0f);
                    direction = glm::normalize(vecDiff);
                    glm::vec3 velocityDifference = glm::vec3(0.0f, 0.0f, 0.0f);
                    velocityDifference = attractors[j].Velocity - attractors[i].Velocity;

                    float relativeSpeed = 0;
                    relativeSpeed = glm::dot(velocityDifference, direction);

                    if (relativeSpeed < 0)
                    {
                        if (!attractors[i].LiquidPhysics)
                        {
                            attractors[i].Velocity = glm::vec3(0.0f, 0.0f, 0.0f);
                            attractors[j].Velocity = glm::vec3(0.0f, 0.0f, 0.0f);
                        }
                        else
                        {
                            attractors[j].LiquidPhysics = true;

                            float speedOne = 0;
                            speedOne = (2 * attractors[j].Mass * relativeSpeed) / (attractors[i].Mass + attractors[j].Mass);
                            float speedTwo = 0;
                            speedTwo = (relativeSpeed * (attractors[j].Mass - attractors[i].Mass)) / (attractors[i].Mass + attractors[j].Mass);
                            attractors[i].Velocity += speedOne * direction;
                            attractors[j].Velocity += direction * (speedTwo - relativeSpeed);
                        }
                    }
                }
            }
        }
    }
}

/// <summary>
/// Iterates through all attractors, translating them if physics is enabled.
/// Resets all attractor variables after translation, then renders the attractor.
/// </summary>
/// <param name="shader"></param>
void TranslateAndRender(Shader shader) 
{
    for (unsigned int i = 0; i < attractors.size(); i++)
    {
        if (unlockPhysics)
        {
            attractors[i].AttractorData = glm::translate(attractors[i].AttractorData, attractors[i].Velocity * deltaTime);
            attractors[i].Position = attractors[i].AttractorData[3];
        }

        attractors[i].Velocity = glm::vec3(0.0f, 0.0f, 0.0f);
        attractors[i].AttractingForces = 0;

        shader.setMat4("model", attractors[i].AttractorData);
        RenderSphere();
    }
}

/// <summary>
/// Culls all particles out of range
/// </summary>
void CullParticles() 
{
    for (unsigned int i = 0; i < attractors.size(); i++)
    {
        if (attractors[i].Position.y < particleCutoff.y)
        {
            attractors.erase(attractors.begin() + i);
        }
    }

    for (unsigned int i = 0; i < repellers.size(); i++) 
    {
        if (repellers[i].Position.y < particleCutoff.y || glm::abs(repellers[i].Position.x) > particleCutoff.x ||
            glm::abs(repellers[i].Position.z) > particleCutoff.z)
        {
            repellers.erase(repellers.begin() + i);
        }
    }
}


/// <summary>
/// Spawns and returns an attractor with the given attributes
/// </summary>
/// <param name="attractorPosition"></param>
/// <param name="attractorScale"></param>
/// <param name="attractorCollisionMult"></param>
/// <param name="attractorMass"></param>
/// <param name="attractorMaxDist"></param>
/// <param name="attractorArray"></param>
/// <returns></returns>
Attractor SpawnAttractor(glm::vec3 attractorPosition, float attractorScale, float attractorCollisionMult,
                    float attractorMass, float attractorMinDist, float attractorMaxDist, std::vector<Attractor> attractorArray)
{
    glm::mat4 modelData = glm::mat4(1.0f);

    // Spawns model in front of player view
    modelData = glm::translate(modelData, attractorPosition);

    // Scale model
    modelData = glm::scale(modelData, glm::vec3(attractorScale, attractorScale, attractorScale));

    Attractor attractor(modelData, modelData[3], attractorArray.size(), attractorScale * attractorCollisionMult, attractorMass, attractorMinDist, attractorMaxDist);
    return attractor;
}

/// <summary>
/// Applies a force to the specified mover from the attractor, recording the force if it is stronger than the current velocity of the attractor
/// </summary>
/// <param name="attractor"></param>
glm::vec3 AttractorForce(Attractor attractor, Attractor mover, float mult)
{
      glm::vec3 velocity = glm::vec3(0.0f, 0.0f, 0.0f);
      velocity = attractor.Attract(mover, gravity) * mult;

      return velocity;
}

/// <summary>
/// Processes all input to quit (ESC), move (WASD) and activate physics (F)
/// </summary>
/// <param name="window"></param>
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(Forward, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(Backward, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(Left, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(Right, deltaTime);

    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
        unlockPhysics = true;
}

/// <summary>
/// Executes whenever window size changes, ensuring the viewport matches the new size
/// </summary>
/// <param name="window"></param>
/// <param name="width"></param>
/// <param name="height"></param>
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    { 
        Attractor repeller;
        repeller = (SpawnAttractor(camera.Position + camera.FrontSide * repellerSpawnDistance, repellerScale, repellerCollisionMult,
            repellerMass, repellerMinDist, repellerMaxDist, repellers));
        repeller.Velocity = repellerSpeed * glm::normalize(camera.FrontSide);
        repellers.push_back(repeller);
    }
}

/// <summary>
/// Called whenever the mouse moves, processing camera movement input
/// </summary>
/// <param name="window"></param>
/// <param name="xposIn"></param>
/// <param name="yposIn"></param>
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

/// <summary>
/// Called whenever the mouse scrolls, processing camera zoom input
/// </summary>
/// <param name="window"></param>
/// <param name="xoffset"></param>
/// <param name="yoffset"></param>
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

unsigned int IndexCount;
unsigned int VertexArrayObject = 0;

/// <summary>
/// Renders and builds a sphere
/// </summary>
void RenderSphere()
{
    if (VertexArrayObject == 0)
    {
        glGenVertexArrays(1, &VertexArrayObject);

        unsigned int vertexBufferObject, elementBufferObject;
        glGenBuffers(1, &vertexBufferObject);
        glGenBuffers(1, &elementBufferObject);

        std::vector<glm::vec3> positions;
        std::vector<glm::vec2> UV;
        std::vector<glm::vec3> normals;

        std::vector<unsigned int> indices;

        const unsigned int xSegments = 64;
        const unsigned int ySegments = 64;
        const float pi = glm::pi<float>();

        for (unsigned int x = 0; x <= xSegments; ++x)
        {
            for (unsigned int y = 0; y <= ySegments; ++y)
            {
                float xSegment = (float)x / (float)xSegments;
                float ySegment = (float)y / (float)ySegments;

                UV.push_back(glm::vec2(xSegment, ySegment));

                float xPos = std::cos(xSegment * 2.0f * pi) * std::sin(ySegment * pi);
                float yPos = std::cos(ySegment * pi);
                float zPos = std::sin(xSegment * 2.0f * pi) * std::sin(ySegment * pi);

                positions.push_back(glm::vec3(xPos, yPos, zPos));
                normals.push_back(glm::vec3(xPos, yPos, zPos));
            }
        }

        bool oddRow = false;
        for (unsigned int y = 0; y < ySegments; ++y)
        {
            if (!oddRow) // even rows: y == 0, y == 2; and so on
            {
                for (unsigned int x = 0; x <= xSegments; ++x)
                {
                    indices.push_back(y * (xSegments + 1) + x);
                    indices.push_back((y + 1) * (xSegments + 1) + x);
                }
            }
            else
            {
                for (int x = xSegments; x >= 0; --x)
                {
                    indices.push_back((y + 1) * (xSegments + 1) + x);
                    indices.push_back(y * (xSegments + 1) + x);
                }
            }

            oddRow = !oddRow;
        }

        IndexCount = static_cast<unsigned int>(indices.size());

        std::vector<float> data;
        for (unsigned int i = 0; i < positions.size(); ++i)
        {
            data.push_back(positions[i].x);
            data.push_back(positions[i].y);
            data.push_back(positions[i].z);
            if (normals.size() > 0)
            {
                data.push_back(normals[i].x);
                data.push_back(normals[i].y);
                data.push_back(normals[i].z);
            }
            if (UV.size() > 0)
            {
                data.push_back(UV[i].x);
                data.push_back(UV[i].y);
            }
        }

        glBindVertexArray(VertexArrayObject);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
        glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBufferObject);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        unsigned int stride = (3 + 2 + 3) * sizeof(float);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
    }

    glBindVertexArray(VertexArrayObject);
    glDrawElements(GL_TRIANGLE_STRIP, IndexCount, GL_UNSIGNED_INT, 0);
}