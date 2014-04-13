UIElement@ browserWindow;
ListView@ browserDirList;
ListView@ browserFileList;
LineEdit@ browserSearch;
BrowserFile@ browserDragFile;
Node@ browserDragNode;
Component@ browserDragComponent;
bool browserActionMenuWaitFrame = false;
int browserSearchSortMode = 0;

BrowserDir@ rootDir;
Array<BrowserDir@> browserDirs;
Array<BrowserFile@> browserFiles;

Array<BrowserFile@> browserFilesToScan;
const uint BROWSER_WORKER_ITEMS_PER_TICK = 10;
const uint BROWSER_SEARCH_LIMIT = 50;
const int BROWSER_SORT_MODE_ALPHA = 1;
const int BROWSER_SORT_MODE_SEARCH = 2;

const int BROWSER_TYPE_UNUSABLE = -2;
const int BROWSER_TYPE_UNKNOWN = -1;
const int BROWSER_TYPE_NOTSET = 0;
const int BROWSER_TYPE_SCENE = 1;
const int BROWSER_TYPE_SCRIPTFILE = 2;
const int BROWSER_TYPE_MODEL = 3;
const int BROWSER_TYPE_MATERIAL = 4;
const int BROWSER_TYPE_ANIMATION = 5;
const int BROWSER_TYPE_IMAGE = 6;
const int BROWSER_TYPE_SOUND = 7;
const int BROWSER_TYPE_TEXTURE = 8;
const int BROWSER_TYPE_FONT = 9;
const int BROWSER_TYPE_PREFAB = 10;
const int BROWSER_TYPE_TECHNIQUE = 11;
const int BROWSER_TYPE_PARTICLEEMITTER = 12;
const int BROWSER_TYPE_UIELEMENT = 13;
const int BROWSER_TYPE_UIELEMENTS = 14;
const int BROWSER_TYPE_ANIMATION_SETTINGS = 15;
const int BROWSER_TYPE_RENDERPATH = 16;
const int BROWSER_TYPE_TEXTURE_ATLAS = 17;
const int BROWSER_TYPE_2D_PARTICLE_EFFECT = 18;
const int BROWSER_TYPE_TEXTURE_3D = 19;
const int BROWSER_TYPE_CUBEMAP = 20;

const ShortStringHash XML_TYPE_SCENE("scene");
const ShortStringHash XML_TYPE_NODE("node");
const ShortStringHash XML_TYPE_MATERIAL("material");
const ShortStringHash XML_TYPE_TECHNIQUE("technique");
const ShortStringHash XML_TYPE_PARTICLEEMITTER("particleemitter");
const ShortStringHash XML_TYPE_TEXTURE("texture");
const ShortStringHash XML_TYPE_ELEMENT("element");
const ShortStringHash XML_TYPE_ELEMENTS("elements");
const ShortStringHash XML_TYPE_ANIMATION_SETTINGS("animation");
const ShortStringHash XML_TYPE_RENDERPATH("renderpath");
const ShortStringHash XML_TYPE_TEXTURE_ATLAS("TextureAtlas");
const ShortStringHash XML_TYPE_2D_PARTICLE_EFFECT("particleEmitterConfig");
const ShortStringHash XML_TYPE_TEXTURE_3D("texture3d");
const ShortStringHash XML_TYPE_CUBEMAP("cubemap");

const ShortStringHash BINARY_TYPE_SCENE("USCN");
const ShortStringHash BINARY_TYPE_PACKAGE("UPAK");
const ShortStringHash BINARY_TYPE_COMPRESSED_PACKAGE("ULZ4");
const ShortStringHash BINARY_TYPE_ANGLESCRIPT("ASBC");
const ShortStringHash BINARY_TYPE_MODEL("UMDL");
const ShortStringHash BINARY_TYPE_SHADER("USHD");
const ShortStringHash BINARY_TYPE_ANIMATION("UANI");

const ShortStringHash EXTENSION_TYPE_TTF(".ttf");
const ShortStringHash EXTENSION_TYPE_OGG(".ogg");
const ShortStringHash EXTENSION_TYPE_WAV(".wav");
const ShortStringHash EXTENSION_TYPE_DDS(".dds");
const ShortStringHash EXTENSION_TYPE_PNG(".png");
const ShortStringHash EXTENSION_TYPE_JPG(".jpg");
const ShortStringHash EXTENSION_TYPE_JPEG(".jpeg");
const ShortStringHash EXTENSION_TYPE_TGA(".tga");
const ShortStringHash EXTENSION_TYPE_OBJ(".obj");
const ShortStringHash EXTENSION_TYPE_FBX(".fbx");
const ShortStringHash EXTENSION_TYPE_COLLADA(".dae");
const ShortStringHash EXTENSION_TYPE_BLEND(".blend");
const ShortStringHash EXTENSION_TYPE_ANGELSCRIPT(".as");
const ShortStringHash EXTENSION_TYPE_LUASCRIPT(".lua");
const ShortStringHash EXTENSION_TYPE_HLSL(".hlsl");
const ShortStringHash EXTENSION_TYPE_GLSL(".glsl");
const ShortStringHash EXTENSION_TYPE_FRAGMENTSHADER(".frag");
const ShortStringHash EXTENSION_TYPE_VERTEXSHADER(".vert");
const ShortStringHash EXTENSION_TYPE_HTML(".html");

