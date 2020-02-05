#pragma once
#define WIN32_LEAN_AND_MEAN
#include "memory/String.h"
#include "memory/Containers.h"
#include "memory/smart_ptr.h"
#include <gameplay/Transform.h>
#include "scene/SceneGraph.h"
#include "gameplay/WaypointSystem.h"
#include "rendering/IRenderer.h"
#include "rendering/IParticleRenderer.h"
#include "core/SparseSet.h"

class GameDatabase;
namespace gfx
{
	struct TextureId;
	struct UITextOptionsForTexture;
	class IUIRenderer;
}

namespace tbsg
{
	class SceneHandler
	{
	public:
		GameDatabase& m_database;
		gfx::IRenderer* m_view{ nullptr };
		gfx::IParticleRenderer* particleRenderer{ nullptr };
		ptl::vector<ptl::shared_ptr<scene::SceneNode>> m_scenes{};
		ptl::vector<ptl::vector<std::pair<unsigned int, ptl::unique_ptr<scene::SceneNode>>>> m_cachedCardUnitsOfPlayers{};
		unsigned int m_currentScene{};// scene = chapter
		ptl::unordered_map<ptl::string, ptl::shared_ptr<scene::SceneNode>> loadedModels{};
        waypoint::WaypointData playerWaypoints{};
        waypoint::WaypointData opponentWaypoints{};
        waypoint::WaypointData cameraWaypoints{};

        waypoint::WaypointData cardWaypoints{};

        ptl::vector<scene::SceneNode*> monsterSpawnPoints{};
        ptl::vector<ptl::vector<scene::SceneNode*>> cardSpawnPoints{2};

        float timer;

        ptl::map<unsigned int, gfx::TextureId> cardTextures{};

        ptl::unordered_map<ptl::string, gfx::ParticleEmitter> particleEmitters{};

    public:
        SceneHandler(GameDatabase& database, gfx::IRenderer* view = nullptr)
            : m_database(database),
              m_view(view)
        {
        }
        /**
         * \brief loads the map file from the HDD and creates the initil unit cache. empty
         */
        void Initialize();
        /**
         * \brief Will disable a scene given by its name. Will unbind renderbles from the renderer
         * \param scene index of scene
         * \ingroup Gameplay
         */
        void DisableScene(unsigned int scene);
        /**
         * \brief Will enable the scene. This means it will bind renderbles to the renderer
         * \param scene index of scene
         * \ingroup Gameplay
         */
        void EnableScene(unsigned int scene);
        /**
         * \brief Will load scene from HDD
         * \warning will increase the m_currentScene member
         * \param scene scene name
         * \ingroup Gameplay
         */
        void LoadScene(ptl::string scene);

        void UnloadScene(unsigned int  id);
        /**
         * \brief Will load the map aka all the scenes associated with that ID.
         * \warning will reset m_currentScene to 0 after it has loaded all scenes
         * \param id for the identification of the chapter gltf names.
         * \ingroup Gameplay
         */
        void LoadMap(unsigned int  id);

        /**
         * \brief Unloads the current map. This means it will free all the resources
         * \ingroup Gameplay
         */
        void UnloadMap();

        void LoadParticles();
        void UnloadParticles();

        ParticleEmitter GetParticle(const ptl::string& name) const;

        ptl::shared_ptr<scene::SceneNode> ChangeScene();


        void DeleteRenderables(scene::SceneNode* sceneNode);
        /**
         * \brief frees the unit resources of the current match and restes the underlaying container
         * \ingroup Gameplay
         */
        void FreeUnitCache();

        /**
         * \brief Loads a Model form HDD if not already loaded and addes it to the scene. (Automatic Enable / add to renderer)
         * \param name name of the GLTF / Model
         * \param transform 
         * \param parent if nullptr it will add the model to the root object 
         * \return the added child.
         * \warning will assert if fails loading
         * \ingroup Gameplay
         */
        scene::SceneNode* AddModel(ptl::string name, core::Transform& transform, scene::SceneNode* parent = nullptr);

        scene::SceneNode* AddModel(ptl::unique_ptr<scene::SceneNode>&&, core::Transform& transform, scene::SceneNode* parent = nullptr);
        
        /**
         * \brief Will remove a model from the current scene. Will automatically disable it and remove renderble from renderer
         * \warning Will remove all *Childerern* objects if its an parent object
         * \param model 
         * \ingroup Gameplay
         */
        //void RemoveModel(scene::SceneNode* model);

        /**
         * \brief 
         * \warning Has as side effect that it will effect all its childeren if it has any.
         * \param model 
         * \ingroup Gameplay
         */
        void DisableModel(scene::SceneNode* model);
        /**
         * \brief 
         * \warning Has as side effect that it will effect all its childeren if it has any.
         * \param model 
         * \ingroup Gameplay
         */
        void EnableModel(scene::SceneNode* model);

