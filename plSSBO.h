#ifndef PL_SSBO_H
#define PL_SSBO_H

#include "plCommon.h"
#include "plBufferObject.h"

class plSSBO : public plBufferObject
{

    public:

        plSSBO();
        plSSBO( PLuint numBytes, const void *buffer = NULL );

        plSSBO( const plSSBO& ssbo );
        plSSBO( plSSBO&& ssbo );

        plSSBO& operator= ( plSSBO&& ssbo );
        plSSBO& operator= ( const plSSBO& ssbo );

        void bind  ( PLuint location ) const { glBindBufferBase( GL_SHADER_STORAGE_BUFFER, location, _id ); }
        void unbind( PLuint location ) const { glBindBufferBase( GL_SHADER_STORAGE_BUFFER, location, 0   ); }

        // set
        template < class T >
        void set( std::vector<T> &ts, PLuint count, PLuint index = 0, PLuint ssboIndex = 0 );

        template < class T >
        void setBytes( T *ts, PLuint numBytes, PLuint byteOffset = 0, PLuint ssboByteOffset = 0 );

        // read
        template < class T >
        void read( std::vector<T> &ts, PLuint count, PLuint index = 0, PLuint ssboIndex = 0 ) const;

        template < class T >
        void readBytes( T *ts, PLuint numBytes, PLuint byteOffset = 0, PLuint ssboByteOffset = 0 ) const;

    private:

        void _create( PLuint numBytes, const void *buffer = NULL );
        void _copy  ( const plSSBO &ssbo );
        void _move  ( plSSBO&& ssbo );
};


template < class T >
void plSSBO::set( std::vector<T> &ts, PLuint count, PLuint index, PLuint ssboIndex )
{
    if ( count == 0 )
    {
        std::cerr << "plSSBO::set() warning: write count is 0, function ignored" << std::endl;
        return;
    }

    setBytes( &ts[0], count*sizeof( T ), index*sizeof( T ), ssboIndex*sizeof( T ) );
}


template < class T >
void plSSBO::setBytes( T *ts, PLuint numBytes, PLuint byteOffset, PLuint ssboByteOffset )
{
    if ( numBytes == 0 )
    {
        std::cerr << "plSSBO::setBytes() warning: numBytes is 0, function ignored" << std::endl;
        return;
    }

    if ( !_id || numBytes > _numBytes )
    {
        // ssbo has not been generated yet or is too small to map properly
        _create( numBytes, (void*)ts );
        return; // no need to map
    }

    glBindBuffer( GL_SHADER_STORAGE_BUFFER, _id );
    T *mappedBuffer = (T*) glMapBufferRange( GL_SHADER_STORAGE_BUFFER,
                                             ssboByteOffset,
                                             numBytes,
                                             GL_MAP_WRITE_BIT );

    memcpy( mappedBuffer, reinterpret_cast< PLchar* >( ts )+byteOffset, numBytes ); // cast to char array to ensure bytewise increments
    glUnmapBuffer( GL_SHADER_STORAGE_BUFFER );
}


template < class T >
void plSSBO::read( std::vector<T> &ts, PLuint count, PLuint index, PLuint ssboIndex ) const
{
    if ( !_id )
    {
        std::cerr << "plSSBO::read() error: SSBO buffer has not been generated" << std::endl;
        return;
    }

    if ( count == 0 )
    {
        std::cerr << "plSSBO::read() warning: read count is 0, function ignored" << std::endl;
        return;
    }

    if ( count*sizeof( T ) > _numBytes )
    {
        std::cerr << "plSSBO::read() error: requested read size larger than SSBO, function ignored" << std::endl;
        return;
    }

    if ( ts.size() < count )
         ts.resize( count );

    readBytes( &ts[0], count*sizeof( T ), index*sizeof( T ), ssboIndex*sizeof( T ) );
}


template < class T >
void plSSBO::readBytes( T *ts, PLuint numBytes, PLuint byteOffset, PLuint ssboByteOffset ) const
{
    if ( !_id )
    {
        std::cerr << "plSSBO::readBytes() error: SSBO buffer has not been generated" << std::endl;
        return;
    }

    if ( numBytes == 0 )
    {
        std::cerr << "plSSBO::readBytes() warning: numBytes is 0, function ignored" << std::endl;
        return;
    }

    if ( numBytes > _numBytes )
    {
        std::cerr << "plSSBO::readBytes() error: requested read size larger than SSBO, function ignored" << std::endl;
        return;
    }

    glBindBuffer( GL_SHADER_STORAGE_BUFFER, _id );
    T *mappedBuffer = (T*) glMapBufferRange( GL_SHADER_STORAGE_BUFFER,
                                             ssboByteOffset,
                                             numBytes,
                                             GL_MAP_READ_BIT );

    memcpy( reinterpret_cast< PLchar* >( ts )+byteOffset, &mappedBuffer[0], numBytes );
    glUnmapBuffer( GL_SHADER_STORAGE_BUFFER );
}


#endif
