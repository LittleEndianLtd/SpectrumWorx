/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#include "jucer_ResourceFile.h"
#include "../Project/jucer_ProjectTreeViewBase.h"
#include "../Application/jucer_OpenDocumentManager.h"

static const char* resourceFileIdentifierString = "JUCER_BINARY_RESOURCE";


//==============================================================================
ResourceFile::ResourceFile (Project& p)
    : project (p),
      className ("BinaryData")
{
    addResourcesFromProjectItem (project.getMainGroup());
}

ResourceFile::~ResourceFile()
{
}

bool ResourceFile::isResourceFile (const File& file)
{
    if (file.hasFileExtension ("cpp;cc;h"))
    {
        ScopedPointer <InputStream> in (file.createInputStream());

        if (in != nullptr)
        {
            MemoryBlock mb;
            in->readIntoMemoryBlock (mb, 256);
            return mb.toString().contains (resourceFileIdentifierString);
        }
    }

    return false;
}

//==============================================================================
void ResourceFile::addResourcesFromProjectItem (const Project::Item& projectItem)
{
    if (projectItem.isGroup())
    {
        for (int i = 0; i < projectItem.getNumChildren(); ++i)
            addResourcesFromProjectItem (projectItem.getChild(i));
    }
    else
    {
        if (projectItem.shouldBeAddedToBinaryResources())
            addFile (projectItem.getFile());
    }
}

//==============================================================================
void ResourceFile::setClassName (const String& name)
{
    className = name;
}

void ResourceFile::addFile (const File& file)
{
    files.add (file);

    const String variableNameRoot (CodeHelpers::makeBinaryDataIdentifierName (file));
    String variableName (variableNameRoot);

    int suffix = 2;
    while (variableNames.contains (variableName))
        variableName = variableNameRoot + String (suffix++);

    variableNames.add (variableName);
}

String ResourceFile::getDataVariableFor (const File& file) const
{
    jassert (files.indexOf (file) >= 0);
    return variableNames [files.indexOf (file)];
}

String ResourceFile::getSizeVariableFor (const File& file) const
{
    jassert (files.indexOf (file) >= 0);
    return variableNames [files.indexOf (file)] + "Size";
}

int64 ResourceFile::getTotalDataSize() const
{
    int64 total = 0;

    for (int i = 0; i < files.size(); ++i)
        total += files.getReference(i).getSize();

    return total;
}

static String getComment()
{
    String comment;
    comment << newLine << newLine
            << "   This is an auto-generated file: Any edits you make may be overwritten!" << newLine
            << newLine
            << "*/" << newLine
            << newLine;

    return comment;
}

bool ResourceFile::writeHeader (MemoryOutputStream& header)
{
    header << "/* ========================================================================================="
           << getComment()
           << "namespace " << className << newLine
           << "{" << newLine;

    bool containsAnyImages = false;

    for (int i = 0; i < files.size(); ++i)
    {
        const File& file = files.getReference(i);
        const int64 dataSize = file.getSize();

        const String variableName (variableNames[i]);

        FileInputStream fileStream (file);

        if (fileStream.openedOk())
        {
            containsAnyImages = containsAnyImages
                                 || (ImageFileFormat::findImageFormatForStream (fileStream) != nullptr);

            const String tempVariable ("temp_" + String::toHexString (file.hashCode()));

            header << "    extern const char*   " << variableName << ";" << newLine;
            header << "    const int            " << variableName << "Size = " << (int) dataSize << ";" << newLine << newLine;
        }
    }

    header << "    // If you provide the name of one of the binary resource variables above, this function will" << newLine
           << "    // return the corresponding data and its size (or a null pointer if the name isn't found)." << newLine
           << "    const char* getNamedResource (const char* resourceNameUTF8, int& dataSizeInBytes) throw();" << newLine
           << "}" << newLine;

    return true;
}

bool ResourceFile::writeCpp (MemoryOutputStream& cpp, const File& headerFile, int& i, const int maxFileSize)
{
    const bool isFirstFile = (i == 0);

    cpp << "/* ==================================== " << resourceFileIdentifierString << " ===================================="
        << getComment()
        << "namespace " << className << newLine
        << "{" << newLine;

    bool containsAnyImages = false;

    while (i < files.size())
    {
        const File& file = files.getReference(i);
        const String variableName (variableNames[i]);

        FileInputStream fileStream (file);

        if (fileStream.openedOk())
        {
            containsAnyImages = containsAnyImages
                                 || (ImageFileFormat::findImageFormatForStream (fileStream) != nullptr);

            const String tempVariable ("temp_" + String::toHexString (file.hashCode()));

            cpp  << newLine << "//================== " << file.getFileName() << " ==================" << newLine
                << "static const unsigned char " << tempVariable << "[] =" << newLine;

            {
                MemoryBlock data;
                fileStream.readIntoMemoryBlock (data);
                CodeHelpers::writeDataAsCppLiteral (data, cpp, true, true);
            }

            cpp << newLine << newLine
                << "const char* " << variableName << " = (const char*) " << tempVariable << ";" << newLine;
        }

        ++i;

        if (cpp.getPosition() > maxFileSize)
            break;
    }

    if (isFirstFile)
    {
        if (i < files.size())
        {
            cpp << newLine
                << "}" << newLine
                << newLine
                << "#include \"" << headerFile.getFileName() << "\"" << newLine
                << newLine
                << "namespace " << className << newLine
                << "{";
        }

        cpp << newLine
            << newLine
            << "const char* getNamedResource (const char*, int&) throw();" << newLine
            << "const char* getNamedResource (const char* resourceNameUTF8, int& numBytes) throw()" << newLine
            << "{" << newLine;

        StringArray returnCodes;
        for (int j = 0; j < files.size(); ++j)
        {
            const File& file = files.getReference(j);
            const int64 dataSize = file.getSize();
            returnCodes.add ("numBytes = " + String (dataSize) + "; return " + variableNames[j] + ";");
        }

        CodeHelpers::createStringMatcher (cpp, "resourceNameUTF8", variableNames, returnCodes, 4);

        cpp << "    numBytes = 0;" << newLine
            << "    return 0;" << newLine
            << "}" << newLine;
    }

    cpp << newLine
        << "}" << newLine;

    return true;
}

bool ResourceFile::write (Array<File>& filesCreated, const int maxFileSize)
{
    const File headerFile (project.getBinaryDataHeaderFile());

    {
        MemoryOutputStream mo;
        if (! (writeHeader (mo) && FileHelpers::overwriteFileWithNewDataIfDifferent (headerFile, mo)))
            return false;

        filesCreated.add (headerFile);
    }

    int i = 0;
    int fileIndex = 0;

    for (;;)
    {
        File cpp (project.getBinaryDataCppFile (fileIndex));

        MemoryOutputStream mo;
        if (! (writeCpp (mo, headerFile, i, maxFileSize) && FileHelpers::overwriteFileWithNewDataIfDifferent (cpp, mo)))
            return false;

        filesCreated.add (cpp);
        ++fileIndex;

        if (i >= files.size())
            break;
    }

    return true;
}
