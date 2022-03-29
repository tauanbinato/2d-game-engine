#ifndef ECS_H
#define ECS_H

#include <bitset>
#include <vector>
#include <unordered_map>
#include <typeindex>
#include <set>
#include <memory>
#include "../Logger/Logger.h"
#include <deque>

const unsigned int MAX_COMPONENTS = 32;

////////////////////////////////////////////////////////////////////////////////////
// Signature
////////////////////////////////////////////////////////////////////////////////////
//// We use a bitset (1..0) to keep track of which components an entity has,
//// and also helps keep track of which entities a system is interested in.
////////////////////////////////////////////////////////////////////////////////////
typedef std::bitset<MAX_COMPONENTS> Signature;

////////////////////////////////////////////////////////////////////////////////////
// Entity
////////////////////////////////////////////////////////////////////////////////////
//// 
////////////////////////////////////////////////////////////////////////////////////
class Entity {
    private:
        int m_id;
    
    public:
        Entity(int id): m_id(id) {};
        Entity(const Entity& entity) = default;

        int GetId() const;
        void Kill();

        Entity& operator =(const Entity& other) = default;
        // Overloading the operator of == when using with Entity instances
        bool operator == (const Entity& other) const { return m_id == other.m_id; }
        bool operator !=(const Entity& other) const { return m_id != other.m_id; }
        bool operator >(const Entity& other) const { return m_id > other.m_id; }
        bool operator <(const Entity& other) const { return m_id < other.m_id; }

        template <typename TComponent, typename ...TArgs> void AddComponent(TArgs&& ...args);
        template <typename TComponent> void RemoveComponent();
        template <typename TComponent> bool HasComponent() const;
        template <typename TComponent> TComponent& GetComponent() const;

        // Hold a pointer to the entity's owner register
        // Just a forward declaration because the actuall class is bellow in file.
        class Registry* m_registry;
};

////////////////////////////////////////////////////////////////////////////////////
// Component
////////////////////////////////////////////////////////////////////////////////////
//// 
////////////////////////////////////////////////////////////////////////////////////

struct IComponent {
    protected:
        static int m_nextId;
};

// Used to assign a unique id to a component type
template <typename TComponent>
class Component: public IComponent {
    public:
        // Returns the unique id of the Component<T>
        static int GetId() {
            static auto id = m_nextId++;
            return id;
        }
};

////////////////////////////////////////////////////////////////////////////////////
// System
////////////////////////////////////////////////////////////////////////////////////
//// The system processes entities that contain a specific signature
////////////////////////////////////////////////////////////////////////////////////
class System {
    private:
        Signature m_componentSignature;
        std::vector<Entity> m_entities;

    public:
        System() = default;
        ~System() = default;

        void AddEntityToSystem(Entity entity);
        void RemoveEntityFromSystem(Entity entity);
        std::vector<Entity> GetSystemEntities() const;
        const Signature& GetComponentSignature() const;

        // Define the component type T that entities must have to be considered by the system
        template <typename TComponent> void RequireComponent();
};

///////////////////////////////////////////////////////////////////////////////////////////////
// Pool
///////////////////////////////////////////////////////////////////////////////////////////////
//// A Pool is just a vector (continuous data) of objects of type T (generic) 
///  this is a template class, we implement that in .h file
///  IPool is just a trick to generic classes, so in the register we can point to IPool and
///  this way we will keep Pool generic.
///////////////////////////////////////////////////////////////////////////////////////////////
class IPool {
    public: 
        virtual ~IPool() {}
};

template <typename T>
class Pool: public IPool {
    private:
        std::vector<T> data;

    public:
        Pool(int size = 100) { 
            data.resize(size); 
        }

        virtual ~Pool() = default;

        bool IsEmpty() const { return data.empty(); }

        int GetSize() const { return data.size(); }

        void Resize(int n) { data.resize(n); }

        void Clear() { data.clear(); }

        void Add(T object) {
            data.push_back(object);
        };

        void Set(int index, T object) {
            data[index] = object;
        }

        T& Get(int index){
            return static_cast<T&>(data[index]);
        }

        T& operator [](unsigned int index) {
            return data[index];
        }
};

///////////////////////////////////////////////////////////////////////////////////////////////
// Registry
///////////////////////////////////////////////////////////////////////////////////////////////
//// The registry manages the creation and destruction of entities, add systems and components 
///////////////////////////////////////////////////////////////////////////////////////////////
class Registry {
    private:

        int m_numEntities = 0;

        // Works like a buffer, entities awaiting creation and destruction in the next Registry update();
        std::set<Entity> m_entitiesTobeAdded;
        std::set<Entity> m_entitiesToBeKilled;

        // Vector of component pools, each pool contains all the data for a certain component type
        // [Vector index = component type id]
        // [Pool index = entity id]
        std::vector<std::shared_ptr<IPool>> m_componentTypePools;

        // Vector of component signatures
        // The signature lets us know which components are turned on for an entity
        // [Vector index = entity id]
        std::vector<Signature> m_entityComponentSignatures;

        // Keep track of all systems
        std::unordered_map<std::type_index, std::shared_ptr<System>> m_systems;

        // List of available free entity id's that were previously removed
        std::deque<int> m_freeIds;

    public:
        Registry() { 
            Logger::Log("Registry constructor called"); 
        }
        ~Registry() {
            Logger::Log("Registry destructor called"); 
        }

        void Update();

        //##### Entity Managment ####################################################################
        Entity CreateEntity();
        void KillEntity(Entity entity);

