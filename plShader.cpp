#include "plShader.h"

plShader::plShader(const char *shaderFile, GLenum shaderType)
{
    // create compute shader object 
    GLuint shader = _createShader(shaderFile, shaderType);
    // compile compute shader object, check for error
    if(_compileShader(shader) == GL_FALSE)
	{			
        // print error
        _printCompileError(shader);
        // delete current objects and abort constructor
		glDeleteShader(shader);
		_shaderProgramID = (GLuint)NULL;               
		return;
	}

	// create program
	_shaderProgramID = glCreateProgram();
	// attach shader 		
	glAttachShader(_shaderProgramID, shader);
	
    // link program, check for error
	if(_linkProgram() == GL_FALSE)
	{
        // print error
        _printLinkError();                
		_shaderProgramID = (GLuint)NULL;
	}

	// free shader objects
	glDeleteShader(shader);          
}  
  
plShader::plShader(const char *vertexFile, const char *fragmentFile)
{         
    // create vertex shader object 
    GLuint vertexShader = _createShader(vertexFile, GL_VERTEX_SHADER);
    // compile vertex shader object, check for error
    if(_compileShader(vertexShader) == GL_FALSE)
	{			
        // print error
        _printCompileError(vertexShader);
        // delete current objects and abort constructor
		glDeleteShader(vertexShader);
		_shaderProgramID = (GLuint)NULL;               
		return;
	}
    // create fragment shader object
    GLuint fragmentShader = _createShader(fragmentFile, GL_FRAGMENT_SHADER);
    // compile fragment shader object, check for error
    if(_compileShader(fragmentShader) == GL_FALSE)
	{
		// print error
        _printCompileError(fragmentShader);
        // delete current objects and abort constructor
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
		_shaderProgramID = (GLuint)NULL;
		return;
	} 
	// create program
	_shaderProgramID = glCreateProgram();
	// attach shaders
	glAttachShader(_shaderProgramID, vertexShader);
	glAttachShader(_shaderProgramID, fragmentShader);
	
	// bind attribute indices, bind fragment shader outputs
    // ** must be done before linking ** 
    // no issues if specific attribute is not present in shader
    //bindAttributeLocations();   
    //bindFragDataLocations();    

    // link program, check for error
	if(_linkProgram() == GL_FALSE)
	{
        // print error
        _printLinkError();                
		_shaderProgramID = (GLuint)NULL;
	}

	// free shader objects
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);   
	
}

plShader::~plShader()
{
	glDeleteProgram(_shaderProgramID);
}

char* plShader::_readShaderFile(const char *filename)
{
    // THIS FUNCTION DOES NOT DE-ALLOCATE HEAP MEMORY!
    std::ifstream stream (filename, std::ifstream::in |         // input stream
                                    std::ifstream::binary |     // consider stream as binary 
                                    std::ifstream::ate);        // set position indicator at end of stream
    char *string;
    if (stream.is_open())
    {				
        std::streamoff size = stream.tellg();
        string = new char[static_cast<PLuint>(size)+1];	        // ** allocates memory but does NOT deallocate **
        stream.seekg(0, std::ios::beg);                         // set position indicator back to beginning of stream
        stream.read(string, size);
        stream.close();
        string[size] = '\0';	                                // null terminate
        return string;
    }
    // print error message
	fprintf(stdout, "Error opening shader file");
    return NULL;
}

GLuint plShader::_createShader(const char *shaderFile, GLenum shaderType)
{
    // load shader file into memory
	const char *shaderSource = _readShaderFile(shaderFile);      // allocate memory to shaderSource 
    // create shader object
	GLuint shader = glCreateShader(shaderType);
    // set source code of shader object
	glShaderSource(shader, 1, &shaderSource, NULL);
    delete [] shaderSource;                                     // deallocate memory from shaderSource
    // return shader object            
    return shader;          
}


GLuint plShader::_compileShader(GLuint shader)
{
    // compile vertex shader
	glCompileShader(shader);          
	// error check
    GLint result;           
	glGetShaderiv(shader, GL_COMPILE_STATUS, &result);          
    // return error code
    return result; 
}


void plShader::_printCompileError(GLuint shader)
{
    // get info log length
    GLint infoLogLength;    
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
    // get error message
    std::vector<char> errorMessage(infoLogLength);  
    glGetShaderInfoLog(shader, infoLogLength, NULL, &errorMessage[0]);
    // print error message
	fprintf(stdout, "%s\n", &errorMessage[0]);
}


