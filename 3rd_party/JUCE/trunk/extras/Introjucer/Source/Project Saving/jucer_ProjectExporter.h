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

#ifndef __JUCER_PROJECTEXPORTER_JUCEHEADER__
#define __JUCER_PROJECTEXPORTER_JUCEHEADER__

#include "../jucer_Headers.h"
#include "../Project/jucer_Project.h"
#include "../Project/jucer_ProjectType.h"


//==============================================================================
class ProjectExporter
{
public:
    ProjectExporter (Project&, const ValueTree& settings);
    virtual ~ProjectExporter();

    static StringArray getExporterNames();

    static ProjectExporter* createNewExporter (Project&, const int index);
    static ProjectExporter* createNewExporter (Project&, const String& name);
    static ProjectExporter* createExporter (Project&, const ValueTree& settings);
    static bool canProjectBeLaunched (Project*);

    static String getCurrentPlatformExporterName();

    //=============================================================================
    virtual bool usesMMFiles() const = 0;
    virtual void createExporterProperties (PropertyListBuilder&) = 0;
    virtual bool canLaunchProject() = 0;
    virtual bool launchProject() = 0;
    virtual void create (const OwnedArray<LibraryModule>&) const = 0; // may throw a SaveError
    virtual bool shouldFileBeCompiledByDefault (const RelativePath& path) const;
    virtual bool canCopeWithDuplicateFiles() = 0;

    virtual bool isXcode() const                { return false; }
    virtual bool isVisualStudio() const         { return false; }
    virtual bool isWindows() const              { return false; }
    virtual int getVisualStudioVersion() const  { return 0; }
    virtual bool isLinux() const                { return false; }
    virtual bool isOSX() const                  { return false; }
    virtual bool isAndroid() const              { return false; }
    virtual bool isCodeBlocks() const           { return false; }

    //==============================================================================
    String getName() const                      { return name; }
    File getTargetFolder() const;

    Project& getProject() noexcept              { return project; }
    const Project& getProject() const noexcept  { return project; }

    Value getSetting (const Identifier& nm)     { return settings.getPropertyAsValue (nm, project.getUndoManagerFor (settings)); }
    String getSettingString (const Identifier& nm) const  { return settings [nm]; }

    Value getJuceFolderValue()                  { return getSetting (Ids::juceFolder); }
    String getJuceFolderString() const          { return getSettingString (Ids::juceFolder); }

    Value getTargetLocationValue()              { return getSetting (Ids::targetFolder); }
    String getTargetLocationString() const      { return getSettingString (Ids::targetFolder); }

    Value getExtraCompilerFlags()               { return getSetting (Ids::extraCompilerFlags); }
    String getExtraCompilerFlagsString() const  { return getSettingString (Ids::extraCompilerFlags).replaceCharacters ("\r\n", "  "); }

    Value getExtraLinkerFlags()                 { return getSetting (Ids::extraLinkerFlags); }
    String getExtraLinkerFlagsString() const    { return getSettingString (Ids::extraLinkerFlags).replaceCharacters ("\r\n", "  "); }

    Value getExternalLibraries()                { return getSetting (Ids::externalLibraries); }
    String getExternalLibrariesString() const   { return getSettingString (Ids::externalLibraries).replaceCharacters ("\r\n", " ;"); }

    Value getUserNotes()                        { return getSetting (Ids::userNotes); }

    RelativePath rebaseFromProjectFolderToBuildTarget (const RelativePath& path) const;
    void addToExtraSearchPaths (const RelativePath& pathFromProjectFolder);

    Value getBigIconImageItemID()               { return getSetting (Ids::bigIcon); }
    Value getSmallIconImageItemID()             { return getSetting (Ids::smallIcon); }
    Image getBigIcon() const;
    Image getSmallIcon() const;
    Image getBestIconForSize (int size, bool returnNullIfNothingBigEnough) const;

    String getExporterIdentifierMacro() const
    {
        return "JUCER_" + settings.getType().toString() + "_"
                + String::toHexString (getSettingString (Ids::targetFolder).hashCode()).toUpperCase();
    }

