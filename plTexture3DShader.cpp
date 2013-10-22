#include "plTexture3DShader.h"
                     
plTexture3DShader::plTexture3DShader(const char *vertexFile, const char *fragmentFile) 
    : plMinimalShader(vertexFile, fragmentFile)
{  
    // get uniform locations
    getUniformLocations();
}


void plTexture3DShader::getUniformLocations()
{			
    plMinimalShader::getUniformLocations();
    // texture uniform
    _textureSamplerID = glGetUniformLocation( _shaderProgramID, "textureSampler");
}
    

void plTexture3DShader::setTextureUniform() const
{	
    // set texture uniforms
    glUniform1i(_textureSamplerID,  0);  // texture unit 0
}

