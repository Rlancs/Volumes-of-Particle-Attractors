#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

class Attractor
{
public:

	glm::mat4 AttractorData;
	glm::vec3 Position;
	int ID;
	float Size;
	float Mass;
	float MinDistance;
	float MaxDistance;

	glm::vec3 Velocity;
	bool LiquidPhysics;
	int AttractingForces;

	/// <summary>
	/// Constructor for the attractor
	/// </summary>
	/// <param name="position"></param>
	/// <param name="mass"></param>
	Attractor(glm::mat4 data = glm::mat4(1.0f), glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), int id = 0, float size = 1.0f, float mass = 1.0f, float minDist = 1.0f, 
		 float maxDist = 2.0f, glm::vec3 velocity = glm::vec3(0.0f, 0.0f, 0.0f), bool liquidPhysics = false, int atrForce = 0)
	{
		AttractorData = data;
		Position = position;
		ID = id;
		Size = size;
		Mass = mass;
		MinDistance = minDist;
		MaxDistance = maxDist;

		Velocity = velocity;
		LiquidPhysics = liquidPhysics;
		AttractingForces = atrForce;
	}

	/// <summary>
	/// Returns a force to apply to a specified mover
	/// </summary>
	/// <param name="mover"></param>
	/// <param name="modelData"></param>
	/// <returns></returns>
	glm::vec3 Attract(Attractor mover, float gravity) 
	{
		glm::vec3 force = glm::vec3(0.0f, 0.0f, 0.0f);
		float distance = 0.0f;
		float strength = 0.0f;

		force = Position - mover.Position;
		distance = glm::length(force);
		distance = glm::clamp(distance, MinDistance, MaxDistance);
		strength = (gravity * Mass * Mass) / (distance * distance);
		force = glm::normalize(force) * strength;

		return force;
	}

	/// <summary>
	/// Returns a velocity for gravity, based on the attracting forces affecting the attractor
	/// </summary>
	/// <param name="gravity"></param>
	/// <returns></returns>
	glm::vec3 Fall(float gravity)
	{
		float gravityForce = glm::clamp((Mass * gravity) - (Mass * glm::length(Velocity) * AttractingForces), 0.0f, Mass * gravity);
		glm::vec3 gravityVelocity = glm::vec3(0.0f, 0.0f, 0.0f);
		gravityVelocity = -glm::vec3(0.0f, gravityForce, 0.0f);

 		return gravityVelocity;
	}

private:

};