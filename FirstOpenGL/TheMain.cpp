#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>			
#include <glm/vec3.hpp> 
#include <glm/vec4.hpp> 
#include <glm/mat4x4.hpp> 
#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtc/type_ptr.hpp> 
#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <sstream>		
#include <string>
#include <vector>		
#include "Utilities.h"
#include "ModelUtilities.h"
#include "cMesh.h"
#include "cShaderManager.h" 
#include "cGameObject.h"
#include "cVAOMeshManager.h";

#include<vector>
#include"myInterface.h"
#include"myBulletPhysics.h"
#include"myPhysics.h"

int uni=0;

std::vector<glm::vec3> disp;
std::vector<glm::vec3> rot;
std::vector<int> collision;
myInterface::force wforce;
myInterface::force sforce;
myInterface::force upforce;
myInterface::force leftforce;
myInterface::force rightforce;
float camerafactor = 5.0f;
bool debugrender = 0;
//Game Objects get stored here
std::vector< cGameObject* >  g_vecGameObjects;
float theta = 180;
//Camera Params
glm::vec3 g_cameraXYZ = glm::vec3(0.0f, 0.0f, 10.0f);	// 5 units "down" z
glm::vec3 g_cameraTarget_XYZ = glm::vec3(0.0f, 0.0f, 0.0f);
//Shader and Mesh manager
cVAOMeshManager* g_pVAOManager = 0;
cShaderManager*		g_pShaderManager;


glm::vec3 direct(glm::vec3 dir)
{
	if (dir.x >= dir.y && dir.x >= dir.z)
	{
		return dir / abs(dir.x);
	}
	else if (dir.y >= dir.x && dir.y >= dir.z)
	{
		return dir / abs(dir.y);
	}
	else
	{
		return dir / abs(dir.z);
	}
}

//Error Callback
static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}



void makeObject(glm::vec3 pos, glm::vec3 or, glm::vec3 or2, float scale, glm::vec4 color, std::string name, eTypeOfObject type, bool phys )
{
	cGameObject* pTempGO = new cGameObject();
	pTempGO->position = pos;
	
	pTempGO->orientation = or;	// Degrees			
	pTempGO->orientation2 = glm::radians(or2) ;										//		pTempGO->orientation2.z = glm::degrees(45.0f);	// Degrees
	
	pTempGO->scale = scale;

	pTempGO->diffuseColour = color;

	pTempGO->meshName = name;		// Was teapot
	pTempGO->typeOfObject = type;
	// ***
	pTempGO->bIsUpdatedInPhysics = phys;
	// ***
	::g_vecGameObjects.push_back(pTempGO);		// Fastest way to add
}


void LoadPly(std::string name, std::string file)
{
	cMesh testMesh;
	testMesh.name = name;
	if (!LoadPlyFileIntoMesh(file, testMesh))
	{
		std::cout << "Didn't load model" << std::endl;
		// do something??
	}
	if (!::g_pVAOManager->loadMeshIntoVAO(testMesh, ::g_pShaderManager->getIDFromFriendlyName("mySexyShader")))
	{
		std::cout << "Could not load mesh into VAO" << std::endl;
	}
}

bool SceneDescriptor(std::string file)
{
	std::string typer;
	glm::vec3 pos; glm::vec3 or ; glm::vec3 or2; float scale;  glm::vec4 color; std::string name; eTypeOfObject type; bool phys;
	std::ifstream plyFile(file.c_str());
	if (!plyFile.is_open())
	{	// Didn't open file, so return
		return false;
	}

	
	ReadFileToToken(plyFile, "models");
	int count;
	plyFile >> count;
	
	for (int i = 1; i <= count; i++)
	{
		
		ReadFileToToken(plyFile, std::to_string(i));
		ReadFileToToken(plyFile, "position");
		plyFile >> pos.x;
		plyFile >> pos.y;
		plyFile >> pos.z;
		
		ReadFileToToken(plyFile, "orientation");
		plyFile >> or.x;
		plyFile >> or.y;
		plyFile >> or.z;

		
		ReadFileToToken(plyFile, "orientation2");
		plyFile >> or2.x;
		plyFile >> or2.y;
		plyFile >> or2.z;
		
		ReadFileToToken(plyFile, "scale");
		plyFile >> scale;
		
		ReadFileToToken(plyFile, "color");
		plyFile >> color.a;
		plyFile >> color.r;
		plyFile >> color.g;
		plyFile >> color.b;
	
		ReadFileToToken(plyFile, "name");
		plyFile >> name;
		
		ReadFileToToken(plyFile, "type");
		plyFile >> typer;
		if (typer == "sphere")
		{
			type = eTypeOfObject::SPHERE;
		}
		else
		{
			type = eTypeOfObject::PLANE;
		}

		ReadFileToToken(plyFile, "physics");
		plyFile >> phys;
		
		makeObject(pos, or , or2, scale, color, name, type, phys);

	}

}


