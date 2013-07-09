#include "Window.h"

static Window *windows[ MAX_NUM_WINDOWS ];

Window::Window( int x, int y, int width, int height, std::string title ) 
{
    glutInitWindowPosition( x, y );
    glutInitWindowSize( width, height );
    m_windowId = glutCreateWindow( title.c_str() );
    
    // init glew AFTER glut (needs rendering context)
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
      fprintf(stderr, "GLEW Error: %s\n", glewGetErrorString(err));
    }
             
    registerWindow( m_windowId );
    glutSetWindow( m_windowId );
    glutDisplayFunc( GLUTdisplay );
    glutReshapeFunc( GLUTreshape );
    glutMouseFunc( GLUTmouseAction );
    glutMotionFunc( GLUTactiveMouseMotion );
    glutPassiveMotionFunc( GLUTpassiveMouseMotion );
    glutKeyboardFunc( GLUTkeyAction );

}

void Window::registerWindow( int windowId )
{
    if (m_windowId < 0 || m_windowId >= MAX_NUM_WINDOWS) 
    {
        std::cerr << "New window has windowId " << m_windowId
                  << ", which is outside the allowed range 0.." << MAX_NUM_WINDOWS-1
                  << std::endl;
        abort();
    }
    windows[ m_windowId ] = this;
}

void Window::reshape( int width, int height ) 
{
    glViewport( 0, 0, width, height );
    plWindowReshape( width, height );
    glutPostRedisplay();
}
        
void GLUTdisplay() 
{
    windows[ glutGetWindow() ]->display();
}

void GLUTreshape( int x, int y ) 
{
    windows[ glutGetWindow() ]->reshape( x, y );
}


void GLUTmouseAction( int button, int state, int x, int y ) 
{
    windows[ glutGetWindow() ]->mouseAction( button, state, x, y );
    windows[ glutGetWindow() ]->setCursor(x, y);
}


void GLUTactiveMouseMotion( int x, int y ) 
{
    windows[ glutGetWindow() ]->activeMouseMotion( x, y );
    windows[ glutGetWindow() ]->setCursor(x, y);
}


void GLUTpassiveMouseMotion( int x, int y ) 
{
    windows[ glutGetWindow() ]->passiveMouseMotion( x, y );
    windows[ glutGetWindow() ]->setCursor(x, y);
}


void GLUTkeyAction( unsigned char c, int x, int y ) 
{
    windows[ glutGetWindow() ]->keyAction( c, x, y );
}