        //##### Component Managment #################################################################

        // Add a component based on its type and arguments.
        template <typename TComponent, typename ...TArgs> void AddComponent(Entity entity, TArgs&& ...args);

        // Remove a component based on its type
        template <typename TComponent> void RemoveComponent(Entity entity);

        template <typename TComponent> bool HasComponent(Entity entity) const;

        template <typename TComponent> TComponent& GetComponent(Entity entity) const;

        //##### System Managment ####################################################################

        // Add and remove entities from their systems.
        void AddEntityToSystems(Entity entity);
        void RemoveEntityFromSystems(Entity entity);

        // Add a system
        template <typename TSystem, typename ...TArgs> void AddSystem(TArgs&& ...args);
        // Remove System
        template <typename TSystem> void RemoveSystem();
        // Check if has a system
        template <typename TSystem> bool HasSystem() const;
        // Get System
        template <typename TSystem> TSystem& GetSystem() const;
        
};

///////////////////////////////////////////////////////////////////////////////////////////////
// Template Functions Implementations
///////////////////////////////////////////////////////////////////////////////////////////////

template <typename TSystem, typename ...TArgs> 
void Registry::AddSystem(TArgs&& ...args) {
    std::shared_ptr<TSystem> newSystem = std::make_shared<TSystem>(std::forward<TArgs>(args)...);
    m_systems.insert(std::make_pair( std::type_index(typeid(TSystem)), newSystem ));
}

template <typename TSystem> 
void Registry::RemoveSystem() {
    auto system = m_systems.find(std::type_index(typeid(TSystem)));
    m_systems.erase(system);
}

template <typename TSystem> 
bool Registry::HasSystem() const {
    return (m_systems.find(std::type_index(typeid(TSystem))) != m_systems.end());
}

template <typename TSystem> 
TSystem& Registry::GetSystem() const {
    auto system = m_systems.find(std::type_index(typeid(TSystem)));
    return *(std::static_pointer_cast<TSystem>(system->second));
}

template <typename TComponent, typename ...TArgs> 
void Registry::AddComponent(Entity entity, TArgs&& ...args) {
    
    const auto componentId = Component<TComponent>::GetId();
    const auto entityId = entity.GetId();

    // if the component id is greater then the current size of the componentPools, then resize the vector
    // in other words, if we have a new component type we need to acomodate for that.
    if (componentId >= static_cast<int>(m_componentTypePools.size())) {
        m_componentTypePools.resize(componentId + 1, nullptr);
    }

    // If we do not have a pool for that component type we create one.
    if (!m_componentTypePools[componentId]) {
        std::shared_ptr<Pool<TComponent>> newComponentPool = std::make_shared<Pool<TComponent>>();
        m_componentTypePools[componentId] = newComponentPool;
    }

    // Getting the pool of component values for that component type
    std::shared_ptr<Pool<TComponent>> componentPool = std::static_pointer_cast<Pool<TComponent>>(m_componentTypePools[componentId]);

    // If the entity id is greater then the current size of the component pool, then we resize the pool
    // to acomodate for that entity
    if (entityId >= componentPool->GetSize()) {
        componentPool->Resize(m_numEntities);
    }

    // Create a new Component object of the type T(generic), and forward the parameters to the constructor.
    // Ex: TransformComponent newComponent(Posx, PosY, etc..)
    TComponent newComponent(std::forward<TArgs>(args)...);

    // Add the new componenet to the component type pool list using the entity id as index.
    componentPool->Set(entityId, newComponent);

    // Finally, change the component signature of the entity and set the component id
    m_entityComponentSignatures[entityId].set(componentId);

    //Logger::Log("Component id = " + std::to_string(componentId) + " was added to entity id " + std::to_string(entityId));
}

template <typename TComponent> 
void Registry::RemoveComponent(Entity entity) {
    const auto componentId = Component<TComponent>::GetId();
    const auto entityId = entity.GetId();

    m_entityComponentSignatures[entityId].set(componentId, false);
    //Logger::Log("Component id = " + std::to_string(componentId) + " was removed from entity id " + std::to_string(entityId));
};

template <typename TComponent> 
bool Registry::HasComponent(Entity entity) const {
    const auto componentId = Component<TComponent>::GetId();
    const auto entityId = entity.GetId();

    return m_entityComponentSignatures[entityId].test(componentId);
}

template <typename TComponent> 
TComponent& Registry::GetComponent(Entity entity) const {
    const auto componentId = Component<TComponent>::GetId();
    const auto entityId = entity.GetId();
    auto componentPool =  std::static_pointer_cast<Pool<TComponent>>(m_componentTypePools[componentId]);
    return componentPool->Get(entityId);
}

template <typename TComponent, typename ...TArgs> 
void Entity::AddComponent(TArgs&& ...args) {
    m_registry->AddComponent<TComponent>(*this, std::forward<TArgs>(args)...);
}

template <typename TComponent> 
void Entity::RemoveComponent() {
    m_registry->RemoveComponent<TComponent>(*this);
}

template <typename TComponent> 
bool Entity::HasComponent() const {
    return m_registry->HasComponent<TComponent>(*this);
}

template <typename TComponent> 
TComponent& Entity::GetComponent() const {
    return m_registry->GetComponent<TComponent>(*this);
}


template <typename TComponent>
void System::RequireComponent(){
    const auto componentId = Component<TComponent>::GetId();
    System::m_componentSignature.set(componentId);
};

#endif