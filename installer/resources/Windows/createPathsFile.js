////////////////////////////////////////////////////////////////////////////////
//
// createPathsFile.js
//
// Copyright (c) 2013 - 2014. Little Endian Ltd. All rights reserved.
//
////////////////////////////////////////////////////////////////////////////////

function stripTrailingSlash(str)
{
   if ( str.charAt( str.length - 1 ) == "\\" ) { str = str.substr( 0, str.length - 1 ); }
   return str
}

function createPathsFile_CA()
{
    try
    {
        var data        = Session.Property( "CustomActionData" );
        var paths       = data.split( "|" );
        var vst24root   = stripTrailingSlash( paths[ 0 ] );
        var supportRoot = stripTrailingSlash( paths[ 1 ] );
        var fso         = new ActiveXObject( "Scripting.FileSystemObject" );
        var textStream  = fso.CreateTextFile( vst24root + "\\SpectrumWorx.paths", true );
        textStream.Write( supportRoot + "\x0A" + supportRoot + "\\Presets" );
    }
    catch (exc1) {
        Session.Property( "CA_EXCEPTION" ) = exc1.message ;
        return 3;
    }
    return 1;
}
