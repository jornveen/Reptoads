# Scene Management - SceneModel

We need an instance of the `SceneModel`. During its initiation it will load all the maps form the `maps.json`. A map is nothing else than a collection of scenes. Scenes are `.glb` files. 

## How to load a map

In order to load a map the function `SceneModel::LoadMap(mapID)` needs to be called where the `mapID` is the a hash value for the name of the map send via the network. This function will load the level into memory and call internally `SceneModel::LoadScene(name)` which loads the actual `.glb` from the HDD. When ever `SceneModel::LoadScene(name)` is called it will increase the scene count (current scene).

To free resources the function `SceneModel::UnloadMap()` needs to be called be aware that everything associated with this scene will be deleted , freed

## Scene Changing, enabling disabling

`SceneModel::GetCurrentScene()` will return the current scene.  A scene can be disabled (`SceneModel:: DisableScene(scene index)`,`SceneModel::EnableScene(scene index)` ) or enabled by default the first scene of a map will be enabled after calling `LoadMap(mapId)`. 

The next scene can be loaded with the call `ChangeScene()` .

## Adding Models to the scene

If models need to be added to the scene the framework provides the following functions:

`SceneModel::AddModel`(ptl::string name, core::Transform& transform, scene::SceneNode* parent = nullptr)` this function will load a model from the HDD if it cannot find it in the cache and will add it to the root of the scene if no parent is given or will add it to the given node.

Models can be enabled or disabled with the function `SceneModel::EnableModel(scene::SceneNode* model)` or `SceneModel::DisableModel(scene::SceneNode* model)`

The framework provides functionality to check of a model is visible (rendered):

```cpp
        bool IsModelEnabled(const ptl::string& name);
        bool IsModelEnabled(scene::SceneNode* model);
```



**Example:**

```cpp
    auto scene = m_sceneModel.GetCurrentScene();
    auto ptr = scene->FindNodeByName("testCube");
   // std::cout << ptr->name << std::endl;
    std::cout << m_sceneModel.IsModelEnabled(ptr) << std::endl;
    std::cout << m_sceneModel.IsModelEnabled("testCube") << std::endl;
```



To free resources the function `SceneModel::RemoveModel(...)`



There are functions to create World UI and a Cube: 

`SceneModel::AddWorldspaceText(...)`

`SceneModel::AddWorldspacePlane(...)`

**Example:**

```cpp
m_sceneModel.AddWorldspacePlane(100.f, 300.f, "simon.jpg", core::Transform{ glm::vec3{0, 0, 0 }, glm::quat({0.f, 0.f, 0.f}), {1,1,1} });
```

`SceneModel::AddCubeToScene(...)`

**Example**

```cpp
m_sceneModel.AddCubeToScene("testCube", "simon.jpg", 1, core::Transform{ glm::vec3{0, 0, 0 }, glm::quat({0.f, 0.f, 0.f}), {1,1,1} });
```



## Game specific model cache

The `SceneModel` contains a game  specific cache for game models such as units. They can be added with the following call:

`CreateCardModelForPlayer(unsigned int cardID, unsigned int playerIndex)` This will add a model to the cache. This will load a `.glb` file associated with the card name from disc. 

To get a model from the cache the function `GetCardModelFromCache(...)` needs to be called. This function will delete the instance in the cache and move it out of the cache. Therefore if a card exists multiple time it needs to be cached multiple times. *The mesh will only be created once*

**Example:**

```cpp
auto model = m_sceneModel.GetCardModelFromCache(tbsg::SimpleHash("Bunny"), 1, 0);
auto transfrom = model->transform;
m_sceneModel.AddModel(ptl::unique_ptr<scene::SceneNode>{model}, transfrom, m_sceneModel.GetCurrentScene()->FindNodeByName("sp_1"));
```

This will spawn the card bunny at the spawn point sp_1

**IMPORTANT:** after each game the cache needs to be freed!



## Fill the Cache

The `GameLogic` know the function `PreLoadDeck(vector<int> cardIds)`  this function will create the cache for the `SceneModel` when ever a player sends the `ESendOpponentDeck` or when ever the player creates the game itself.



## Utility

The `SceneModel` has as well a function to change the material of a model either by giving it a pointer to the model or its identifier (name)