void handlekeys(float deltaTime)
{
	
	if (GetKeyState('W') < 0)
	{
		wforce.on = 1;
	}
	else
	{
		wforce.on = 0;
	}

	if (GetKeyState('S') < 0)
	{
		sforce.on = 1;
	}
	else
	{
		sforce.on = 0;
	}
	if (GetKeyState('A') < 0)
	{
		leftforce.on = 1;
	}
	else
	{
		leftforce.on = 0;
	}


	if (GetKeyState('D') < 0)
	{
		rightforce.on = 1;
	}
	else
	{
		rightforce.on = 0;
	}

	if (GetKeyState('O') < 0)
	{
		upforce.on = 1;
	}
	else
	{
		upforce.on = 0;
	}
	if (GetKeyState(0x25) < 0)
	{
		theta += (float)deltaTime;
	}

	if (GetKeyState(0x27) < 0)
	{
		theta -= (float)deltaTime;
	}

	if (GetKeyState(0x26) < 0)
	{
		camerafactor += 7 * (float)deltaTime;
	}

	if (GetKeyState(0x28) < 0)
	{
		camerafactor -= 7 * (float)deltaTime;
	}


	if (GetKeyState('9') < 0)
	{
		debugrender = 1;
		std::cout << "DEBUG RENDER ACTIVATED" << '\a';
	}
	if (GetKeyState('0') < 0)
	{
		debugrender = 0;
		std::cout << "DEBUG RENDER DEACTIVATED";
	}

}