const ShortStringHash TEXT_VAR_FILE_ID("browser_file_id");
const ShortStringHash TEXT_VAR_DIR_ID("browser_dir_id");
const ShortStringHash TEXT_VAR_BROWSER_TYPE("browser_type");

uint browserDirIndex = 1;
uint browserFileIndex = 1;
BrowserDir@ selectedBrowserDirectory;
BrowserFile@ selectedBrowserFile;
Text@ browserStatusMessage;
Text@ browserResultsMessage;

void CreateResourceBrowser()
{
    if (browserWindow !is null) return;

    CreateResourceBrowserUI();
    ScanResourceDirectories();
    PopulateBrowserDirectories();
    PopulateResourceBrowserResults(rootDir);
}

void ScanResourceDirectories()
{
    browserDirs.Clear();
    browserFiles.Clear();
    browserFilesToScan.Clear();

    rootDir = BrowserDir("");
    browserDirs.Push(rootDir);

    // collect all of the items and sort them afterwards
    for(uint i=0; i < cache.resourceDirs.length; i++)
        ScanDirectory(rootDir, i);

    for(uint i=0; i < browserDirs.length; i++)
        browserDirs[i].ScanFiles();
}

// used to stop ui from blocking while determining file types
void DoResourceBrowserWork()
{
    if (browserFilesToScan.length == 0)
        return;

    int counter = 0;
    bool updateBrowserUI = false;
    BrowserFile@ scanItem = browserFilesToScan[0];
    while(counter < BROWSER_WORKER_ITEMS_PER_TICK)
    {
        scanItem.DetermainResourceType();

        // next
        browserFilesToScan.Erase(0);
        if (browserFilesToScan.length > 0)
            @scanItem = browserFilesToScan[0];
        else
            break;
        counter++;
    }

    if (browserFilesToScan.length > 0)
        browserStatusMessage.text = "Files left to scan: " + browserFilesToScan.length;
    else
        browserStatusMessage.text = "Scan complete";

}

void CreateResourceBrowserUI()
{
    browserWindow = ui.LoadLayout(cache.GetResource("XMLFile", "UI/EditorResourceBrowser.xml"));
    browserDirList = browserWindow.GetChild("DirectoryList", true);
    browserFileList = browserWindow.GetChild("FileList", true);
    browserSearch = browserWindow.GetChild("Search", true);
    browserStatusMessage = browserWindow.GetChild("StatusMessage", true);
    browserResultsMessage = browserWindow.GetChild("ResultsMessage", true);
    browserWindow.visible = false;
    browserWindow.opacity = uiMaxOpacity;

    int height = Min(ui.root.height * .25, 300);
    browserWindow.SetSize(500, height);
    browserWindow.SetPosition(35, ui.root.height - height - 25);

    CloseContextMenu();
    ui.root.AddChild(browserWindow);

    SubscribeToEvent(browserWindow.GetChild("CloseButton", true), "Released", "HideResourceBrowserWindow");
    SubscribeToEvent(browserWindow.GetChild("RescanButton", true), "Released", "HandleRescanResourceBrowserClick");
    SubscribeToEvent(browserDirList, "SelectionChanged", "HandleResourceBrowserListSelectionChange");
    SubscribeToEvent(browserSearch, "TextChanged", "HandleResourceBrowserSearchTextChange");
    SubscribeToEvent(browserFileList, "ItemClicked", "HandleBrowserFileClick");
}

void CreateDirList(BrowserDir@ dir, UIElement@ parentUI = null)
{
    Text@ dirText = Text();
    browserDirList.InsertItem(browserDirList.numItems, dirText, parentUI);
    dirText.style = "FileSelectorListText";
    dirText.text = dir.resourceKey.empty ? "Root" : dir.name;
    dirText.name = dir.resourceKey;
    dirText.vars[TEXT_VAR_DIR_ID] = dir.id;

    // Sort directories alphetically
    browserSearchSortMode = BROWSER_SORT_MODE_ALPHA;
    dir.children.Sort();

    for(uint i=0; i<dir.children.length; i++)
        CreateDirList(dir.children[i], dirText);
}

