#include "RenderManager.h"

#include "IcosphereGenerator.h"
#include "OrthographicCamera.h"
#include "PerspectiveCamera.h"
#include "ProjectionState.h"
#include <iostream>

using std::cout;
using std::endl;

GLuint RenderManager::CreateFullscreenQuad( const EngineConfig &engineCfg ) const
{
    GLuint quadDisplayList = glGenLists( 1 );

    glNewList( quadDisplayList, GL_COMPILE );
        glBegin(GL_QUADS);
            glTexCoord2f(0.0f, 1.0f);
            glVertex2f(0.0f, 0.0f);
            glTexCoord2f(0.0f, 0.0f);
            glVertex2f(0.0f, (float) engineCfg.GetActiveHeight());
            glTexCoord2f(1.0f, 0.0f);
            glVertex2f( (float) engineCfg.GetActiveWidth(), (float) engineCfg.GetActiveHeight() );
            glTexCoord2f(1.0f, 1.0f);
            glVertex2f( (float) engineCfg.GetActiveWidth(), 0.0f );
        glEnd();
    glEndList();

    return quadDisplayList;
}

void RenderManager::ResetViewport( const EngineConfig &engineCfg ) const
{
    glViewport( 0, 0, engineCfg.GetActiveWidth(), engineCfg.GetActiveHeight() );
}

void RenderManager::BindDeferredFbo() const
{
    glBindFramebuffer( GL_FRAMEBUFFER, deferredFbo );
	glPushAttrib( GL_VIEWPORT_BIT );

    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ); //clear the FBO

	glActiveTexture( GL_TEXTURE0 );
	glEnable( GL_TEXTURE_2D );

	// Specify what to render and start acquiring
	GLenum buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers( 2, buffers );
}

void RenderManager::UnbindDeferredFbo() const
{
	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
	glPopAttrib();
}

void RenderManager::BindShadowFbo( const GLenum cubeFace ) const
{
	glBindFramebuffer( GL_DRAW_FRAMEBUFFER, shadowFbo );

	//glViewport(0, 0, SHADOW_SIZE, SHADOW_SIZE);

	glFramebufferTexture2D( GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, cubeFace, shadowMap, 0);
	glDrawBuffer( GL_COLOR_ATTACHMENT0 );
}

void RenderManager::UnbindShadowFbo() const
{
	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
}