GLuint plShader::_linkProgram()
{
    // link shader program
	glLinkProgram(_shaderProgramID);           
	// error check
    GLint result; 
	glGetProgramiv(_shaderProgramID, GL_LINK_STATUS, &result);
    // return error code
    return result; 
}


void plShader::_printLinkError()
{
    // get info log length
    GLint infoLogLength;
    glGetShaderiv(_shaderProgramID, GL_INFO_LOG_LENGTH, &infoLogLength);
    // get error message
    std::vector<char> errorMessage(infoLogLength);
	glGetShaderInfoLog(_shaderProgramID, infoLogLength, NULL, &errorMessage[0]);
    // print error message
	fprintf(stdout, "%s\n", &errorMessage[0]);
}


void reportOpenGLError( const std::string &str ) 
{
    GLuint errnum;
    const char *errstr;
    while ( errnum = glGetError() ) 
    {
        errstr = reinterpret_cast<const char*> ( gluErrorString(errnum) );
        std::cout << str << " " << errstr << "\n";
    }
}


void checkOpenGLImplementation()
{
    GLint data[256];
    
    std::cout << "OpenGL Compute Shader Uniform and Shader Storage blocks: " << data[0] << std::endl << std::endl;

        glGetIntegerv( GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS, data );    
        std::cout << "\tGL_MAX_SHADER_STORAGE_BUFFER_BINDINGS: " << data[0] << std::endl;
        
        glGetIntegerv( GL_MAX_SHADER_STORAGE_BLOCK_SIZE, data );    
        std::cout << "\tGL_MAX_SHADER_STORAGE_BLOCK_SIZE: " << data[0] << std::endl;
    
        glGetIntegerv( GL_MAX_COMPUTE_SHADER_STORAGE_BLOCKS, data );    
        std::cout << "\tGL_MAX_COMPUTE_SHADER_STORAGE_BLOCKS: " << data[0] << std::endl;

        glGetIntegerv( GL_MAX_COMBINED_SHADER_STORAGE_BLOCKS, data );    
        std::cout << "\tGL_MAX_COMBINED_SHADER_STORAGE_BLOCKS: " << data[0] << std::endl;

        glGetIntegerv( GL_MAX_COMPUTE_UNIFORM_BLOCKS, data );    
        std::cout << "\tGL_MAX_COMPUTE_UNIFORM_BLOCKS: " << data[0] << std::endl;

        glGetIntegerv( GL_MAX_COMPUTE_UNIFORM_COMPONENTS, data );    
        std::cout << "\tGL_MAX_COMPUTE_UNIFORM_COMPONENTS: " << data[0] << std::endl;
    
        glGetIntegerv( GL_MAX_COMBINED_UNIFORM_BLOCKS, data );    
        std::cout << "\tGL_MAX_COMBINED_UNIFORM_BLOCKS: " << data[0] << std::endl;
        
        glGetIntegerv( GL_MAX_UNIFORM_BLOCK_SIZE, data );    
        std::cout << "\tGL_MAX_UNIFORM_BLOCK_SIZE: " << data[0] << std::endl;
        
        glGetIntegerv( GL_MAX_UNIFORM_LOCATIONS, data );    
        std::cout << "\tGL_MAX_UNIFORM_LOCATIONS: " << data[0] << std::endl;

    std::cout << std::endl << "OpenGL Compute Shader Invocations and work Groups: " << data[0] << std::endl << std::endl;;
    
        glGetIntegerv( GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, data );    
        std::cout << "\tGL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS: " << data[0] << std::endl;
        
        glGetIntegerv( GL_MAX_COMPUTE_WORK_GROUP_COUNT, data );    
        std::cout << "\tGL_MAX_COMPUTE_WORK_GROUP_COUNT: " << data[0] << std::endl;
        
        glGetIntegerv( GL_MAX_COMPUTE_WORK_GROUP_SIZE, data );    
        std::cout << "\tGL_MAX_COMPUTE_WORK_GROUP_SIZE: " << data[0] << std::endl;

}


PLuint createSSBO( PLuint numBytes, const GLvoid *buffer )
{
    PLuint bufferID;
    glGenBuffers(1, &bufferID);     
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferID);
    glBufferData(GL_SHADER_STORAGE_BUFFER, numBytes, buffer, GL_STREAM_COPY);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    return bufferID;
} 