void CreateFileList(BrowserFile@ file)
{
    Text@ fileText = Text();
    fileText.style = "FileSelectorListText";
    fileText.layoutMode = LM_HORIZONTAL;
    browserFileList.InsertItem(browserFileList.numItems, fileText);
    VariantMap params = VariantMap();
    fileText.vars[TEXT_VAR_FILE_ID] = file.id;
    fileText.vars[TEXT_VAR_DIR_ID] = file.dir.id;
    fileText.vars[TEXT_VAR_BROWSER_TYPE] = file.resourceType;
    if (file.resourceType > 0)
        fileText.dragDropMode = DD_SOURCE;

    {
        Text@ text = Text();
        fileText.AddChild(text);
        text.style = "FileSelectorListText";
        text.text = file.fullname;
        text.name = file.resourceKey;
        // text.position = IntVector2(70, 0);
    }

    {
        Text@ text = Text();
        fileText.AddChild(text);
        text.style = "FileSelectorListText";
        text.text = file.FriendlyTypeName();
    }

    if (file.resourceType == BROWSER_TYPE_MATERIAL || 
            file.resourceType == BROWSER_TYPE_MODEL ||
            file.resourceType == BROWSER_TYPE_PARTICLEEMITTER ||
            file.resourceType == BROWSER_TYPE_PREFAB
        )
    {
        SubscribeToEvent(fileText, "DragBegin", "HandleBrowserFileDragBegin");
        SubscribeToEvent(fileText, "DragEnd", "HandleBrowserFileDragEnd");
    }
}

// Opens a contextual menu based on what resource item was actioned
void HandleBrowserFileClick(StringHash eventType, VariantMap& eventData)
{
    if (eventData["Button"].GetInt() != MOUSEB_RIGHT)
        return;

    UIElement@ uiElement = eventData["Item"].GetPtr();
    BrowserFile@ file = GetBrowserFileFromUIElement(uiElement);

    if (file is null)
        return;

    Array<UIElement@> actions;
    if (file.resourceType == BROWSER_TYPE_MATERIAL)
    {
        actions.Push(CreateBrowserFileActionMenu("Edit", "HandleBrowserEditResource", file));
    }
    else if (file.resourceType == BROWSER_TYPE_MODEL)
    {
        actions.Push(CreateBrowserFileActionMenu("Instance Animated Model", "HandleBrowserInstantiateAnimatedModel", file));
        actions.Push(CreateBrowserFileActionMenu("Instance Static Model", "HandleBrowserInstantiateStaticModel", file));
    }
    else if (file.resourceType == BROWSER_TYPE_PREFAB)
    {
        actions.Push(CreateBrowserFileActionMenu("Instance Prefab", "HandleBrowserInstantiatePrefab", file));
        actions.Push(CreateBrowserFileActionMenu("Instance in Spawner", "HandleBrowserInstantiateInSpawnEditor", file));
    }
    else if (file.extensionType == EXTENSION_TYPE_OBJ ||
        file.extensionType == EXTENSION_TYPE_COLLADA ||
        file.extensionType == EXTENSION_TYPE_FBX ||
        file.extensionType == EXTENSION_TYPE_BLEND)
    {
        actions.Push(CreateBrowserFileActionMenu("Import Model", "HandleBrowserImportModel", file));
        actions.Push(CreateBrowserFileActionMenu("Import Scene", "HandleBrowserImportScene", file));
    }
    else if (file.resourceType == BROWSER_TYPE_UIELEMENT)
    {
        actions.Push(CreateBrowserFileActionMenu("Open UI Layout", "HandleBrowserOpenUILayout", file));
    }
    else if (file.resourceType == BROWSER_TYPE_SCENE)
    {
        actions.Push(CreateBrowserFileActionMenu("Load Scene", "HandleBrowserLoadScene", file));
    }
    else if (file.resourceType == BROWSER_TYPE_SCRIPTFILE)
    {
        actions.Push(CreateBrowserFileActionMenu("Execute Script", "HandleBrowserRunScript", file));
    }

    actions.Push(CreateBrowserFileActionMenu("Open", "HandleBrowserOpenResource", file));

    ActivateContextMenu(actions);
}

void ScanDirectory(BrowserDir@ dir, uint resourceDirIndex)
{
    // scan for child directories
    String resourceDir = cache.resourceDirs[resourceDirIndex];
    String fullPath = resourceDir + dir.resourceKey;

    Array<String> dirs = fileSystem.ScanDir(fullPath, "*", SCAN_DIRS, false);
    for (uint i=0; i < dirs.length; i++)
    {
        String dirName = dirs[i];
        if (dirName.StartsWith(".")) continue;

        BrowserDir@ childDir = dir.GetChild(dirName);
        if (childDir is null)
        {
            childDir = dir.AddDir(dirName);
            browserDirs.Push(childDir);
        }

        ScanDirectory(childDir, resourceDirIndex);
    }
}

void HideResourceBrowserWindow()
{
    browserWindow.visible = false;
}