    // An exception that can be thrown by the create() method.
    class SaveError
    {
    public:
        SaveError (const String& error) : message (error)
        {}

        SaveError (const File& fileThatFailedToWrite)
            : message ("Can't write to the file: " + fileThatFailedToWrite.getFullPathName())
        {}

        String message;
    };

    RelativePath getJucePathFromTargetFolder() const;
    RelativePath getJucePathFromProjectFolder() const;

    void createPropertyEditors (PropertyListBuilder& props);

    //==============================================================================
    void copyMainGroupFromProject();
    Array<Project::Item>& getAllGroups() noexcept               { jassert (itemGroups.size() > 0); return itemGroups; }
    const Array<Project::Item>& getAllGroups() const noexcept   { jassert (itemGroups.size() > 0); return itemGroups; }
    Project::Item& getModulesGroup();

    //==============================================================================
    String xcodePackageType, xcodeBundleSignature, xcodeBundleExtension;
    String xcodeProductType, xcodeProductInstallPath, xcodeFileType;
    String xcodeOtherRezFlags, xcodeExcludedFiles64Bit;
    bool xcodeIsBundle, xcodeCreatePList, xcodeCanUseDwarf;
    StringArray xcodeFrameworks;
    Array<RelativePath> xcodeExtraLibrariesDebug, xcodeExtraLibrariesRelease;
    Array<XmlElement> xcodeExtraPListEntries;

    //==============================================================================
    String makefileTargetSuffix;
    bool makefileIsDLL;
    StringArray linuxLibs;

    //==============================================================================
    String msvcTargetSuffix;
    StringPairArray msvcExtraPreprocessorDefs;
    bool msvcIsDLL, msvcIsWindowsSubsystem;
    String msvcDelayLoadedDLLs;
    StringArray mingwLibs;

    //==============================================================================
    StringArray extraSearchPaths;

    //==============================================================================
    class BuildConfiguration  : public ReferenceCountedObject
    {
    public:
        BuildConfiguration (Project& project, const ValueTree& configNode);
        ~BuildConfiguration();

        typedef ReferenceCountedObjectPtr<BuildConfiguration> Ptr;

        //==============================================================================
        virtual void createConfigProperties (PropertyListBuilder&) = 0;

        //==============================================================================
        Value getNameValue()                                { return getValue (Ids::name); }
        String getName() const                              { return config [Ids::name]; }

        Value isDebugValue()                                { return getValue (Ids::isDebug); }
        bool isDebug() const                                { return config [Ids::isDebug]; }

        Value getTargetBinaryName()                         { return getValue (Ids::targetName); }
        String getTargetBinaryNameString() const            { return config [Ids::targetName]; }

        // the path relative to the build folder in which the binary should go
        Value getTargetBinaryRelativePath()                 { return getValue (Ids::binaryPath); }
        String getTargetBinaryRelativePathString() const    { return config [Ids::binaryPath]; }

        Value getOptimisationLevel()                        { return getValue (Ids::optimisation); }
        int getOptimisationLevelInt() const                 { return config [Ids::optimisation]; }
        String getGCCOptimisationFlag() const;

        Value getBuildConfigPreprocessorDefs()              { return getValue (Ids::defines); }
        String getBuildConfigPreprocessorDefsString() const { return config [Ids::defines]; }
        StringPairArray getAllPreprocessorDefs() const; // includes inherited definitions

        Value getHeaderSearchPathValue()                    { return getValue (Ids::headerPath); }
        String getHeaderSearchPathString() const            { return config [Ids::headerPath]; }
        StringArray getHeaderSearchPaths() const;

        Value getLibrarySearchPathValue()                   { return getValue (Ids::libraryPath); }
        String getLibrarySearchPathString() const           { return config [Ids::libraryPath]; }
        StringArray getLibrarySearchPaths() const;
        String getGCCLibraryPathFlags() const;

        Value getUserNotes()                                { return getValue (Ids::userNotes); }

        Value getValue (const Identifier& nm)               { return config.getPropertyAsValue (nm, getUndoManager()); }
        UndoManager* getUndoManager() const                 { return project.getUndoManagerFor (config); }