void RenderManager::Render( const Simulation &gameSim, const EngineConfig &engineCfg ) const
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ); //clear the screen buffer

	//render scene to fbo with active camera
    glUseProgram( geometryPassShader.GetProgramHandler() );

		glDepthMask( GL_TRUE ); //enable writing to the depth buffer
		glEnable( GL_DEPTH_TEST );

		BindDeferredFbo();

			glEnable( GL_CULL_FACE );
			glCullFace( GL_BACK );

			//bind the surface texture and pass it to the shader
			glActiveTexture( GL_TEXTURE0 );
			glUniform1i( surfaceTextureId, 0 );

			//render the simulation
			ProjectionState cameraProjection = gameSim.RenderLit();
			//ProjectionState cameraProjection = gameSim.RenderShadowCasters( gameSim.GetStaticLights()[2].GetCameras()[5] ); //for testing light cameras

			int viewportMatrix[4];
			float perspectiveMatrix[16];
			cameraProjection.CopyViewportMatrix( viewportMatrix );
			cameraProjection.CopyPerspectiveMatrix( perspectiveMatrix );

		UnbindDeferredFbo();

	glUseProgram( 0 );

	//render depth buffer to fullscreen quad
	glClear( GL_DEPTH_BUFFER_BIT );
	OrthographicCamera orthoCamera( frmr::Vec3f(), frmr::Vec2f(), engineCfg.GetActiveWidth(), engineCfg.GetActiveHeight() );
	orthoCamera.ApplyTransformation();

    glUseProgram( depthTransferShader.GetProgramHandler() );
		glActiveTexture( GL_TEXTURE0 );
		glBindTexture( GL_TEXTURE_2D, depthTexture );
		glUniform1i( depthId, 0 );
		glCallList( fullscreenQuad );
	glUseProgram( 0 );

    glDepthMask( GL_FALSE ); //disable writing to the depth buffer
    glDisable( GL_DEPTH_TEST );

    //send all the textures, the viewport parameters and the perspective matrix to the deferred rendering shader
    glUseProgram( lightPassShader.GetProgramHandler() );
		glUniform1i( normalsId, 0 );
		glUniform1i( diffuseId, 1 );
		glUniform1i( depthId, 2 );
		glUniform4iv( viewportParamsId, 4, viewportMatrix );
		glUniformMatrix4fv( perspectiveMatrixId, 16, false, perspectiveMatrix );
    glUseProgram( 0 );

    glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );

    glEnable( GL_STENCIL_TEST );
    glClearStencil( 0 );

    //const vector<Light> staticLights;// = gameSim.GetStaticLights();
    vector<Light> staticLights = gameSim.GetStaticLights();

	PerspectiveCamera activeCamera = gameSim.GetActiveCamera();

    glDisable( GL_BLEND );

    for ( auto lightIt : staticLights )
    {
    	//if light casts shadows
    	//for each face
    	//render face to fbo

    	if ( lightIt.CastsShadows() )
		{
			glBindFramebuffer( GL_FRAMEBUFFER, shadowFbo );
			glViewport( 0, 0, 1024, 1024 );
			glCullFace( GL_FRONT );

			glUseProgram( shadowPassShader.GetProgramHandler() );

				int cameraIndex = 0;
				for ( auto camIt : lightIt.GetCameras() )
				{
					//if camera frustum intersects activeCamera frustum
					glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X + cameraIndex++, shadowMap, 0 );
					glClear( GL_DEPTH_BUFFER_BIT );

					gameSim.RenderShadowCasters( camIt );
				}

			glUseProgram( 0 );
			glBindFramebuffer( GL_FRAMEBUFFER, 0 );
		}

        glColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE ); //disable writing to the color buffer
        glStencilMask( 0xFF ); //enable writing to the stencil buffer

        glClear( GL_STENCIL_BUFFER_BIT );

        glStencilFunc( GL_NEVER, 1, 0xFF ); // never pass stencil test
        glStencilOp( GL_REPLACE, GL_KEEP, GL_KEEP );  // replace stencil buffer values to ref=1

        activeCamera.ApplyTransformation();
		ResetViewport( engineCfg );

		//render icosphere
        glPushMatrix();
            glTranslatef( lightIt.GetPosition().GetX(), lightIt.GetPosition().GetY(), lightIt.GetPosition().GetZ() );
            glScalef( lightIt.GetRadius(), lightIt.GetRadius(), lightIt.GetRadius() );
            glDisable( GL_CULL_FACE );
            glDisable( GL_DEPTH_TEST ); //TODO: use the stencil buffer with depth fail and cull front face
            glBindTexture( GL_TEXTURE_2D, 0 );
            glCallList( icosphere );
        glPopMatrix();

        glDisable( GL_DEPTH_TEST );

        //light only where the stencil buffer is equal to 1
        glStencilFunc( GL_EQUAL, 1, 0xFF );

        glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE ); //enable writing to the color buffer
        glStencilMask( 0x00 ); //disable writing to the stencil buffer

		orthoCamera.ApplyTransformation();


		glDisable( GL_STENCIL_TEST );

        glUseProgram( lightPassShader.GetProgramHandler() );

			//pass the light's attributes to the shader
			glUniform3f( lightPositionId, lightIt.GetPosition().GetX(), lightIt.GetPosition().GetY(), lightIt.GetPosition().GetZ() );
			glUniform3f( lightColorId, lightIt.GetColor().GetX(), lightIt.GetColor().GetY(), lightIt.GetColor().GetZ() );
			glUniform1f( lightLinearAttenuationId, lightIt.GetLinearAttenuation() );
			glUniform1f( lightQuadraticAttenuationId, lightIt.GetQuadraticAttenuation() );

			glUniform1i( shadowId, shadowMap );
			glEnable( GL_TEXTURE_CUBE_MAP );
			glBindTexture( GL_TEXTURE_CUBE_MAP, shadowMap );

			glDisable( GL_CULL_FACE );

			//enable blending so that each new quad adds to whatever's in the render buffer
			glEnable( GL_BLEND );
			glBlendFunc( GL_ONE, GL_ONE );

			//bind the diffuse, normal and depth maps before rendering fullscreen quad
			glActiveTexture( GL_TEXTURE0 );
			glBindTexture( GL_TEXTURE_2D, normalsTexture );
			glActiveTexture( GL_TEXTURE1 );
			glBindTexture( GL_TEXTURE_2D, diffuseTexture );
			glActiveTexture( GL_TEXTURE2 );
			glBindTexture( GL_TEXTURE_2D, depthTexture );

			glCallList( fullscreenQuad );

			glDisable( GL_TEXTURE_CUBE_MAP );
			glDisable( GL_BLEND );

		glUseProgram( 0 );
    }

    glDisable(GL_STENCIL_TEST );

    // Reset OpenGL state
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, 0);

    //change to perspective projection
	//enable depth test
	//enable depth mask
	//draw projectiles

    //change to orthogonal projection
	//disable depth test
	//disable depth mask
	//draw HUD
	//draw UI
}