bool ShowResourceBrowserWindow()
{
    browserWindow.visible = true;
    browserWindow.BringToFront();
    ui.focusElement = browserSearch;
    return true;
}

void PopulateBrowserDirectories()
{
    browserDirList.RemoveAllItems();
    CreateDirList(rootDir);
    browserDirList.selection = 0;
}

void PopulateResourceBrowserResults(BrowserDir@ dir)
{
    @selectedBrowserDirectory = dir;
    browserFileList.RemoveAllItems();
    if (dir is null) return;

    Array<String> filenames;
    for(uint x=0; x < dir.files.length; x++)
        filenames.Push(dir.files[x].fullname);

    // Sort alphetically
    browserSearchSortMode = BROWSER_SORT_MODE_ALPHA;
    dir.files.Sort();
    PopulateResourceBrowserResults(dir.files);
    browserResultsMessage.text = "Showing " + dir.files.length + " files";
}

void PopulateResourceBrowserResults(Array<BrowserFile@>@ files)
{
    browserFileList.RemoveAllItems();
    for(uint i=0; i < files.length; i++)
    {
        CreateFileList(files[i]);
    }
}

void HandleRescanResourceBrowserClick(StringHash eventType, VariantMap& eventData)
{
    ScanResourceDirectories();
    PopulateBrowserDirectories();
    PopulateResourceBrowserResults(rootDir);
}

void HandleResourceBrowserListSelectionChange(StringHash eventType, VariantMap& eventData)
{
    if (browserDirList.selection == M_MAX_UNSIGNED)
        return;

    UIElement@ uiElement = browserDirList.GetItems()[browserDirList.selection];
    BrowserDir@ dir = GetBrowserDirFromUIElement(uiElement);
    if (dir is null)
        return;
    PopulateResourceBrowserResults(dir);
}

void HandleResourceBrowserSearchTextChange(StringHash eventType, VariantMap& eventData)
{
    if (browserSearch.text.empty)
    {
        browserDirList.visible = true;
        PopulateResourceBrowserResults(selectedBrowserDirectory);
    }
    else
    {
        browserDirList.visible = false;

        String query = browserSearch.text;

        Array<int> scores;
        Array<BrowserFile@> scored;
        Array<BrowserFile@> filtered;
        {
            BrowserFile@ file;
            for(uint x=0; x < browserFiles.length; x++)
            {
                @file = browserFiles[x];
                file.sortScore = -1;
                int find = file.fullname.Find(query, 0, false);
                if (find > -1)
                {
                    int fudge = query.length - file.fullname.length;
                    int score = find * Abs(fudge*2) + Abs(fudge);
                    file.sortScore = score;
                    scored.Push(file);
                    scores.Push(score);
                }
            }
        }

        // cut this down for a faster sort
        if (scored.length > BROWSER_SEARCH_LIMIT)
        {
            scores.Sort();
            int scoreThreshold = scores[BROWSER_SEARCH_LIMIT];
            BrowserFile@ file;
            for(uint x=0;x<scored.length;x++)
            {
                file = scored[x];
                if (file.sortScore <= scoreThreshold)
                    filtered.Push(file);
            }
        }
        else
            filtered = scored;

        browserSearchSortMode = BROWSER_SORT_MODE_ALPHA;
        filtered.Sort();
        PopulateResourceBrowserResults(filtered);
        browserResultsMessage.text = "Showing top " + filtered.length + " of " + scored.length + " results";
    }
}

BrowserDir@ GetBrowserDirFromId(uint id)
{
    if (id == 0)
        return null;
    BrowserDir@ dir;
    for(uint i=0; i<browserDirs.length; i++)
    {
        @dir = @browserDirs[i];
        if (dir.id == id) return dir;
    }
    return null;
}

BrowserFile@ GetBrowserFileFromId(uint id)
{
    if (id == 0)
        return null;

    BrowserFile@ file;
    for(uint i=0; i<browserFiles.length; i++)
    {
        @file = @browserFiles[i];
        if (file.id == id) return file;
    }
    return null;
}

BrowserFile@ GetBrowserFileFromUIElement(UIElement@ element)
{
    if (element is null || !element.vars.Contains(TEXT_VAR_FILE_ID))
        return null;
    return GetBrowserFileFromId(element.vars[TEXT_VAR_FILE_ID].GetUInt());
}

BrowserDir@ GetBrowserDirFromUIElement(UIElement@ element)
{
    if (element is null || !element.vars.Contains(TEXT_VAR_DIR_ID))
        return null;
    return GetBrowserDirFromId(element.vars[TEXT_VAR_DIR_ID].GetUInt());
}