bool changePhysicsEngine = 0;
//Main
int main(void)
{
	
	myInterface* io = new myBulletPhysics();
	myInterface* io2 = new myPhysics();
	std::vector<myInterface::object1> my = io->InitializePhysics("Scene.txt");
	//io2->InitializePhysics("Scene.txt");


	for (int i = 0; i < my.size(); i++)
	{
		disp.push_back(glm::vec3(0, 0, 0));
		rot.push_back(glm::vec3(0, 0, 0));
	}

	for (int i = 0; i < my.size(); i++)
	{
		std::cout << i <<my[i].name << "     "<< my[i].pos.x << std:: endl;
	}
	GLFWwindow* window;
	GLuint vertex_buffer, vertex_shader, fragment_shader, program;
	GLint mvp_location, vpos_location, vcol_location;
	int height;	/* default */
	int width;	// default
	std::string title;


	glfwSetErrorCallback(error_callback);

	if (!glfwInit())
		exit(EXIT_FAILURE);


	std::ifstream infoFile("config.txt");
	if (!infoFile.is_open())
	{	// File didn't open...
		std::cout << "Can't find config file" << std::endl;
		std::cout << "Using defaults" << std::endl;
	}
	else
	{	// File DID open, so read it... 
		std::string a;

		infoFile >> a;	// "Game"	//std::cin >> a;
		infoFile >> a;	// "Config"
		infoFile >> a;	// "width"

		infoFile >> width;	// 1080

		infoFile >> a;	// "height"

		infoFile >> height;	// 768

		infoFile >> a;		// Title_Start

		std::stringstream ssTitle;		// Inside "sstream"
		bool bKeepReading = true;
		do
		{
			infoFile >> a;		// Title_End??
			if (a != "Title_End")
			{
				ssTitle << a << " ";
			}
			else
			{	// it IS the end! 
				bKeepReading = false;
				title = ssTitle.str();
			}
		} while (bKeepReading);


	}//if ( ! infoFile.is_open() )



	if (!SceneDescriptor("Scene.txt"))
	{
		std::cout << "ERROR";
	}

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

	// C++ string
	// C no strings. Sorry. char    char name[7] = "Michael\0";
    window = glfwCreateWindow( width, height, 
							   title.c_str(), 
							   NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval(1);

	std::cout << glGetString(GL_VENDOR) << " " 
		<< glGetString(GL_RENDERER) << ", " 
		<< glGetString(GL_VERSION) << std::endl;
	std::cout << "Shader language version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

	
	::g_pShaderManager = new cShaderManager();

	cShaderManager::cShader vertShader;
	cShaderManager::cShader fragShader;

	vertShader.fileName = "simpleVert.glsl";	
	fragShader.fileName = "simpleFrag.glsl"; 

	::g_pShaderManager->setBasePath( "assets//shaders//" );

	// Shader objects are passed by reference so that
	//	we can look at the results if we wanted to. 
	if ( ! ::g_pShaderManager->createProgramFromFile(
		        "mySexyShader", vertShader, fragShader ) )
	{
		std::cout << "Oh no! All is lost!!! Blame Loki!!!" << std::endl;
		std::cout << ::g_pShaderManager->getLastError() << std::endl;
		// Should we exit?? 
		return -1;	
//		exit(
	}
	std::cout << "The shaders comipled and linked OK" << std::endl;


	// Load models
	::g_pVAOManager = new cVAOMeshManager();
	LoadPly("PlaneXZ", "Flat_XZ_Plane_xyz.ply");
	LoadPly("SphereRadius1", "Sphereply_xyz.ply");


	GLint currentProgID = ::g_pShaderManager->getIDFromFriendlyName( "mySexyShader" );

	mvp_location = glGetUniformLocation(currentProgID, "MVP");		// program, "MVP");

	glEnable( GL_DEPTH );

	// Gets the "current" time "tick" or "step"
	double lastTimeStep = glfwGetTime();
	// Main game or application loop
	while ( ! glfwWindowShouldClose(window) )
    {
        float ratio;
        int width, height;
        glm::mat4x4 m, p, mvp;			//  mat4x4 m, p, mvp;

        glfwGetFramebufferSize(window, &width, &height);
        ratio = width / (float) height;
        glViewport(0, 0, width, height);

        glClear(GL_COLOR_BUFFER_BIT );

		unsigned int sizeOfVector = ::g_vecGameObjects.size();	//*****//
		for ( int index = 0; index != sizeOfVector; index++ )
		{
			// Is there a game object? 
			if ( ::g_vecGameObjects[index] == 0 )	//if ( ::g_GameObjects[index] == 0 )
			{	// Nothing to draw
				continue;		// Skip all for loop code and go to next
			}

			// Was near the draw call, but we need the mesh name
			std::string meshToDraw = ::g_vecGameObjects[index]->meshName;		

			sVAOInfo VAODrawInfo;
			if ( ::g_pVAOManager->lookupVAOFromName( meshToDraw, VAODrawInfo ) == false )
			{	// Didn't find mesh
				continue;
			}



			// There IS something to draw

			m = glm::mat4x4(1.0f);	//		mat4x4_identity(m);

			glm::mat4 matRreRotZ = glm::mat4x4(1.0f);
			matRreRotZ = glm::rotate( matRreRotZ, ::g_vecGameObjects[index]->orientation.z, 
								     glm::vec3(0.0f, 0.0f, 1.0f) );
			m = m * matRreRotZ;

			glm::mat4 trans = glm::mat4x4(1.0f);
			trans = glm::translate( trans, 
								    ::g_vecGameObjects[index]->position );
			m = m * trans; 

			glm::mat4 matPostRotZ = glm::mat4x4(1.0f);
			matPostRotZ = glm::rotate( matPostRotZ, ::g_vecGameObjects[index]->orientation2.z, 
								     glm::vec3(0.0f, 0.0f, 1.0f) );
			m = m * matPostRotZ;

//			::g_vecGameObjects[index]->orientation2.y += 0.01f;

			glm::mat4 matPostRotY = glm::mat4x4(1.0f);
			matPostRotY = glm::rotate( matPostRotY, ::g_vecGameObjects[index]->orientation2.y, 
								     glm::vec3(0.0f, 1.0f, 0.0f) );
			m = m * matPostRotY;


			glm::mat4 matPostRotX = glm::mat4x4(1.0f);
			matPostRotX = glm::rotate( matPostRotX, ::g_vecGameObjects[index]->orientation2.x, 
								     glm::vec3(1.0f, 0.0f, 0.0f) );
			m = m * matPostRotX;
			
			float finalScale = ::g_vecGameObjects[index]->scale;

			glm::mat4 matScale = glm::mat4x4(1.0f);
			matScale = glm::scale( matScale, 
								   glm::vec3( finalScale,
								              finalScale,
								              finalScale ) );
			m = m * matScale;

			p = glm::perspective( 0.6f,			// FOV
								  ratio,		// Aspect ratio
								  0.1f,			// Near (as big as possible)
								  1000.0f );	// Far (as small as possible)

			// View or "camera" matrix
			glm::mat4 v = glm::mat4(1.0f);	// identity



			//Camera Handler
			glm::vec3 g_cameraXYZ = { ::g_vecGameObjects[1]->position.x + camerafactor*cosf(theta), ::g_vecGameObjects[1]->position.y + 0.5*camerafactor, ::g_vecGameObjects[1]->position.z + camerafactor * sin(theta) };	// 5 units "down" z
			glm::vec3 g_cameraTarget_XYZ = ::g_vecGameObjects[1]->position;
			upforce.strength = 0.1f;
			upforce.direction = glm::vec3(0, 1, 0);
			wforce.direction = (g_cameraTarget_XYZ - g_cameraXYZ);
			wforce.strength = 0.5f;
			leftforce.direction = (glm::cross(glm::vec3(0, 1, 0), g_cameraTarget_XYZ - g_cameraXYZ));
			rightforce.direction = -leftforce.direction; rightforce.strength = 0.1f;
			leftforce.strength = 0.1f;


			//glm::vec3 cameraXYZ = glm::vec3( 0.0f, 0.0f, 5.0f );	// 5 units "down" z
			v = glm::lookAt( g_cameraXYZ ,						// "eye" or "camera" position
							 g_cameraTarget_XYZ,		// "At" or "target" 
							 glm::vec3( 0.0f, 1.0f, 0.0f ) );	// "up" vector

			mvp = p * v * m;			// This way (sort of backwards)

			GLint shaderID = ::g_pShaderManager->getIDFromFriendlyName("mySexyShader");
			GLint diffuseColour_loc = glGetUniformLocation( shaderID, "diffuseColour" );

			glUniform4f( diffuseColour_loc, 
						::g_vecGameObjects[index]->diffuseColour.r, 
						::g_vecGameObjects[index]->diffuseColour.g, 
						::g_vecGameObjects[index]->diffuseColour.b, 
						::g_vecGameObjects[index]->diffuseColour.a );


	//        glUseProgram(program);
			::g_pShaderManager->useShaderProgram( "mySexyShader" );

			
			glUniformMatrix4fv(mvp_location, 1, GL_FALSE, 
							   (const GLfloat*) glm::value_ptr(mvp) );

	//		glPolygonMode( GL_FRONT_AND_BACK, GL_POINT );
			glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
	

				glBindVertexArray( VAODrawInfo.VAO_ID );

				glDrawElements( GL_TRIANGLES, 
								VAODrawInfo.numberOfIndices,		// testMesh.numberOfTriangles * 3,	// How many vertex indices
								GL_UNSIGNED_INT,					// 32 bit int 
								0 );

				glBindVertexArray( 0 );

//			}// if ( ::g_pVAOManager->lookupVAOFromName(

		}//for ( int index = 0...


		std::stringstream ssTitle;
		ssTitle << "Camera (xyz): " 
			<< g_cameraXYZ.x << ", " 
			<< g_cameraXYZ.y << ", " 
			<< g_cameraXYZ.z;
		glfwSetWindowTitle( window, ssTitle.str().c_str() );

        glfwSwapBuffers(window);
        glfwPollEvents();


		// Essentially the "frame time"
		// Now many seconds that have elapsed since we last checked
		double curTime = glfwGetTime();
		double deltaTime =  curTime - lastTimeStep;
		float dr,dl,vr,vl;
		handlekeys((float)deltaTime);


		if (GetKeyState('C') < 0)
		{
			if (changePhysicsEngine == 1)
			{
				changePhysicsEngine = 0;
			}
				
			else
			{
				changePhysicsEngine = 1;
			}		

			Sleep(150);
		}

		if (!changePhysicsEngine)
		{
			io->PhysicsStep((float)deltaTime, wforce, sforce, leftforce, rightforce, upforce, disp, rot, my, collision);
		}
		
		else
		{
			io2->PhysicsStep((float)deltaTime, wforce, sforce, leftforce, rightforce, upforce, disp, rot, my, collision);
		}
	
		for (int i = 0; i < ::g_vecGameObjects.size(); i++)
		{
			if (debugrender)
			{
				if (collision[i] == 1)
				{
					::g_vecGameObjects[i]->diffuseColour = glm::vec4(1.0f, 0, 0, 1.0f);
				}
				else if (collision[i] == 2)
				{
					::g_vecGameObjects[i]->diffuseColour = glm::vec4(0, 1.0f, 0, 1.0f);
				}
				else if (collision[i] == 0)
				{
					::g_vecGameObjects[i]->diffuseColour = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
				}
				else
				{
					::g_vecGameObjects[i]->diffuseColour = glm::vec4(0, 0.0f, 0, 1.0f);
			}
			}
			::g_vecGameObjects[i]->position += disp[i];
			::g_vecGameObjects[i]->orientation2 += glm::radians(150.0f*rot[i]);
		}

		
		//PhysicsStep( deltaTime );

		lastTimeStep = curTime;

    }// while ( ! glfwWindowShouldClose(window) )


    glfwDestroyWindow(window);
    glfwTerminate();

	// 
	delete ::g_pShaderManager;
	delete ::g_pVAOManager;

//    exit(EXIT_SUCCESS);
	return 0;
}