        void createPropertyEditors (PropertyListBuilder&);
        void removeFromExporter();

        //==============================================================================
        ValueTree config;
        Project& project;

    protected:

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BuildConfiguration)
    };

    void addNewConfiguration (const BuildConfiguration* configToCopy);
    bool hasConfigurationNamed (const String& name) const;
    String getUniqueConfigName (String name) const;

    String getExternalLibraryFlags (const BuildConfiguration& config) const;

    //==============================================================================
    struct ConfigIterator
    {
        ConfigIterator (ProjectExporter& exporter);

        bool next();

        BuildConfiguration& operator*() const       { return *config; }
        BuildConfiguration* operator->() const      { return config; }

        BuildConfiguration::Ptr config;
        int index;

    private:
        ProjectExporter& exporter;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ConfigIterator)
    };

    struct ConstConfigIterator
    {
        ConstConfigIterator (const ProjectExporter& exporter);

        bool next();

        const BuildConfiguration& operator*() const       { return *config; }
        const BuildConfiguration* operator->() const      { return config; }

        BuildConfiguration::Ptr config;
        int index;

    private:
        const ProjectExporter& exporter;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ConstConfigIterator)
    };

    int getNumConfigurations() const;
    BuildConfiguration::Ptr getConfiguration (int index) const;

    ValueTree getConfigurations() const;
    void createDefaultConfigs();

    static const Identifier configurations, configuration;

    //==============================================================================
    Value getExporterPreprocessorDefs()                 { return getSetting (Ids::extraDefs); }
    String getExporterPreprocessorDefsString() const    { return getSettingString (Ids::extraDefs); }

    // includes exporter, project + config defs
    StringPairArray getAllPreprocessorDefs (const BuildConfiguration& config) const;
    // includes exporter + project defs..
    StringPairArray getAllPreprocessorDefs() const;

    String replacePreprocessorTokens (const BuildConfiguration&, const String& sourceString) const;

    ValueTree settings;

    //==============================================================================
    enum OptimisationLevel
    {
        optimisationOff = 1,
        optimiseMinSize = 2,
        optimiseMaxSpeed = 3
    };

protected:
    //==============================================================================
    String name;
    Project& project;
    const ProjectType& projectType;
    const String projectName;
    const File projectFolder;

    mutable Array<Project::Item> itemGroups;
    void initItemGroups() const;
    Project::Item* modulesGroup;

    virtual BuildConfiguration::Ptr createBuildConfig (const ValueTree&) const = 0;

    static String getDefaultBuildsRootFolder()            { return "Builds/"; }

    static String getLibbedFilename (String name)
    {
        if (! name.startsWith ("lib"))
            name = "lib" + name;
        if (! name.endsWithIgnoreCase (".a"))
            name = name + ".a";
        return name;
    }

    //==============================================================================
    static void overwriteFileIfDifferentOrThrow (const File& file, const MemoryOutputStream& newData)
    {
        if (! FileHelpers::overwriteFileWithNewDataIfDifferent (file, newData))
            throw SaveError (file);
    }

    static void createDirectoryOrThrow (const File& dirToCreate)
    {
        if (! dirToCreate.createDirectory())
            throw SaveError ("Can't create folder: " + dirToCreate.getFullPathName());
    }

    static void writeXmlOrThrow (const XmlElement& xml, const File& file, const String& encoding, int maxCharsPerLine, bool useUnixNewLines = false)
    {
        MemoryOutputStream mo;
        xml.writeToStream (mo, String::empty, false, true, encoding, maxCharsPerLine);

        if (useUnixNewLines)
        {
            MemoryOutputStream mo2;
            mo2 << mo.toString().replace ("\r\n", "\n");
            overwriteFileIfDifferentOrThrow (file, mo2);
        }
        else
        {
            overwriteFileIfDifferentOrThrow (file, mo);
        }
    }

    static Image rescaleImageForIcon (Image image, int iconSize);

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProjectExporter)
};


#endif   // __JUCER_PROJECTEXPORTER_JUCEHEADER__