void HandleBrowserEditResource(StringHash eventType, VariantMap& eventData)
{
    UIElement@ element = eventData["Element"].GetPtr();
    BrowserFile@ file = GetBrowserFileFromUIElement(element);
    if (file is null)
        return;

    if (file.resourceType == BROWSER_TYPE_MATERIAL)
    {
        Material@ material = cache.GetResource("Material", file.resourceKey);
        if (material !is null)
            EditMaterial(material);
    }
}

void HandleBrowserOpenResource(StringHash eventType, VariantMap& eventData)
{
    UIElement@ element = eventData["Element"].GetPtr();
    BrowserFile@ file = GetBrowserFileFromUIElement(element);
    if (file !is null)
        OpenResource(file.resourceKey);
}

void HandleBrowserImportScene(StringHash eventType, VariantMap& eventData)
{
    UIElement@ element = eventData["Element"].GetPtr();
    BrowserFile@ file = GetBrowserFileFromUIElement(element);
    if (file !is null)
        ImportScene(file.GetFullPath());
}

void HandleBrowserImportModel(StringHash eventType, VariantMap& eventData)
{
    UIElement@ element = eventData["Element"].GetPtr();
    BrowserFile@ file = GetBrowserFileFromUIElement(element);
    if (file !is null)
        ImportModel(file.GetFullPath());
}

void HandleBrowserOpenUILayout(StringHash eventType, VariantMap& eventData)
{
    UIElement@ element = eventData["Element"].GetPtr();
    BrowserFile@ file = GetBrowserFileFromUIElement(element);
    if (file !is null)
        OpenUILayout(file.GetFullPath());
}

void HandleBrowserInstantiateStaticModel(StringHash eventType, VariantMap& eventData)
{
    UIElement@ element = eventData["Element"].GetPtr();
    BrowserFile@ file = GetBrowserFileFromUIElement(element);
    if (file !is null)
        CreateModelWithStaticModel(file.resourceKey, editNode);
}

void HandleBrowserInstantiateAnimatedModel(StringHash eventType, VariantMap& eventData)
{
    UIElement@ element = eventData["Element"].GetPtr();
    BrowserFile@ file = GetBrowserFileFromUIElement(element);
    if (file !is null)
        CreateModelWithAnimatedModel(file.resourceKey, editNode);
}

void HandleBrowserInstantiatePrefab(StringHash eventType, VariantMap& eventData)
{
    UIElement@ element = eventData["Element"].GetPtr();
    BrowserFile@ file = GetBrowserFileFromUIElement(element);
    if (file !is null)
        LoadNode(file.GetFullPath());
}

void HandleBrowserInstantiateInSpawnEditor(StringHash eventType, VariantMap& eventData)
{
    UIElement@ element = eventData["Element"].GetPtr();
    BrowserFile@ file = GetBrowserFileFromUIElement(element);
    if (file !is null)
    {
        spawnedObjectsNames.Resize(1);
        spawnedObjectsNames[0] = VerifySpawnedObjectFile(file.GetPath());
        RefreshPickedObjects();
        ShowSpawnEditor();
    }
}

void HandleBrowserLoadScene(StringHash eventType, VariantMap& eventData)
{
    UIElement@ element = eventData["Element"].GetPtr();
    BrowserFile@ file = GetBrowserFileFromUIElement(element);
    if (file !is null)
        LoadScene(file.GetFullPath());
}

void HandleBrowserRunScript(StringHash eventType, VariantMap& eventData)
{
    UIElement@ element = eventData["Element"].GetPtr();
    BrowserFile@ file = GetBrowserFileFromUIElement(element);
    if (file !is null)
        ExecuteScript(ExtractFileName(eventData));
}

void HandleBrowserFileDragBegin(StringHash eventType, VariantMap& eventData)
{
    UIElement@ uiElement = eventData["Element"].GetPtr();
    @browserDragFile = GetBrowserFileFromUIElement(uiElement);
}

void HandleBrowserFileDragEnd(StringHash eventType, VariantMap& eventData)
{
    if (@browserDragFile is null)
        return;

    UIElement@ element = ui.GetElementAt(ui.cursor.screenPosition);
    if (element !is null)
        return;

    if (browserDragFile.resourceType == BROWSER_TYPE_MATERIAL)
    {
        StaticModel@ model = cast<StaticModel>(GetDrawableAtMousePostion());
        if (model !is null)
            AssignMaterial(model, browserDragFile.resourceKey);
    }
    else if (browserDragFile.resourceType == BROWSER_TYPE_PREFAB)
    {
        browserDragNode = LoadNode(browserDragFile.GetFullPath());
        browserDragNode.position = GetMouse3DPosition();
    }

    browserDragFile = null;
    browserDragComponent = null;
    browserDragNode = null;
}

Menu@ CreateBrowserFileActionMenu(String text, String handler, BrowserFile@ browserFile = null)
{
    Menu@ menu = CreateContextMenuItem(text, handler);
    if (browserFile !is null)
        menu.vars[TEXT_VAR_FILE_ID] = browserFile.id;

    return menu;
}

