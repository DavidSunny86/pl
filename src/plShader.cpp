#include "plShader.h"


plShader::plShader()
    :   _id( 0 ), _good( true )
{
}


plShader::~plShader()
{
	glDeleteProgram(_id);
}


GLuint plShader::_createShader( const std::string &shaderFile, GLenum shaderType )
{
    // load shader file into memory
	const char *shaderSource = _readShaderFile( shaderFile );     // allocate memory to shaderSource
    // check for error
    if ( !shaderSource )
    {
        delete [] shaderSource;
        return 0;
    }
    // create shader object
	GLuint shader = glCreateShader(shaderType);
    // set source code of shader object
	glShaderSource( shader, 1, &shaderSource, NULL );
    delete [] shaderSource;                                     // deallocate memory from shaderSource
    // return shader object
    return shader;
}


GLuint plShader::_createShader( const std::vector<std::string> &shaderFiles, GLenum shaderType )
{
    std::vector<const char*> sources;

    for (PLuint i=0; i< shaderFiles.size(); i++)
    {
        // load shader file into memory
        const char *shaderSource = _readShaderFile( shaderFiles[i] );  // allocate memory to shaderSource

        // check for error
        if ( !shaderSource )
        {
            // deallocate previous arrays
            for (PLuint j=0; j<i; j++)
            {
                delete [] sources[j];
            }
            return 0;
        }
        sources.push_back( _readShaderFile( shaderFiles[i] ) );  // allocate memory to shaderSource
    }

    // create shader object
	GLuint shader = glCreateShader(shaderType);

    // set source code of shader object
	glShaderSource( shader, sources.size(), &sources[0], NULL );
    for (PLuint i=0; i< shaderFiles.size(); i++)
    {
        delete [] sources[i];  // deallocate memory from shaderSource
    }

    // return shader object
    return shader;
}


char* plShader::_readShaderFile( const std::string& filename )
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
    std::cerr << "plShader::_readShaderFile() error: could not open shader file: " << filename << std::endl;
    return NULL;
}


PLbool plShader::_compileShader( GLuint shader )
{
    // compile vertex shader
	glCompileShader( shader );
	// error check
    GLint result;
	glGetShaderiv( shader, GL_COMPILE_STATUS, &result );

    if ( result == GL_FALSE )
	{
        // print error
        _printCompileError(shader);
        // delete current objects and abort constructor
		glDeleteShader(shader);
		_id = 0;
		_good = false;
		return false;
	}
    return true;
}


GLuint plShader::_createAndCompileShader( const std::string &shaderFile, GLenum shaderType )
{
    // create shader object
    GLuint shader = _createShader( shaderFile, shaderType );
    // compile shader object, check for error
    if( !_compileShader( shader ) )
        return 0;
    return shader;
}


GLuint plShader::_createAndCompileShader( const std::vector<std::string> &shaderFiles, GLenum shaderType )
{
    // create shader object
    GLuint shader = _createShader( shaderFiles, shaderType );
    // compile shader object, check for error
    if( !_compileShader( shader ) )
        return 0;
    return shader;
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
	fprintf(stdout, "Compilation errors:\n%s\n", &errorMessage[0]);
}


PLbool plShader::_linkProgram()
{
    // link shader program
	glLinkProgram( _id );
	// error check
    GLint result;
	glGetProgramiv( _id, GL_LINK_STATUS, &result );
    // check for error
    if ( result == GL_FALSE )
	{
        // print error
        _printLinkError();
		_id = 0;
		_good = false;
        return false;
    }
    return true;
}


void plShader::_printLinkError()
{
    // get info log length
    GLint infoLogLength;
    glGetProgramiv( _id, GL_INFO_LOG_LENGTH, &infoLogLength );
    // get error message
    std::vector<char> errorMessage(infoLogLength);
	glGetProgramInfoLog( _id, infoLogLength, NULL, &errorMessage[0] );
    // print error message
	fprintf(stdout, "Linking errors:\n%s\n", &errorMessage[0]);
}
