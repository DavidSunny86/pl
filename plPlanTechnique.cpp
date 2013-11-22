#include "plPlanTechnique.h"

plPlanTechnique::plPlanTechnique()
{
}      


void plPlanTechnique::render( const std::set< plRenderComponent >& componentSet ) const
{

    const std::shared_ptr< plFBO >&    fbo    = plRenderResources::fbos( PL_MAIN_FBO );
    const std::shared_ptr< plShader >& shader = plRenderResources::shaders( PL_PHONG_SHADER );

    // set initial rendering state
    _initState();
    
    // bind fbo
    fbo->bind(); 
    // bind shader
    shader->bind();

    // clear fbo
    glClearColor( 0, 0, 0, 0 );          
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
    glViewport( 0, 0, plWindow::viewportWidth(), plWindow::viewportHeight() );      
    
    // draw main render components
    for ( const plRenderComponent& component : componentSet )
    { 
        component.draw( *shader );   
    }

    // unbind shader
    shader->unbind();
    // unbind fbo
    fbo->unbind();      
        
    /*      
    // clear back buffer
    glClearColor( PL_CLEAR_COLOUR );          
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );         
    glViewport( plWindow::viewportX(), lWindow::viewportY(), plWindow::viewportWidth(), plWindow::viewportHeight() );  //////    

    // @#$%@#$ draw fbo to screen    
    plRenderComponent( _testTexture();
    for ( const plRenderComponent& component : componentSet )
    { 
        component.draw( *shader );   
    }
    */
}


void plPlanTechnique::_initState() const
{
    // enable back face culling
    glEnable( GL_CULL_FACE );
    glCullFace( GL_BACK );    
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    // set depth testing
    glEnable( GL_DEPTH_TEST ); 
    glDepthFunc( GL_LEQUAL );
    // enable blending
    glEnable( GL_BLEND ); 
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );      
}

/*
void plPlanTechnique::_drawFbo() const
{
    // VAO GENERATION
    std::vector<plVector3> vertices;
    std::vector<PLuint>    indices;
    std::vector<PLuint>    attributeTypes;
    
    attributeTypes.push_back( PL_POSITION_ATTRIBUTE );  
    attributeTypes.push_back( PL_TEXCOORD_ATTRIBUTE );
    
    // position                                     // texture coord       
    vertices.push_back( plVector3( -1, -1, 0 ) );   vertices.push_back( plVector3( 0,0,0) );   
    vertices.push_back( plVector3(  1, -1, 0 ) );   vertices.push_back( plVector3( 1,0,0) );   
    vertices.push_back( plVector3(  1,  1, 0 ) );   vertices.push_back( plVector3( 1,1,0) );    
    vertices.push_back( plVector3( -1,  1, 0 ) );   vertices.push_back( plVector3( 0,1,0) );
    
    indices.push_back( 0 );   indices.push_back( 1 );   indices.push_back( 2 );
    indices.push_back( 0 );   indices.push_back( 2 );   indices.push_back( 3 );
       
    static plVAO vao( vertices, attributeTypes, indices );
    ///
    
    // TEX
    std::vector<PLuint> texUnits;
    texUnits.push_back( 0 );
    texUnits.push_back( 1 );
    //
          
    plMatrix44 ortho( -1, 1, -1, 1, -1, 1 );

    plMatrix44 camera( 1, 0,  0, 0,
                       0, 1,  0, 0,
                       0, 0, -1, 0,
                       0, 0,  0, 1 ); 
        
    plTexture2DStack::push ( *_textures[ PL_DRAW_TEXTURE ],    0 );    
    plTexture2DStack::push ( *_textures[ PL_OUTLINE_TEXTURE ], 1 ); 
          
    plShaderStack::push    ( PL_FBO_SHADER );                   
    plProjectionStack::push( ortho );        // ortho projection
    plModelStack::push     ( plMatrix44() ); // identity model matrix
    plCameraStack::push    ( camera );       // default camera matrix

    plRenderComponent rc( &vao );
    // attach transformation uniforms
    rc.attach( plUniform( PL_MODEL_MATRIX_UNIFORM,      plMatrix44() );
    rc.attach( plUniform( PL_VIEW_MATRIX_UNIFORM,       camera       );
    rc.attach( plUniform( PL_PROJECTION_MATRIX_UNIFORM, ortho        );
    // attach texture uniforms
    rc.attach( plUniform( PL_TEXTURE_UNIT_0_UNIFORM, &_fbos[ PL_MAIN_FBO ]->texture2DAttachment( PL_MAIN_TEXTURE    ) );
    rc.attach( plUniform( PL_TEXTURE_UNIT_1_UNIFORM, &_fbos[ PL_MAIN_FBO ]->texture2DAttachment( PL_OUTLINE_TEXTURE ) );     
    
    plProjectionStack::pop(); 
    plModelStack::pop();       
    plCameraStack::pop();    
    plShaderStack::pop();   
}
*/