class BrowserDir
{
    uint id;
    BrowserDir@ parent;
    String resourceKey;
    String name;
    Array<BrowserDir@> children;
    Array<BrowserFile@> files;
    int sortScore;

    BrowserDir(String name, BrowserDir@ parent = null)
    {
        @this.parent = parent;
        this.name = name;

        if (parent is null)
            resourceKey = name;
        else
            resourceKey = parent.resourceKey.empty ? name : parent.resourceKey + "/" + name;

        id = browserDirIndex++;
    }

    int opCmp(BrowserDir@ b)
    {
        if (browserSearchSortMode == 1)
            return name.opCmp(b.name);
        else
            return sortScore - b.sortScore;
    }

    BrowserFile@ AddFile(String name, uint resourceDirIndex)
    {
        BrowserFile@ file = BrowserFile(name, this, resourceDirIndex);
        files.Push(file);
        return file;
    }

    BrowserDir@ AddDir(String dir)
    {
        BrowserDir@ child = BrowserDir(dir, this);
        children.Push(child);
        return child;
    }

    Array<BrowserFile@> GetFilesSorted()
    {
        return files;
    }

    BrowserDir@ GetChild(String name)
    {
        BrowserDir@ itr;
        for(uint x=0; x < children.length; x++)
        {
            @itr = children[x];
            if (itr.name == name) return itr;
        }
        return null;
    }

    void ScanFiles()
    {
        files.Clear();
        for(uint i=0; i<cache.resourceDirs.length; i++)
            ScanFiles(i);
    }

    void ScanFiles(uint resourceDirIndex)
    {
        String path = cache.resourceDirs[resourceDirIndex] + resourceKey;
        if (!fileSystem.DirExists(path))
            return;

        // get files in directory
        Array<String> dirFiles = fileSystem.ScanDir(path, "*.*", SCAN_FILES, false);

        // add new files
        for (uint x=0; x < dirFiles.length; x++)
        {
            String filepath = dirFiles[x];
            BrowserFile@ browserFile = AddFile(filepath, resourceDirIndex);
            browserFiles.Push(browserFile);
            browserFilesToScan.Push(browserFile);
        }
    }
}

class BrowserFile
{
    uint id;
    uint resourceDirIndex;
    String resourceKey;
    String name;
    String fullname;
    String extension;
    ShortStringHash extensionType;
    ShortStringHash binaryType;
    ShortStringHash xmlType;
    int resourceType = 0;
    bool isResource = false;
    bool isBinary = false;
    bool isXml = false;
    BrowserDir@ dir;
    int sortScore = 0;

    BrowserFile(String filename, BrowserDir@ dir, uint resourceDirIndex)
    {
        @this.dir = dir;
        this.resourceDirIndex = resourceDirIndex;
        name = GetFileName(filename);
        extension = GetExtension(filename);
        fullname = filename;
        resourceKey = dir.resourceKey + "/" + fullname;
        id = browserFileIndex++;
    }

    int opCmp(BrowserFile@ b)
    {
        if (browserSearchSortMode == 1)
            return fullname.opCmp(b.fullname);
        else
            return sortScore - b.sortScore;
    }

    String FriendlyTypeName()
    {
        if (resourceType == BROWSER_TYPE_SCENE)
            return "Scene";
        else if (resourceType == BROWSER_TYPE_SCRIPTFILE)
            return "Script File";
        else if (resourceType == BROWSER_TYPE_MODEL)
            return "Model";
        else if (resourceType == BROWSER_TYPE_MATERIAL)
            return "Material";
        else if (resourceType == BROWSER_TYPE_ANIMATION)
            return "Animation";
        else if (resourceType == BROWSER_TYPE_IMAGE)
            return "Image";
        else if (resourceType == BROWSER_TYPE_SOUND)
            return "Sound";
        else if (resourceType == BROWSER_TYPE_TEXTURE)
            return "Texture";
        else if (resourceType == BROWSER_TYPE_FONT)
            return "Font";
        else if (resourceType == BROWSER_TYPE_PREFAB)
            return "Prefab";
        else if (resourceType == BROWSER_TYPE_TECHNIQUE)
            return "Render Technique";
        else if (resourceType == BROWSER_TYPE_PARTICLEEMITTER)
            return "Particle Emitter";
        else if (resourceType == BROWSER_TYPE_UIELEMENT)
            return "UI Element";
        else if (resourceType == BROWSER_TYPE_UIELEMENTS)
            return "UI Elements";
        else if (resourceType == BROWSER_TYPE_ANIMATION_SETTINGS)
            return "Animation Settings";
        else if (resourceType == BROWSER_TYPE_RENDERPATH)
            return "Render Path";
        else if (resourceType == BROWSER_TYPE_TEXTURE_ATLAS)
            return "Texture Atlas";
        else if (resourceType == BROWSER_TYPE_2D_PARTICLE_EFFECT)
            return "2D Particle Effect";
        else if (resourceType == BROWSER_TYPE_TEXTURE_3D)
            return "Texture 3D";
        else if (resourceType == BROWSER_TYPE_CUBEMAP)
            return "Cubemap";
        else
            return "";
    }

