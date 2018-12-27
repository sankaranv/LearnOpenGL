#include "glad/glad.h"
#include "shader.h"
#include "camera.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <iostream>
#include <math.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

//The window we'll be rendering to
SDL_Window* gWindow = NULL;

//Camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// Timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

//Lighting
glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

//OpenGL context
SDL_GLContext gContext;

int main()
{
	//Initialization flag
	int success = 0;

	//Initialize SDL
	if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
	{
		std::cout <<  "SDL could not initialize! SDL Error: " << SDL_GetError() << std::endl;
		success = 1;
	}
	else
	{
		//Use OpenGL 3.1 core
		SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
		SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 3 );
		SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );
		
		//Create window
		gWindow = SDL_CreateWindow( "OpenGL with SDL", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCR_WIDTH, SCR_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN );
		if( gWindow == NULL )
		{
			std::cout <<  "Window could not be created! SDL Error: " << SDL_GetError() << std::endl;
			success = 1;
		}
		else
		{
			//Create context
			gContext = SDL_GL_CreateContext( gWindow );
			if( gContext == NULL )
			{
				std::cout <<  "OpenGL context could not be created! SDL Error: " << SDL_GetError() << std::endl;
				success = 1;
			}
			else
			{
				// GLAD: load all OpenGL function pointers
				if (!gladLoadGLLoader(SDL_GL_GetProcAddress))
				{
					std::cout << "Failed to initialize GLAD" << std::endl;
					success = 1;
				}    

				//Use Vsync
				if( SDL_GL_SetSwapInterval(1) < 0 )
				{
					std::cout <<  "Warning: Unable to set VSync! SDL Error: " << SDL_GetError() << std::endl;
				}
				int imgFlags = IMG_INIT_JPG;
				if( !( IMG_Init( imgFlags ) & imgFlags ) )
				{
					std::cout << "SDL_image could not initialize! SDL_image Error: " << IMG_GetError() << std::endl;
					success = 1;
				}
			}
		}
	}

	glEnable(GL_DEPTH_TEST); 

    Shader objShader("shaders/shader.vert", "shaders/object.frag");
    Shader lightShader("shaders/shader.vert", "shaders/light.frag");

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float vertices[] = {
        -0.5f, -0.5f, -0.5f, 
         0.5f, -0.5f, -0.5f,  
         0.5f,  0.5f, -0.5f,  
         0.5f,  0.5f, -0.5f,  
        -0.5f,  0.5f, -0.5f, 
        -0.5f, -0.5f, -0.5f, 

        -0.5f, -0.5f,  0.5f, 
         0.5f, -0.5f,  0.5f,  
         0.5f,  0.5f,  0.5f,  
         0.5f,  0.5f,  0.5f,  
        -0.5f,  0.5f,  0.5f, 
        -0.5f, -0.5f,  0.5f, 

        -0.5f,  0.5f,  0.5f, 
        -0.5f,  0.5f, -0.5f, 
        -0.5f, -0.5f, -0.5f, 
        -0.5f, -0.5f, -0.5f, 
        -0.5f, -0.5f,  0.5f, 
        -0.5f,  0.5f,  0.5f, 

         0.5f,  0.5f,  0.5f,  
         0.5f,  0.5f, -0.5f,  
         0.5f, -0.5f, -0.5f,  
         0.5f, -0.5f, -0.5f,  
         0.5f, -0.5f,  0.5f,  
         0.5f,  0.5f,  0.5f,  

        -0.5f, -0.5f, -0.5f, 
         0.5f, -0.5f, -0.5f,  
         0.5f, -0.5f,  0.5f,  
         0.5f, -0.5f,  0.5f,  
        -0.5f, -0.5f,  0.5f, 
        -0.5f, -0.5f, -0.5f, 

        -0.5f,  0.5f, -0.5f, 
         0.5f,  0.5f, -0.5f,  
         0.5f,  0.5f,  0.5f,  
         0.5f,  0.5f,  0.5f,  
        -0.5f,  0.5f,  0.5f, 
        -0.5f,  0.5f, -0.5f, 
    };

    unsigned int VBO, objVAO;
    glGenVertexArrays(1, &objVAO);
    glGenBuffers(1, &VBO);
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(objVAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    unsigned int lightVAO;
	glGenVertexArrays(1, &lightVAO);
	glBindVertexArray(lightVAO);
	// we only need to bind to the VBO, the container's VBO's data already contains the correct data.
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	// set the vertex attributes (only position data for our lamp)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	bool quit = false;
	float angle;
	SDL_Event e;
	while (!quit)
	{
		while( SDL_PollEvent( &e ) != 0 )
		{
			// Camera speed
			float currentFrame = SDL_GetTicks()/1000.0f;
			deltaTime = currentFrame - lastFrame;
			lastFrame = currentFrame;  

			// A bunch of SDL events for mouse and keyboard input
			if( e.key.keysym.sym == SDLK_ESCAPE || e.type == SDL_QUIT)
				quit = true;
			if( e.key.keysym.sym == SDLK_w )
				camera.ProcessKeyboard(FORWARD, deltaTime);
			if( e.key.keysym.sym == SDLK_s )
				camera.ProcessKeyboard(BACKWARD, deltaTime);
			if( e.key.keysym.sym == SDLK_a )
				camera.ProcessKeyboard(LEFT, deltaTime);
			if( e.key.keysym.sym == SDLK_d )
				camera.ProcessKeyboard(RIGHT, deltaTime);
			if (e.type == SDL_MOUSEMOTION)
			{
				float xPos = e.motion.x;
				float yPos = e.motion.y;
				if(firstMouse)
    			{
        			lastX = xPos;
        			lastY = yPos;
        			firstMouse = false;
    			}
				float delX = (xPos - lastX);
				float delY = (lastY - yPos);
				lastX = xPos;
				lastY = yPos;
				camera.ProcessMouseMovement(delX, delY);
			}
			if (e.type == SDL_MOUSEWHEEL)
			{
				float yPos = e.wheel.y;
				camera.ProcessMouseScroll(yPos);
			}

			glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			objShader.use();
			objShader.setVec3("objectColor", glm::vec3(1.0f, 0.5f, 0.31f));
			objShader.setVec3("lightColor",  glm::vec3(1.0f, 1.0f, 1.0f));

			glm::mat4 viewMatrix = camera.GetViewMatrix();
			glm::mat4 projectionMatrix = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        	objShader.setMat4("projection", projectionMatrix);
        	objShader.setMat4("view", viewMatrix);

        	glm::mat4 modelMatrix = glm::mat4();
			objShader.setMat4("model", modelMatrix);

			glBindVertexArray(objVAO);
			glDrawArrays(GL_TRIANGLES, 0, 36);

			lightShader.use();
			lightShader.setMat4("view", viewMatrix);
			lightShader.setMat4("projection", projectionMatrix);
			modelMatrix = glm::mat4();
			modelMatrix = glm::translate(modelMatrix, lightPos);
			modelMatrix = glm::scale(modelMatrix, glm::vec3(0.2f));
			lightShader.setMat4("model", modelMatrix);

			glBindVertexArray(lightVAO);
			glDrawArrays(GL_TRIANGLES, 0, 36);

			SDL_GL_SwapWindow( gWindow );

		}
	}

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &objVAO);
    glDeleteVertexArrays(1, &lightVAO);
    glDeleteBuffers(1, &VBO);

	SDL_DestroyWindow( gWindow );
	gWindow = NULL;
	IMG_Quit();
	SDL_Quit();
	return success;
}