void RenderManager::SetupOpenGL( const EngineConfig &engineCfg ) const
{
    glewInit();
    ResetViewport( engineCfg );
    glClearColor( 0.2f, 0.0f, 0.2f, 0.0f );
    glDepthFunc( GL_LEQUAL );
}

void RenderManager::SimpleRender( const Simulation &gameSim, const EngineConfig &engineCfg ) const
{
    glEnable( GL_TEXTURE_2D );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glEnable( GL_DEPTH_TEST );
    glDisable( GL_CULL_FACE );
    glUseProgram( 0 );
    //gameSim.RenderLit( engineCfg.GetFOV(), engineCfg.GetActiveVerticalFOV() );
    gameSim.RenderLit();
}

void RenderManager::CreateDeferredFbo( const EngineConfig &engineCfg )
{
	// Generate the OGL resources for what we need
	glGenFramebuffers(1, &deferredFbo);
	glGenRenderbuffers(1, &normalsRT);
	glGenRenderbuffers(1, &diffuseRT);
	glGenRenderbuffers(1, &depthRT );

    // Bind the FBO so that the next operations will be bound to it
	glBindFramebuffer(GL_FRAMEBUFFER, deferredFbo);

	// Bind the normal render target
	glBindRenderbuffer(GL_RENDERBUFFER, normalsRT);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGB8, engineCfg.GetActiveWidth(), engineCfg.GetActiveHeight());
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, normalsRT);

    glBindRenderbuffer(GL_RENDERBUFFER, diffuseRT);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGB8, engineCfg.GetActiveWidth(), engineCfg.GetActiveHeight());
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_RENDERBUFFER, diffuseRT);

	glBindRenderbuffer(GL_RENDERBUFFER, depthRT);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, engineCfg.GetActiveWidth(), engineCfg.GetActiveHeight());
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRT);

	// Generate and bind the OGL texture for normals
	glGenTextures(1, &normalsTexture);
	glBindTexture(GL_TEXTURE_2D, normalsTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, engineCfg.GetActiveWidth(), engineCfg.GetActiveHeight(), 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// Attach the texture to the FBO
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, normalsTexture, 0);

    // Generate and bind the OGL texture for diffuse
	glGenTextures(1, &diffuseTexture);
	glBindTexture(GL_TEXTURE_2D, diffuseTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, engineCfg.GetActiveWidth(), engineCfg.GetActiveHeight(), 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// Attach the texture to the FBO
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, diffuseTexture, 0);

    // Generate and bind the OGL texture for depth
	glGenTextures(1, &depthTexture);
	glBindTexture(GL_TEXTURE_2D, depthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, engineCfg.GetActiveWidth(), engineCfg.GetActiveHeight(), 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// Attach the texture to the FBO
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);
    //check that FBO is complete

    if ( glCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
    {
        cout << "RenderManager::CreateDeferredFbo() - FBO is not complete." << endl;
    }

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Get the handles from the shader
	normalsId = glGetUniformLocation( lightPassShader.GetProgramHandler(), "normalsTexture" );
	diffuseId = glGetUniformLocation( lightPassShader.GetProgramHandler(), "diffuseTexture" );
	depthId = glGetUniformLocation( lightPassShader.GetProgramHandler(), "depthTexture" );

	shadowId = glGetUniformLocation( lightPassShader.GetProgramHandler(), "shadow" );
}

