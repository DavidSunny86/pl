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
		plMesh(const plSeq<plTriangle> &triangles);        
        plMesh(const plSeq<plVector3> &interleaved_vertices, const plSeq<PLuint> &indices);

        void triangleToInterleaved(const plSeq<plTriangle> &triangles);
        void destroy();       
        
        virtual void setBuffers( const plSeq<plVector3> &interleaved_vertices, const plSeq<PLuint> &indices);
        virtual void draw() const;	
        	
		void draw(const plSeq<PLuint> &indices) const;
        
    protected:

        GLuint _numIndices;

        GLuint _vertexBufferIndices;
        GLuint _vertexBufferObject;	    // vertex buffer objects
		GLuint _vertexArrayObject;	    // vertex array object

};

#endif
