//-*****************************************************************************
//
// Copyright (c) 2013,
//  Sony Pictures Imageworks Inc. and
//  Industrial Light & Magic, a division of Lucasfilm Entertainment Company Ltd.
//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
// *       Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
// *       Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
// *       Neither the name of Sony Pictures Imageworks, nor
// Industrial Light & Magic, nor the names of their contributors may be used
// to endorse or promote products derived from this software without specific
// prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//-*****************************************************************************

#include <Alembic/AbcCoreOgawa/StreamManager.h>

namespace Alembic {
namespace AbcCoreOgawa {
namespace ALEMBIC_VERSION_NS {

StreamManager::StreamManager( std::size_t iNumStreams )
{

    // only do this if we have more than 1 stream
    // otherwise we don't need to lock
    if ( iNumStreams > 1 )
    {
        m_streamIDs.resize( iNumStreams );
        for ( std::size_t i = 0; i < iNumStreams; ++i )
        {
            m_streamIDs[i] = i;
        }
    }

    m_curStream = 0;
    m_default( new StreamID( NULL, 0 ) );
}

StreamManager::~StreamManager()
{
}

StreamIDPtr StreamManager::get()
{
    // no need to lock
    if ( m_streamIDs.empty() )
    {
        return m_default;
    }

    Alembic::Util::scoped_lock l( m_lock );

    // we've used up more than we have, just return the default
    if ( m_curStream == m_streamIDs.size() )
    {
        return m_default;
    }

    m_curStream ++;
    return StreamIDPtr( new StreamID( this, m_streamIDs[ m_curStream - 1 ] ) );
}

void StreamManager::put( std::size_t iStreamID )
{
    // shouldn't ever hit this case, it's why we have m_default
    assert( iStreamID < m_streamIDs.size() && m_curStream > 0 );

    Alembic::Util::scoped_lock l( m_lock );
    m_curStream --;
    m_streamIDs[ m_curStream ] = iStreamID;
}

StreamID::StreamID( StreamManager * iManager, std::size_t iStreamID ) :
    m_manager( iManager ), m_streamID( iStreamID )
{
}

StreamID::~StreamID()
{
    // if our manager is valid, give back our ID
    if ( m_manager != NULL )
    {
        m_manager->put( m_streamID );
    }
}

} // End namespace ALEMBIC_VERSION_NS
} // End namespace Ogawa
} // End namespace Alembic
