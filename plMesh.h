#ifndef _PL_MESH_H_
#define _PL_MESH_H_ 

#include "plCommon.h"
#include "plVector3.h"
#include "plTriangle.h"
#include "plShader.h" // attribute enums
#include "plRenderingPipeline.h"

class plMesh
{
    public:
		
		plMesh();		
		plMesh( const plSeq<plTriangle> &triangles );        
        plMesh( const plSeq<plVector3> &vertices, const plSeq<PLuint> &indices );       
        plMesh( const plMesh &mesh );
        
        virtual ~plMesh();
        
        plMesh& operator = ( const plMesh &mesh ); 
              
        virtual void setBuffers( const plSeq<plVector3> &vertices, const plSeq<PLuint> &indices);
        virtual void draw() const;	        	
		virtual void draw( const plSeq<PLuint> &indices ) const;    // draw with newly defined index order ( for transparency sorting )
        
    protected:

        GLuint _numIndices;             // num indices needed by glDrawElements
        GLuint _numBytes;               // size of vertex buffer

        GLuint _vertexBufferIndices;
        GLuint _vertexBufferObject;	    // vertex buffer objects
		GLuint _vertexArrayObject;	    // vertex array object
		
		virtual void _destroy();
		virtual void _copyMesh( const plMesh &mesh );
		virtual void _triangleToInterleaved( const plSeq<plTriangle> &triangles );
        
		

};

#endif