    void DetermainResourceType()
    {
        resourceType = TryToReadExtensionType();
        if (resourceType != -1) 
        {
            if (resourceType > 0)
                isResource = true;
            return;
        }

        resourceType = TryToReadBinaryType();
        if (resourceType > 0) 
        {
            isBinary = true;
            isResource = true;
            return;
        }

        resourceType = TryToReadXMLType();
        if (resourceType > 0) 
        {
            isXml = true;
            isResource = true;
            return;
        }

        resourceType = -1;
    }

    String GetFullPath()
    {
        return String(cache.resourceDirs[resourceDirIndex] + resourceKey);
    }

    String GetPath()
    {
        return String(resourceKey);
    }

    int TryToReadExtensionType()
    {
        ShortStringHash type =  ShortStringHash(extension);
        if (type == EXTENSION_TYPE_TTF)
        {
            extensionType = EXTENSION_TYPE_TTF;
            return BROWSER_TYPE_FONT;
        }
        else if (type == EXTENSION_TYPE_OGG)
        {
            extensionType = EXTENSION_TYPE_OGG;
            return BROWSER_TYPE_SOUND;
        }
        else if(type == EXTENSION_TYPE_WAV)
        {
            extensionType = EXTENSION_TYPE_WAV;
            return BROWSER_TYPE_SOUND;
        }
        else if(type == EXTENSION_TYPE_DDS)
        {
            extensionType = EXTENSION_TYPE_DDS;
            return BROWSER_TYPE_IMAGE;
        }
        else if(type == EXTENSION_TYPE_PNG)
        {
            extensionType = EXTENSION_TYPE_PNG;
            return BROWSER_TYPE_IMAGE;
        }
        else if(type == EXTENSION_TYPE_JPG)
        {
            extensionType = EXTENSION_TYPE_JPG;
            return BROWSER_TYPE_IMAGE;
        }
        else if(type == EXTENSION_TYPE_JPEG)
        {
            extensionType = EXTENSION_TYPE_JPEG;
            return BROWSER_TYPE_IMAGE;
        }
        else if(type == EXTENSION_TYPE_TGA)
        {
            extensionType = EXTENSION_TYPE_TGA;
            return BROWSER_TYPE_IMAGE;
        }
        else if(type == EXTENSION_TYPE_OBJ)
        {
            extensionType = EXTENSION_TYPE_OBJ;
            return BROWSER_TYPE_UNUSABLE;
        }
        else if(type == EXTENSION_TYPE_FBX)
        {
            extensionType = EXTENSION_TYPE_FBX;
            return BROWSER_TYPE_UNUSABLE;
        }
        else if(type == EXTENSION_TYPE_COLLADA)
        {
            extensionType = EXTENSION_TYPE_COLLADA;
            return BROWSER_TYPE_UNUSABLE;
        }
        else if(type == EXTENSION_TYPE_BLEND)
        {
            extensionType = EXTENSION_TYPE_BLEND;
            return BROWSER_TYPE_UNUSABLE;
        }
        else if(type == EXTENSION_TYPE_ANGELSCRIPT)
        {
            extensionType = EXTENSION_TYPE_ANGELSCRIPT;
            return BROWSER_TYPE_SCRIPTFILE;
        }
        else if(type == EXTENSION_TYPE_LUASCRIPT)
        {
            extensionType = EXTENSION_TYPE_LUASCRIPT;
            return BROWSER_TYPE_SCRIPTFILE;
        }
        else if(type == EXTENSION_TYPE_HLSL)
        {
            extensionType = EXTENSION_TYPE_HLSL;
            return BROWSER_TYPE_UNUSABLE;
        }
        else if(type == EXTENSION_TYPE_GLSL)
        {
            extensionType = EXTENSION_TYPE_GLSL;
            return BROWSER_TYPE_UNUSABLE;
        }
        else if(type == EXTENSION_TYPE_FRAGMENTSHADER)
        {
            extensionType = EXTENSION_TYPE_FRAGMENTSHADER;
            return BROWSER_TYPE_UNUSABLE;
        }
        else if(type == EXTENSION_TYPE_VERTEXSHADER)
        {
            extensionType = EXTENSION_TYPE_VERTEXSHADER;
            return BROWSER_TYPE_UNUSABLE;
        }
        else if(type == EXTENSION_TYPE_HTML)
        {
            extensionType = EXTENSION_TYPE_HTML;
            return BROWSER_TYPE_UNUSABLE;
        }
        return BROWSER_TYPE_UNKNOWN;
    }

