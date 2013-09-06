#include "plRenderer.h"

namespace plRenderer
{
    // private function prototypes
    void _setOpenGLState();
    
    void _beginPicking();
    void _endPicking();
    
    void _beginDrawing();
    void _endDrawing();
    
    void _drawScene();
    void _drawScenePicking();

    void _clearRenderQueue();

    void _drawArthroTexture();

    // private variable declarations
    const plPlan*            _planToDraw           = NULL;
    const plGraftEditor*     _graftEditorToDraw    = NULL;
    const plBoundaryEditor*  _boundaryEditorToDraw = NULL;
    const plTextureMesh*     _arthroTextureToDraw  = NULL;
    const plTrackedObject*   _probeToDraw          = NULL;
    const plTrackedObject*   _scopeToDraw          = NULL;

    plMinimalShader*         _minimalShader        = NULL;
    plPhongShader*           _phongShader          = NULL; 
    plPickingShader*         _pickingShader        = NULL;
    plTextureShader*         _textureShader        = NULL;

    void init()
    {   
        delete _minimalShader;
        delete _phongShader;
        delete _pickingShader;
        delete _textureShader;
        
        _minimalShader = new plMinimalShader("./shaders/minimal.vert", "./shaders/minimal.frag");       
        _phongShader   = new plPhongShader  ("./shaders/phong.vert", "./shaders/phong.frag");        
        _pickingShader = new plPickingShader("./shaders/picking.vert", "./shaders/picking.frag");         
        _textureShader = new plTextureShader("./shaders/texture.vert", "./shaders/texture.frag");  
    } 


    void _clearRenderQueue()
    {
        _planToDraw           = NULL;
        _graftEditorToDraw    = NULL;
        _boundaryEditorToDraw = NULL;
        _arthroTextureToDraw  = NULL;
        _probeToDraw          = NULL;
        _scopeToDraw          = NULL;
    }


    void queue( const plPlan &plan )
    {
        if (_planToDraw != NULL)
            std::cerr << "plRenderer queue() error: plPlan already queued to draw, overridding previous \n";
            
        _planToDraw = &plan;
    }


    void queue( const plGraftEditor &editor )
    {
        if (_boundaryEditorToDraw != NULL)
            std::cerr << "plRenderer queue() error: plGraftEditor already queued to draw, overridding previous \n";
            
        _graftEditorToDraw = &editor;
    }


    void queue( const plBoundaryEditor &editor )
    {
        if (_boundaryEditorToDraw != NULL)
            std::cerr << "plRenderer queue() error: plBoundaryEditor already queued to draw, overridding previous \n";
            
        _boundaryEditorToDraw = &editor;
    }


    void queue( const plTextureMesh &arthroTexture )
    {
        if (_arthroTextureToDraw != NULL)
            std::cerr << "plRenderer queue() error: plTextureMesh already queued to draw, overridding previous \n";
            
        _arthroTextureToDraw = &arthroTexture;
    }


    void queue( const plTrackedObject &object )
    {
        if ( object.isArthroscope() )
        {
            if (_scopeToDraw != NULL)
                std::cerr << "plRenderer queue() error: Arthroscope already queued to draw, overridding previous \n";
            
            _scopeToDraw = &object;
        }
        else
        {
            if (_probeToDraw != NULL)
                std::cerr << "plRenderer queue() error: Probe already queued to draw, overridding previous \n";
            
            _probeToDraw = &object;
        }
    }


    void reportError( const plString &str  ) 
    {
        GLuint errnum;
        const char *errstr;
        while (errnum = glGetError()) 
        {
            errstr = reinterpret_cast<const char*> (gluErrorString(errnum));
            std::cout << str << " " << errstr << "\n";
        }
    }


