#include "PlannerWindow.h"

PlannerWindow::PlannerWindow( int x, int y, int width, int height, std::string title, int argc, char **argv )
    : _button( GLUT_NO_BUTTON ),
      _camera( ".view0" ), 
      _cameraMode( CAMERA_ROTATION_MODE ),
      _graftEditor(),
      _boundaryEditor(),
      _plan( argc, argv ), 
      Window( x, y, width, height, title )
{  
    plInit();
    _graftEditor.selectGraft( _plan, 0, PL_PICKING_INDEX_GRAFT_DONOR );
}


void PlannerWindow::idle()
{
    //glutPostRedisplay();
}


void PlannerWindow::display()
{
    plCameraStack::load( _camera );  
    plProjectionStack::load( plProjection( PL_FIELD_OF_VIEW , PL_ASPECT_RATIO, PL_NEAR_PLANE, PL_FAR_PLANE ) );

    plRenderer::queue( _plan );

    plRenderer::queue( _graftEditor );
    plRenderer::queue( _boundaryEditor );   

    plRenderer::draw();

    glutSwapBuffers();
}


void PlannerWindow::setCursor( int mx, int my )
{
    switch ( _button )
    {   
        case GLUT_MIDDLE_BUTTON:        glutSetCursor( GLUT_CURSOR_CROSSHAIR );     break;      // middle button:   zoom
        case GLUT_LEFT_BUTTON:          glutSetCursor( GLUT_CURSOR_INFO );          break;      // left button:     click / drag 
        case GLUT_RIGHT_BUTTON:  
           
            // right button:    translate / rotate 
            switch( _cameraMode )
            { 
                case CAMERA_TRANSLATION_MODE:   glutSetCursor( GLUT_CURSOR_CROSSHAIR );     break;    // translation cursor  
                case CAMERA_ROTATION_MODE:      glutSetCursor( GLUT_CURSOR_CYCLE );         break;    // rotation cursor
            }
            break;
            
        case GLUT_NO_BUTTON:            glutSetCursor( GLUT_CURSOR_RIGHT_ARROW );   break;      // no button:     idle cursor   
    }
}


void PlannerWindow::keyAction( unsigned char key, int mx, int my )
{
    static int currentView = 0;

    switch (key) 
    {
        case 27:    // esc  
             exit(0);   
                                        
        case ' ':  // spacebar 
            _cameraMode =  (_cameraMode == CAMERA_ROTATION_MODE) ? CAMERA_TRANSLATION_MODE : CAMERA_ROTATION_MODE;  
            break; 
            
        case 'W':   
            _camera.exportViewParams( ".view" + std::to_string( currentView ) );   
            break;
            
        case 'R':   
            _camera.importViewParams( ".view" + std::to_string( currentView ) );
            break;  
       
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case '0':   // 0-9            
            currentView = (PLint)(key - '0');
            break;                

        case 'b':   _plan.models(0).bone.toggleVisibility();                    break;            
        case 'c':   _plan.models(0).cartilage.toggleVisibility();               break;
        case 'i':   _plan.iGuides(0).toggleVisibility();                        break;
        case 'p':   _plan.toggleVisibility();                                   break;    
        case 'z':   _camera.reset( _plan.models(0).getCentroid() );             break;          
        case 't':   _graftEditor.setEditMode( PL_GRAFT_EDIT_MODE_TRANSLATE );   break; 
        case 'r':   _graftEditor.setEditMode( PL_GRAFT_EDIT_MODE_ROTATE );      break;     
        case 'l':   _graftEditor.setEditMode( PL_GRAFT_EDIT_MODE_LENGTH );      break; 
        case 'v':   _graftEditor.toggleSelectedVisibility();  
                    _boundaryEditor.toggleSelectedVisibility();                 break;
        case 'g':   _plan.iGuides(0).generateIGuideModels();                    break; // TODO: This is broken at the moment. Need to not have static 0 here
        case 'e':   _plan.iGuides(0).exportIGuideModels("output/");             break;
        case 'O':   _plan.exportFile("plan");                                   break;

        case 'D':   _plan.addDonorSite();                                       break;
        case 'S':   _plan.addDefectSite();                                      break;
        case 'G':   _plan.addIGuideSite();                                      break;

        case 'P':   plAutomaticPlanner::calculate( _plan );                     break;

        case 'N':   _plan.clear();  
                    _graftEditor.clearSelection   ( _plan );   
                    _boundaryEditor.clearSelection( _plan );                                          
                    break;     

        case 127:	 // delete 
        {   
            if (glutGetModifiers() == GLUT_ACTIVE_CTRL) 
	        {
                // delete boundary 
	            _boundaryEditor.clearSelectedBoundary( _plan );
            }
            else
            {
                // delete point
                _boundaryEditor.removeSelectedPoint();         	            
	        }
            break;
        } 
        
        case 'H':
            std::cout << plTimer::now() << "\n";
            break;
    }
    
    glutPostRedisplay();
}


void PlannerWindow::passiveMouseMotion( int mx, int my )
{   
    // do nothing   
}

void PlannerWindow::activeMouseMotion( int mx, int my )
{    
    // convert from GLUT window coords to OpenGL coords
    PLint x = mx;
    PLint y = glutGet(GLUT_WINDOW_HEIGHT)-my;

    switch ( _button )
    {
        case GLUT_LEFT_BUTTON: 

            // process drag movements 
            _graftEditor.processMouseDrag   ( _plan, x, y );   
            _boundaryEditor.processMouseDrag( _plan, x, y );  
            break;       

        case GLUT_MIDDLE_BUTTON:    
            
            // zoom camera
            _camera.zoom( y - _previousMouse.y );                             
            break;

        case GLUT_RIGHT_BUTTON:
        
            // previous and current mouse coords will always be very small, as 
            // _previousMouse has already been updated in mouseAction() by this point
            if (_cameraMode == CAMERA_ROTATION_MODE)
            {
                _camera.rotate( _previousMouse.x, _previousMouse.y, x, y );
            }
            else
		    {
			    _camera.translate( _previousMouse.x - x,
			                       _previousMouse.y - y );
            }
            break;  
    }

    // update mouse position on drag    
    _previousMouse.x = x;
    _previousMouse.y = y;

    glutPostRedisplay();
}



// Record button state when mouse button is pressed or released
void PlannerWindow::mouseAction( int button, int state, int mx, int my )
{
    // NOTE: this function ALWAYS executes BEFORE activeMouseMotion()
    
    // convert from GLUT window coords to OpenGL coords
    PLint x = mx;
    PLint y = glutGet(GLUT_WINDOW_HEIGHT)-my;
        
    switch (state)
    {
        case GLUT_UP:           
        {
            _graftEditor.processMouseRelease   ( _plan, x, y ); 
            _boundaryEditor.processMouseRelease( _plan, x, y ); 
            _button = GLUT_NO_BUTTON;  
            break; 
        }
        case GLUT_DOWN:  _button = button;  break;   // button press
    }
    
    switch (_button)
    {   
        case GLUT_LEFT_BUTTON:  

            if (glutGetModifiers() == GLUT_ACTIVE_CTRL) 
	        {
                // add new point
                _boundaryEditor.addPoint( _plan, x, y ); 
            }
            else
            {
                // process mouse clicks 
                _graftEditor.processMouseClick   ( _plan, x, y );    
                _boundaryEditor.processMouseClick( _plan, x, y ); 
            }
            break;
    }    
     
    // update mouse position on key press
    _previousMouse.x = x;
    _previousMouse.y = y;   
    
    glutPostRedisplay();
}