        /**
         * \brief 
         * \param name 
         * \return will return false if the object cannot be found in scene as well
         */
        bool IsModelEnabled(const ptl::string& name) const;
        bool IsModelEnabled(scene::SceneNode* model) const;



        /**
         * \brief Returns the current scene based on the current scene index.
         * \ingroup Gameplay
         * \return 
         */
        ptl::shared_ptr <scene::SceneNode> GetCurrentScene();

		/// \brief Create a worldspace plane, with text on it. And add it as a child to the 'parent' node
		///
		/// \param options Text options which the text renderer will take into account.
		/// \param worldspacePanelWidth width of the panel in units
		/// \param worldspacePanelHeight height of the panel in units
		/// \param localTransform local transform values, which will be used for the child node
		/// \param parent (OPTIONAL) parent node, when nullptr is provided, the node will be parented to the scene root
		/// \param reverseWindingOrder reverse the winding order of the vertices of the plane (this will reverse the direction the panel is facing
		/// \param flipXAxisUVs option to flip the text image horizontally. Can be thought of as mirroring the image.
		scene::SceneNode* AddWorldspaceText(const gfx::UITextOptionsForTexture& options, float worldspacePanelWidth, float worldspacePanelHeight, 
			core::Transform localTransform = core::Transform{}, scene::SceneNode* parent = nullptr, bool reverseWindingOrder = false, bool flipXAxisUVs = true);

		/// \brief Create a worldspace plane, with text on it. And add it as a child to the 'parent' node
		///
		/// \param worldspacePanelWidth width of the panel in units
		/// \param worldspacePanelHeight height of the panel in units
		/// \param textureName texture name, from the texture path (eg "EmptyCard.png"). this function will load the texture if it hasn't been loaded already.
		/// \param localTransform local transform values, which will be used for the child node
		/// \param parent (OPTIONAL) parent node, when nullptr is provided, the node will be parented to the scene root
		/// \param reverseWindingOrder reverse the winding order of the vertices of the plane (this will reverse the direction the panel is facing
		/// \param flipXAxisUVs option to flip the text image horizontally. Can be thought of as mirroring the image.
		scene::SceneNode* AddWorldspacePlane(float worldspacePanelWidth, float worldspacePanelHeight, const ptl::string& textureName, //TODO: Add emissive texture support
			core::Transform localTransform = core::Transform{}, scene::SceneNode* parent = nullptr, bool reverseWindingOrder = false, bool flipXAxisUVs = true);

		scene::SceneNode* AddWorldspacePlane(float worldspacePanelWidth, float worldspacePanelHeight, const gfx::TextureId textureId, 
			core::Transform localTransform = core::Transform{}, scene::SceneNode* parent = nullptr, bool reverseWindingOrder = false, bool flipXAxisUVs = true);

		void UpdateWorldspaceText(const gfx::UITextOptionsForTexture& options, scene::SceneNode* node);

        /**
         * \brief Adds a cube to the world (if parent is not provieded) otherweise it will attach the cube to the given scenenode.
         * \param parent (OPTIONAL) if not provided it will choose the scene root.
         * \return returns the scene node.
         */
        scene::SceneNode* AddCubeToScene(const ptl::string& name, TextureId textureId, size_t size, core::Transform transform,scene::SceneNode* parent = nullptr); //TODO: Add emissive support.

		scene::SceneNode* GetHeroModel(unsigned int heroIndex);

        /**
         * \brief returns a cached object changes the name accordingly to its slot index.
         * \param cardID the card which is requested.
         * \param slotIndex the solt index where it is suppost to be played. Important for identifying the right model if a card is double
         * \param playerIndex player index to find the right card in cache.
         * \return 
         */
        scene::SceneNode* GetCardModelFromCache(unsigned int cardID, unsigned int slotIndex, unsigned int playerIndex);

        scene::SceneNode* CreateCardModelForPlayer(unsigned int cardID, unsigned int playerIndex);


        gfx::TextureId GetCardTexture(unsigned int cardID);

        void CreateCardTexture(unsigned int cardID);

        scene::SceneNode* ChangeMaterialOf(scene::SceneNode* model, const ptl::string& materialName);

        scene::SceneNode* ChangeMaterialOf(const ptl::string& modelName, const ptl::string& materialName);

        void SetRenderer(gfx::IRenderer* const view);
		void SetParticleRenderer(gfx::IParticleRenderer* const particleRenderer);
	};
}

//for later
//\note gfx::UITextOptionsForTexture::size will be overwritten by this function, it does not need to be filled in beforehand