    void _setOpenGLState()
    {
        glEnable( GL_CULL_FACE );
        glCullFace( GL_BACK );    
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
        glEnable( GL_DEPTH_TEST );   
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );  
    }


    void draw()
    {
        _setOpenGLState();

        // picking draw    
        _beginPicking();
        {
            _drawScenePicking();
        }
        _endPicking();
            
        // normal draw
        _beginDrawing();
        {
            _drawScene();
        }

        _endDrawing();

        // clear queues from this frame
        _clearRenderQueue();
    }


    void _beginPicking()
    {
        glDisable( GL_BLEND );

        // bind picking shader
        plShaderStack::push( _pickingShader );
        
        // bind picking texture
        plPicking::texture->bind();

        // clear picking texture
        glClearColor(0,0,0,0);
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);  

    }


    void _endPicking()
    {
        plShaderStack::pop();
        plPicking::texture->unbind();  
    }


    void _beginDrawing()
    {
        glEnable( GL_BLEND );
        
        glClearColor( 1,1,1,0 );
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        

        glStencilFunc( GL_ALWAYS, 0x00, 0x00);               // only render to bits = 0 (have not been written)
	    plShaderStack::push( _phongShader );
    }


    void _endDrawing()
    {
        plShaderStack::pop(); 
    }


    void _drawScene()
    {
        // arthro texture
        if (_arthroTextureToDraw != NULL)
        {
            _drawArthroTexture();
        }
        
        // plan
        if (_planToDraw != NULL)
        {
            _planToDraw->drawElements();
            _planToDraw->drawModels();
        }

        // graft editor handles
        if (_graftEditorToDraw != NULL)
        {
            _graftEditorToDraw->drawHandles();
        }
        
        // draw editor menus
        if ( (_boundaryEditorToDraw != NULL || _graftEditorToDraw != NULL) && _planToDraw != NULL)
        {
            // set flat shader
            plShaderStack::push( _minimalShader );  
            plCameraStack::push( plMatrix44() );                                                        // identity viewing matrix
            plProjectionStack::push( plMatrix44( 0, plWindow::width(), 0, plWindow::height(), -1, 1) ); // ortho, viewport dimensions
            {
                _boundaryEditorToDraw->drawMenu( *_planToDraw );
                _graftEditorToDraw->drawMenu( *_planToDraw );
            }
            plProjectionStack::pop(); 
            plCameraStack::pop();                
            plShaderStack::pop();   
        }   
        
        // draw probe
        if (_probeToDraw != NULL)
        {
            _probeToDraw->draw();
        }
        
        // draw arthroscope
        if (_scopeToDraw != NULL)
        {
            _scopeToDraw->draw();
        }
        
        // DEBUG
        // GRID POINTS
        for (PLuint i=0; i<plAutomaticPlanner::_donorSiteGrids.size(); i++)
        {
            plColourStack::load( 0.9, 0.6, 0.2 );
            for (PLuint j=0; j<plAutomaticPlanner::_donorSiteGrids[i].gridSize(); j++)
            {
                PLfloat x = plAutomaticPlanner::_donorSiteGrids[i].points(j).x;
                PLfloat y = plAutomaticPlanner::_donorSiteGrids[i].points(j).y;
                PLfloat z = plAutomaticPlanner::_donorSiteGrids[i].points(j).z;
                
                plDraw::sphere( plVector3(x,y,z), 0.09f );
            }
        }
        for (PLuint i=0; i<plAutomaticPlanner::_defectSiteGrids.size(); i++)
        {    
            plColourStack::load( 0.2, 0.6, 0.9 );
            
            for (PLuint j=0; j<plAutomaticPlanner::_defectSiteGrids[i].gridSize(); j++)
            {
                PLfloat x = plAutomaticPlanner::_defectSiteGrids[i].points(j).x;
                PLfloat y = plAutomaticPlanner::_defectSiteGrids[i].points(j).y;
                PLfloat z = plAutomaticPlanner::_defectSiteGrids[i].points(j).z;
                plDraw::sphere( plVector3(x,y,z), 0.09f );
                
            }
        }
               
        /* DEBUG FOR OCTREES
        // set flat shader
        plShaderStack::push( _minimalShader );    
        
        // octrees objects
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);    
        for (PLuint i =0; i < _planToDraw->models().size(); i++)
        {      
            plColourStack::load( 0.3, 0.1, 0.4 );
            //_planToDraw->models(i).bone.octree().draw();
            //plColourStack::load( 0.1, 0.4, 0.3 );
            _planToDraw->models(i).cartilage.octree().draw();
        }
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        */

    }


    void _drawArthroTexture()
    {
        PLfloat xmargin = (plWindow::width() / plWindow::height()- 1) / 2.0;
        
        plMatrix44 ortho( -1, 1, -1, 1, -1, 1);

        plMatrix44 camera( 1, 0,  0, 0,
                           0, 1,  0, 0,
                           0, 0, -1, 0,
                           0, 0,  0, 1 ); 
                           
        plShaderStack::push( _textureShader );                   
        plProjectionStack::push( ortho );   // ortho projection
        plModelStack::push( plMatrix44() ); // identity model matrix
        plCameraStack::push( camera );      // default camera matrix
        {
            glDisable( GL_DEPTH_TEST );

            _arthroTextureToDraw->draw();
             
            glEnable( GL_DEPTH_TEST ); 
        }       
        plProjectionStack::pop(); 
        plModelStack::pop();       
        plCameraStack::pop();    
        plShaderStack::pop();
    }


    void _drawScenePicking()
    {
        // plan
        if (_planToDraw != NULL)
        {       
            glEnable     ( GL_STENCIL_TEST );				      // need stencil testing enabled  					
	        glStencilFunc( GL_ALWAYS, 0xFF, 0xFF );               // replace where rendered 		
	        glStencilOp  ( GL_REPLACE, GL_REPLACE, GL_REPLACE );  // always replace previous bit	
	        glStencilMask( 0x01 );                                // only write to first bit
           
            // elements
            _planToDraw->drawElements();

            glStencilOp  ( GL_KEEP, GL_KEEP, GL_KEEP );	        // no longer modify the stencil bits  
            glStencilFunc( GL_EQUAL, 0x00, 0xFF);               // only render to bits = 0 (have not been written)

            // models
            _planToDraw->drawModels();

            glDisable( GL_STENCIL_TEST );                       // no more stencil testing
        }

        // graft editor
        if (_graftEditorToDraw != NULL)
        {
            _graftEditorToDraw->drawHandles();
        }
        
        // draw editor menus
        if ( (_boundaryEditorToDraw != NULL || _graftEditorToDraw != NULL) && _planToDraw != NULL)
        {        
            plCameraStack::push( plMatrix44() );                                                        // identity viewing matrix   
            plProjectionStack::push( plMatrix44( 0, plWindow::width(), 0, plWindow::height(), -1, 1) ); // ortho, viewport dimensions
            {
                _boundaryEditorToDraw->drawMenu( *_planToDraw );
                _graftEditorToDraw->drawMenu( *_planToDraw );
            }
            plCameraStack::pop(); 
            plProjectionStack::pop();
        }   

    }

}
