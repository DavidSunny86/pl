#include "plMesh.h"

plMesh::plMesh() 
    : _vertexBufferObject(0), _vertexBufferIndices(0), _vertexArrayObject(0)
{
}

plMesh::plMesh(const plSeq<plTriangle> &triangles)
    : _vertexBufferObject(0), _vertexBufferIndices(0), _vertexArrayObject(0)
{
    // set vertex count
	_numIndices = triangles.size() * 3;
	// convert triangles to interleaved
	triangleToInterleaved(triangles);
}

plMesh::plMesh(const plSeq<plVector3> &interleaved_vertices, const plSeq<unsigned int> &indices)
    : _vertexBufferObject(0), _vertexBufferIndices(0), _vertexArrayObject(0)
{            
	// set index count
	_numIndices = indices.size();
	// set VBO and VAO
    setBuffers(interleaved_vertices, indices);
}

void plMesh::destroy()
{            
    glDeleteBuffers(1, &_vertexBufferObject);	    // delete buffer objects
    glDeleteBuffers(1, &_vertexBufferIndices);      // delete indices
	glDeleteVertexArrays(1, &_vertexArrayObject);	// delete VAO		    
}

void plMesh::triangleToInterleaved(const plSeq<plTriangle> &triangles)
{			
	// convert to interleaved format
	plSeq<plVector3> interleaved_vertices;
	plSeq<unsigned int> indices;
	
	interleaved_vertices.reserve(_numIndices*2);
	indices.reserve(_numIndices);
	
    int indexCount = 0;
    for (PLint i = 0; i < triangles.size(); i++) 
    {  
        // p1
	    interleaved_vertices.add(triangles[i].point1);
	    interleaved_vertices.add(triangles[i].normal);
	    indices.add(indexCount++);
	    // p2
	    interleaved_vertices.add(triangles[i].point2);
	    interleaved_vertices.add(triangles[i].normal);
	    indices.add(indexCount++);
	    // p3
	    interleaved_vertices.add(triangles[i].point3);
	    interleaved_vertices.add(triangles[i].normal);
	    indices.add(indexCount++);	    
	}

    setBuffers(interleaved_vertices, indices);
}

void plMesh::setBuffers( const plSeq<plVector3> &interleaved_vertices, const plSeq<unsigned int> &indices)
{
    // destroy previous buffers incase
    //destroy();

    // size of each vertex 
	const GLuint POS_SIZE = sizeof(GLfloat)*3;
	const GLuint NOR_SIZE = sizeof(GLfloat)*3;
    const GLuint TOTAL_SIZE = POS_SIZE + NOR_SIZE ;  
    const GLuint ARRAY_SIZE = TOTAL_SIZE * interleaved_vertices.size()/2;
    
    // create and bind VAO
	glGenVertexArrays(1, &_vertexArrayObject);     
	glBindVertexArray(_vertexArrayObject);
     
	// create and bind vertex VBO
	glGenBuffers(1, &_vertexBufferObject);
    glBindBuffer(GL_ARRAY_BUFFER, _vertexBufferObject); 
    glBufferData(GL_ARRAY_BUFFER, ARRAY_SIZE, &interleaved_vertices[0], GL_STATIC_DRAW);    
       
    // set position pointer, offset and stride
	glEnableVertexAttribArray(PL_POSITION_ATTRIBUTE);
	glVertexAttribPointer(PL_POSITION_ATTRIBUTE, 3, GL_FLOAT, GL_FALSE, TOTAL_SIZE, 0); 

    // set normal pointer, offset and stride
	glEnableVertexAttribArray(PL_NORMAL_ATTRIBUTE);
	glVertexAttribPointer(PL_NORMAL_ATTRIBUTE, 3, GL_FLOAT, GL_FALSE, TOTAL_SIZE, (GLvoid*)(POS_SIZE));                                
     
    // bind vertex array object
    glGenBuffers(1, &_vertexBufferIndices);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _vertexBufferIndices);   
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*_numIndices, &indices[0], GL_STREAM_DRAW);
     
	// unbind the vertex array object
	glBindVertexArray(0); 			
}

