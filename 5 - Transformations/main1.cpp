#include "glad/glad.h"
#include "shader.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <iostream>
#include <math.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

const unsigned int SCREEN_WIDTH = 800;
const unsigned int SCREEN_HEIGHT = 600;

//The window we'll be rendering to
SDL_Window* gWindow = NULL;

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
		gWindow = SDL_CreateWindow( "OpenGL with SDL", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN );
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

	//Load texture
	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	// set the texture wrapping/filtering options (on the currently bound texture object)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	SDL_Surface* image = IMG_Load("texture.jpg");
	int mode = GL_RGB;
	if(image->format->BytesPerPixel == 4)
    	mode = GL_RGBA;

	if(image->pixels)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, mode, image->w, image->h, 0, mode, GL_UNSIGNED_BYTE, image->pixels);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
		std::cout << "Failed to load texture" << std::endl;

    Shader ourShader("shader.vert", "shader.frag");

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
	float vertices[] = {
	    // positions          // colors           // texture coords
	     0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // top right
	     0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,   // bottom right
	    -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left
	    -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f    // top left 
	};
    unsigned int indices[] = {  
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };
    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1,&EBO);
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // texture coord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0); 

    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    glBindVertexArray(0);

    //Transformation Matrix
    glm::mat4 transformMatrix; 
	unsigned int transformLoc = glGetUniformLocation(ourShader.ID, "transform"); 	
	float rot = 0.0f, scale = 1.0f, tx = 0.0f, ty = 0.0f;
	bool quit = false;
	SDL_Event e;
	while (!quit)
	{
		while( SDL_PollEvent( &e ) != 0 )
		{
			transformMatrix = glm::mat4(1.0f);
			if( e.key.keysym.sym == SDLK_ESCAPE )
				quit = true;
			if( e.key.keysym.sym == SDLK_q )
				rot -= 2.0f;
			if( e.key.keysym.sym == SDLK_r )
				rot += 2.0f;
			if( e.key.keysym.sym == SDLK_s )
				scale -= 0.05;
			if( e.key.keysym.sym == SDLK_w )
				scale += 0.05;
			if( e.key.keysym.sym == SDLK_UP )
				ty += 0.02f;
			if( e.key.keysym.sym == SDLK_DOWN )
				ty -= 0.02f;
			if( e.key.keysym.sym == SDLK_LEFT )
				tx -= 0.02f;
			if( e.key.keysym.sym == SDLK_RIGHT )
				tx += 0.02f;
			glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);
			ourShader.use();
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture);
			glBindVertexArray(VAO);
			transformMatrix = glm::translate(transformMatrix, glm::vec3(tx, ty, 0.0f));
			transformMatrix = glm::rotate(transformMatrix, glm::radians(rot), glm::vec3(0.0, 0.0, 1.0));
			transformMatrix = glm::scale(transformMatrix, glm::vec3(scale, scale, scale));
			glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transformMatrix));
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
			SDL_GL_SwapWindow( gWindow );
		}
	}

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

	SDL_DestroyWindow( gWindow );
	gWindow = NULL;
	IMG_Quit();
	SDL_Quit();
	return success;
}