void RenderManager::CreateShadowCubemap()
{
	glGenFramebuffers( 1, &shadowFbo );

//	// Create the depth buffer
//	glGenTextures( 1, &shadowDepth );
//    glBindTexture( GL_TEXTURE_2D, shadowDepth );
//    glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL );
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//    glBindTexture(GL_TEXTURE_2D, 0);

    // Create the cube map
   	glGenTextures( 1, &shadowMap );
    glBindTexture( GL_TEXTURE_CUBE_MAP, shadowMap );
    glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE );

	for ( unsigned int face = 0 ; face < 6 ; face++ )
	{
        glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, GL_DEPTH_COMPONENT, 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL );
    }

	glBindTexture( GL_TEXTURE_CUBE_MAP, 0 );

	glBindFramebuffer( GL_FRAMEBUFFER, shadowFbo );

     // Disable reads and writes to the color buffer
    //glDrawBuffer( GL_NONE );
    //glReadBuffer( GL_NONE );

    GLenum status = glCheckFramebufferStatus( GL_FRAMEBUFFER );

    if ( status != GL_FRAMEBUFFER_COMPLETE)
	{
        cout << "RenderManager::CreateShadowCubemap - FBO is not complete." << endl;
    }

    glBindFramebuffer( GL_FRAMEBUFFER, 0 );
}

RenderManager::RenderManager( const EngineConfig &engineCfg )
{
    SetupOpenGL( engineCfg );
    geometryPassShader.Load( "../data/shaders/geometryPass.vert", "../data/shaders/geometryPass.frag" );
    lightPassShader.Load( "../data/shaders/lightPass.vert", "../data/shaders/lightPass.frag" );
    depthTransferShader.Load( "../data/shaders/depthTransfer.vert", "../data/shaders/depthTransfer.frag" );
    shadowPassShader.Load( "../data/shaders/shadowPass.vert", "../data/shaders/shadowPass.frag" );

    CreateDeferredFbo( engineCfg );
	CreateShadowCubemap();

    //get the memory location of the surface texture in the shader
	surfaceTextureId = glGetUniformLocation( geometryPassShader.GetProgramHandler(), "surfaceTexture" );

	viewportParamsId = glGetUniformLocation( lightPassShader.GetProgramHandler(), "viewportParams" );
	perspectiveMatrixId = glGetUniformLocation( lightPassShader.GetProgramHandler(), "perspectiveMatrix" );

	lightPositionId = glGetUniformLocation( lightPassShader.GetProgramHandler(), "lightPosition" );
	lightColorId = glGetUniformLocation( lightPassShader.GetProgramHandler(), "lightColor" );
	lightLinearAttenuationId = glGetUniformLocation( lightPassShader.GetProgramHandler(), "lightLinearAttenuation" );
    lightQuadraticAttenuationId = glGetUniformLocation( lightPassShader.GetProgramHandler(), "lightQuadraticAttenuation" );


    fullscreenQuad = CreateFullscreenQuad( engineCfg );
	icosphere = IcosphereGenerator::GenerateIcosphereDisplayList( 1 );

	glDisable(GL_LIGHTING);
}

RenderManager::~RenderManager()
{
    glDeleteTextures(1, &normalsTexture);
    glDeleteTextures(1, &diffuseTexture);
    glDeleteTextures(1, &depthTexture );
	glDeleteFramebuffers(1, &deferredFbo);
	glDeleteRenderbuffers(1, &normalsRT);
	glDeleteRenderbuffers(1, &diffuseRT);
	glDeleteRenderbuffers(1, &depthRT);
	glDeleteLists( icosphere, 1 );
	glDeleteLists( fullscreenQuad, 1 );
}