void plMesh::draw() const
{		    			
	// bind vertex array object
	glBindVertexArray(_vertexArrayObject);
	// draw batch
	glDrawElements( GL_TRIANGLES, _numIndices, GL_UNSIGNED_INT, 0); 
    // unbind VBO
	glBindVertexArray(0); 	    
}

void plMesh::draw(const std::vector<plOrderPair> &order) const
{
    plSeq<unsigned int> indices;
	indices.reserve(_numIndices);
	
	for (unsigned int i = 0; i < _numIndices/3; i++)
	{
	    indices.add( order[i].index*3 );
	    indices.add( order[i].index*3+1 );
	    indices.add( order[i].index*3+2 );
	}

    // bind vertex array object
	glBindVertexArray(_vertexArrayObject);		
    // buffer new index data
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, _numIndices * sizeof(unsigned int), &indices[0], GL_STREAM_DRAW);        
	// draw batch
	glDrawElements( GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0); 
    // unbind VBO
	glBindVertexArray(0); 
}


void plMesh::generateSphere(float radius, int slices, int stacks) 
{
    float rho, drho, theta, dtheta;
    float x, y, z;
    int imin, imax;
    PLint base;

    drho = PL_PI / stacks;
    dtheta = 2.0f * PL_PI / slices;
    
    plSeq<plVector3> interleaved_vertices; 
    plSeq<unsigned int> indices;

    // draw +Z end as a triangle fan
    // centre of triangle fan
    interleaved_vertices.add( plVector3(0.0f, 0.0f, radius));   // position
    interleaved_vertices.add( plVector3(0.0f, 0.0f, 1.0f));     // normal    
    for (int j = 0; j <= slices; j++) 
    {
        theta = (j == slices) ? 0.0f : j * dtheta;
        x = -sin(theta) * sin(drho);
        y = cos(theta) * sin(drho);
        z = cos(drho);
    
        interleaved_vertices.add( plVector3(x * radius, y * radius, z * radius)); // position
        interleaved_vertices.add( plVector3(x, y, z) );                           // normal
    }

    for (int j = 1; j <= slices; j++) 
    {
        indices.add(0);
        indices.add(j);
        indices.add(j+1);
    }
     
    imin = 1;
    imax = stacks - 1;

    // draw intermediate stacks as quad strips
    for (int i = imin; i < imax; i++) 
    {
        base = interleaved_vertices.size()/2;

        rho = i * drho;

        for (int j = 0; j <= slices; j++) 
        {
            theta = (j == slices) ? 0.0f : j * dtheta;
            
            x = -sin(theta) * sin(rho);
            y = cos(theta) * sin(rho);
            z = cos(rho);

            interleaved_vertices.add( plVector3(x * radius, y * radius, z * radius)); // position
            interleaved_vertices.add( plVector3(x, y, z));                            // normal

            x = -sin(theta) * sin(rho + drho);
            y = cos(theta) * sin(rho + drho);
            z = cos(rho + drho);

            interleaved_vertices.add( plVector3(x * radius, y * radius, z * radius)); // position
            interleaved_vertices.add( plVector3(x, y, z));                            // normal
        }

        for (int j = 0; j < slices*2; j+=2) 
        {
            indices.add(base+j);
            indices.add(base+j+1);
            indices.add(base+j+2);

            indices.add(base+j+1);
            indices.add(base+j+3);
            indices.add(base+j+2);
        }
    }

    // draw -Z end as a triangle fan
    base = interleaved_vertices.size()/2;

    interleaved_vertices.add( plVector3(0.0f, 0.0f, -radius));   // position
    interleaved_vertices.add( plVector3(0.0f, 0.0f, -1.0f));     // normal

    rho = PL_PI - drho;

    for (int j = slices; j >= 0; j--) 
    {
        theta = (j == slices) ? 0.0f : j * dtheta;
        x = -sin(theta) * sin(rho);
        y = cos(theta) * sin(rho);
        z = cos(rho);

        interleaved_vertices.add( plVector3(x * radius, y * radius, z * radius)); // position
        interleaved_vertices.add( plVector3(x, y, z));                            // normal
    }

    for (int j = 1; j <= slices; j++) 
    {
        indices.add(base+0);
        indices.add(base+j);
        indices.add(base+j+1);
    }

    // set index count
	_numIndices = indices.size();
    // set VBOs and VAO
    setBuffers(interleaved_vertices, indices);
}


