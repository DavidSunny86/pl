#include "plPlanTechnique.h"

plPlanTechnique::plPlanTechnique()
{
}      


void plPlanTechnique::render( const std::set< plRenderComponent >& componentSet ) const
{
    const std::shared_ptr< plFBO >&    fbo    = plRenderResources::fbos( PL_MAIN_FBO );
    const std::shared_ptr< plShader >& shader = plRenderResources::shaders( PL_PHONG_SHADER );

    // bind fbo
    fbo->bind(); 

    // clear fbo before individual draw buffers are set
    glClearColor( 0, 0, 0, 0 );          
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
    glViewport( 0, 0, plWindow::viewportWidth(), plWindow::viewportHeight() ); 

    // set draw buffers
    std::vector<GLenum> drawBuffers;
    drawBuffers.push_back( GL_COLOR_ATTACHMENT0 );
    drawBuffers.push_back( GL_NONE );
    drawBuffers.push_back( GL_NONE );
    drawBuffers.push_back( GL_NONE );
    drawBuffers.push_back( GL_COLOR_ATTACHMENT4 );
    fbo->setDrawBuffers( drawBuffers );

    // bind shader
    shader->bind();

    glDisable( GL_CULL_FACE );

    // draw main render components
    for ( const plRenderComponent& component : componentSet )
    {     
        component.draw( *shader );   
    }

    glEnable( GL_CULL_FACE );

    // unbind shader
    shader->unbind();

    // unbind fbo
    fbo->unbind();  
}