    int TryToReadBinaryType()
    {
        File@ file = File();
        if (!file.Open(GetFullPath()))
            return -1;

        ShortStringHash type = ShortStringHash(file.ReadFileID());
        file.Close();
        if (type == BINARY_TYPE_SCENE)
        {
            binaryType = BINARY_TYPE_SCENE;
            return BROWSER_TYPE_SCENE;
        }
        else if (type == BINARY_TYPE_PACKAGE)
        {
            binaryType = BINARY_TYPE_PACKAGE;
            return BROWSER_TYPE_UNUSABLE;
        }
        else if (type == BINARY_TYPE_COMPRESSED_PACKAGE)
        {
            binaryType = BINARY_TYPE_COMPRESSED_PACKAGE;
            return BROWSER_TYPE_UNUSABLE;
        }
        else if (type == BINARY_TYPE_ANGLESCRIPT)
        {
            binaryType = BINARY_TYPE_ANGLESCRIPT;
            return BROWSER_TYPE_SCRIPTFILE;
        }
        else if (type == BINARY_TYPE_MODEL)
        {
            binaryType = BINARY_TYPE_MODEL;
            return BROWSER_TYPE_MODEL;
        }
        else if (type == BINARY_TYPE_SHADER)
        {
            binaryType = BINARY_TYPE_SHADER;
            return BROWSER_TYPE_UNUSABLE;
        }
        else if (type == BINARY_TYPE_ANIMATION)
        {
            binaryType = BINARY_TYPE_ANIMATION;
            return BROWSER_TYPE_ANIMATION;
        }
        return -1;
    }

    int TryToReadXMLType()
    {
        XMLFile xml;
        File@ file = File();
        if (!file.Open(GetFullPath()))
            return -1;

        if(xml.Load(file))
        {
            String name = xml.root.name;
            file.Close();
            if (name.empty) return -1;

            ShortStringHash type = ShortStringHash(name);
            if (type == XML_TYPE_SCENE)
            {
                xmlType = XML_TYPE_SCENE;
                return BROWSER_TYPE_SCENE;
            }
            else if (type == XML_TYPE_NODE)
            {
                xmlType = XML_TYPE_NODE;
                return BROWSER_TYPE_PREFAB;
            }
            else if(type == XML_TYPE_MATERIAL)
            {
                xmlType = XML_TYPE_MATERIAL;
                return BROWSER_TYPE_MATERIAL;
            }
            else if(type == XML_TYPE_TECHNIQUE)
            {
                xmlType = XML_TYPE_TECHNIQUE;
                return BROWSER_TYPE_TECHNIQUE;
            }
            else if(type == XML_TYPE_PARTICLEEMITTER)
            {
                xmlType = XML_TYPE_PARTICLEEMITTER;
                return BROWSER_TYPE_PARTICLEEMITTER;
            }
            else if(type == XML_TYPE_TEXTURE)
            {
                xmlType = XML_TYPE_TEXTURE;
                return BROWSER_TYPE_TEXTURE;
            }
            else if(type == XML_TYPE_ELEMENT)
            {
                xmlType = XML_TYPE_ELEMENT;
                return BROWSER_TYPE_UIELEMENT;
            }
            else if(type == XML_TYPE_ELEMENTS)
            {
                xmlType = XML_TYPE_ELEMENTS;
                return BROWSER_TYPE_UIELEMENTS;
            }
            else if (type == XML_TYPE_ANIMATION_SETTINGS)
            {
                xmlType = XML_TYPE_ANIMATION_SETTINGS;
                return BROWSER_TYPE_ANIMATION_SETTINGS;
            }
            else if (type == XML_TYPE_RENDERPATH)
            {
                xmlType = XML_TYPE_RENDERPATH;
                return BROWSER_TYPE_RENDERPATH;
            }
            else if (type == XML_TYPE_TEXTURE_ATLAS)
            {
                xmlType = XML_TYPE_TEXTURE_ATLAS;
                return BROWSER_TYPE_TEXTURE_ATLAS;
            }
            else if (type == XML_TYPE_2D_PARTICLE_EFFECT)
            {
                xmlType = XML_TYPE_2D_PARTICLE_EFFECT;
                return BROWSER_TYPE_2D_PARTICLE_EFFECT;
            }
            else if (type == XML_TYPE_TEXTURE_3D)
            {
                xmlType = XML_TYPE_TEXTURE_3D;
                return BROWSER_TYPE_TEXTURE_3D;
            }
            else if (type == XML_TYPE_CUBEMAP)
            {
                xmlType = XML_TYPE_CUBEMAP;
                return BROWSER_TYPE_CUBEMAP;
            }
            return -1;
        }
        file.Close();
        return -1;
    }
}