void plMesh::generateCylinder(float baseRadius, float topRadius, float height, int slices, int stacks) 
{
 
    float da, r, dr, dz;
    float x, y, z, nz;

    da = 2.0f * PL_PI / slices;
    dr = (topRadius - baseRadius) / stacks;
    dz = height / stacks;
    nz = (baseRadius - topRadius) / height;
   
    // Z component of normal vectors

    z = 0.0f;
    r = baseRadius;
    
    plSeq<plVector3> interleaved_vertices; 
    plSeq<unsigned int> indices;
    
    for (int j = 0; j < stacks; j++) 
    {
        PLint base = interleaved_vertices.size()/2;
        
        for (int i = 0; i <= slices; i++) 
        {
            if (i == slices) 
            {
                x = sin(0.0f);
                y = cos(0.0f);
            } else {
                x = sin((i * da));
                y = cos((i * da));
            }
            
            interleaved_vertices.add( plVector3(x * r, y * r, z) );  // position
            interleaved_vertices.add( plVector3(x, y, nz) );         // normal

            interleaved_vertices.add( plVector3( x * (r + dr), y * (r + dr), z + dz) );  // position
            interleaved_vertices.add( plVector3(x, y, nz) );                             // normal                        
        } 
        
        for (int i = 0; i < slices*2; i+=2) 
        {
            indices.add(base+i);
            indices.add(base+i+1);
            indices.add(base+i+2);

            indices.add(base+i+1);
            indices.add(base+i+3);
            indices.add(base+i+2);
        }
        r += dr;
        z += dz;
        
    }
    
    // set index count
	_numIndices = indices.size();
    // set VBOs and VAO
    setBuffers(interleaved_vertices, indices);
}       

void plMesh::generateDisk(float innerRadius, float outerRadius, int slices, int loops, bool up)
{
    float da, dr;

    plVector3 normal = (up) ? plVector3(0.0f, 0.0f, 1.0f) : plVector3(0.0f, 0.0f, -1.0f);

    da = 2.0f * PL_PI / slices;
    dr = (outerRadius - innerRadius) /  loops;

    float sa, ca;
    float r1 = innerRadius;

    plSeq<plVector3> interleaved_vertices; 
    plSeq<unsigned int> indices;

    for (int l = 0; l < loops; l++) 
    {
        float r2 = r1 + dr;
        
        PLint base = interleaved_vertices.size()/2;

        if (up) 
        {
            for (int s = 0; s <= slices; s++) 
            {
                float a = (s == slices) ? 0.0f : s * da;
                
                sa = sin(a);
                ca = cos(a);
                
                interleaved_vertices.add( plVector3(r2 * sa, r2 * ca, 0.0f) );  // position
                interleaved_vertices.add( normal );                             // normal
                
                interleaved_vertices.add( plVector3(r1 * sa, r1 * ca, 0.0f) );  // position
                interleaved_vertices.add( normal );                             // normal
            }
        }
        else 
        {
            for (int s = slices; s >= 0; s--) 
            {
                float a = (s == slices) ? 0.0f : s * da;
                
                sa = sin(a);
                ca = cos(a);
                
                interleaved_vertices.add( plVector3(r2 * sa, r2 * ca, 0.0f) );  // position
                interleaved_vertices.add( normal );                             // normal
                
                interleaved_vertices.add( plVector3(r1 * sa, r1 * ca, 0.0f) );  // position
                interleaved_vertices.add( normal );                             // normal
            }
        }
        
        r1 = r2;
        
        for (int i = 0; i < slices*2; i+=2) 
        {
            indices.add(base+i);
            indices.add(base+i+1);
            indices.add(base+i+2);

            indices.add(base+i+1);
            indices.add(base+i+3);
            indices.add(base+i+2);
        }
        
    }
    
    // set index count
	_numIndices = indices.size();
    // set VBOs and VAO
    setBuffers(interleaved_vertices, indices);
}

